#include "comp.h"
#include "teutils.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <array>
#include <fstream>

#include "logging.h"

//#include <bitset>

Comp::Comp(CompFlags compFlags, uint8_t device) 
	:	compFlags_(compFlags),
		tryDevice_(device)
{
}

Comp::Comp(const std::string& filePath, CompFlags compFlags, int64_t fps, uint8_t device)
	:	compFlags_(compFlags),
		tryDevice_(device)
{
	spdlog::debug("Creating Comp");
	
	initComp(compFlags, device);
	load(filePath, compFlags, fps);

	spdlog::default_logger()->flush();
}

void
Comp::initComp(CompFlags compFlags, uint8_t device)
{
	createRenderer(device);

	if (!(compFlags & CompFlagBits::CudaDisable)) cudaInit();
	
	initInstance();
}

Comp::~Comp()
{
	// Calling unload here can cause python to deadlock if it doesn't finish before python is closed... 
	// call unload() before destruction, or not at all but that will cause memory leaks if the object is the global scope
	//unload();

	if (cudaStream_ && compFlags_ & CompFlagBits::CudaStreamInternal) CUDA_CHECK(cudaStreamDestroy(cudaStream_));

	vkDestroyFence(device_, submitFence_, nullptr);

	spdlog::debug("Comp destroyed");
	spdlog::default_logger()->flush();
}

void 
Comp::createRenderer(uint8_t device)
{
	renderer_ = Renderer::instance(device);

	device_ = renderer_->vContext().device;
	physicalDevice_ = renderer_->vContext().physicalDevice;
	queueFamilyIndices_ = renderer_->vContext().queueFamilyIndices.usedFamilyIndices();
	queue_ = renderer_->vContext().transferQueue;
	commandBuffer_ = renderer_->vContext().transferCommandBuffers[0];

	VkFenceCreateInfo fenceCreateInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 0 };
	vkCreateFence(device_, &fenceCreateInfo, nullptr, &submitFence_);
}

void
Comp::cudaInit()
{
	if (!setCudaDevice())
	{
		spdlog::warn("No capable CUDA device set, texture IO is not available");
		return;
	}

	if (compFlags_ & CompFlagBits::CudaStreamInternal) 
	{
		CUDA_CHECK(cudaStreamCreate(&cudaStream_));
		spdlog::debug("CUDA stream created: {}", static_cast<void*>(cudaStream_));
	}

	return;
}

bool
Comp::setCudaDevice()
{
	int deviceCount;
	CUDA_CHECK(cudaGetDeviceCount(&deviceCount));
	if (deviceCount == 0)
	{
		spdlog::warn("No CUDA devices found");
		spdlog::default_logger()->flush();
		return false;
	}

	int device = 0;
	int devicesProhibited = 0;
	cudaDeviceProp deviceProp;

	while (device < deviceCount)
	{
		CUDA_CHECK(cudaGetDeviceProperties(&deviceProp, device));
		if (deviceProp.computeMode != cudaComputeModeProhibited)
		{
			int result = std::memcmp(&deviceProp.uuid, renderer_->physicalDeviceUUID(), VK_UUID_SIZE);
			if (result == 0)
			{
				CUDA_CHECK(cudaSetDevice(device));
				CUDA_CHECK(cudaGetDeviceProperties(&deviceProp, device));
				spdlog::debug("Set CUDA device: {} : {} with compute {}", device, deviceProp.name, deviceProp.major, deviceProp.minor);

				cudaDevice_ = device;
				return true;
			}
		}
		else
		{
			++devicesProhibited;
		}
		++device;
	}

	if (devicesProhibited == deviceCount)
	{
		spdlog::warn("No Vulkan/CUDA interop capable device found");
		spdlog::default_logger()->flush();
		return false;
	}

	spdlog::warn("No CUDA device found with Vulkan device UUID");
	spdlog::default_logger()->flush();
	return false;
}

bool 
Comp::initInstance()
{
	TEResult result = TEInstanceCreate(eventCallback, linkEventCallback, this, instance_.take());
	if (result == TEResultSuccess)
		spdlog::debug("TEInstance created");
	else
	{
		spdlog::error("Failed to create TEInstance: {}", TEResultGetDescription(result));
		spdlog::default_logger()->flush();
		throw std::runtime_error("Failed to create TEInstance");
	}

	result = TEInstanceAssociateGraphicsContext(instance_, renderer_->teContext());
	if (result == TEResultSuccess)
		spdlog::debug("TEInstance associated with Vulkan Graphics Context");
	else
	{
		spdlog::error("Failed to associate TEInstance with Graphics Context: {}", TEResultGetDescription(result));
		spdlog::default_logger()->flush();
		throw std::runtime_error("Failed to associate TEInstance with Graphics Context");
	}
	return true;
}

bool Comp::load(const std::string& filePath, int64_t fps)
{
	if (!renderer_) initComp(compFlags_, tryDevice_);

	std::ifstream file(filePath, std::ios::in | std::ios::binary);
	if (!file.is_open())
	{
		spdlog::error("Failed to open tox file: {}", filePath);
		throw std::runtime_error("Failed to open tox file");
	}

	return load(filePath, compFlags_, fps);
}

bool Comp::load(const std::string& filePath, CompFlags compFlags, int64_t fps)
{
	std::ifstream file(filePath, std::ios::in | std::ios::binary);
	if (!file.is_open())
	{
		spdlog::error("Failed to open tox file: {}", filePath);
		throw std::runtime_error("Failed to open tox file");
	}

	compFlags_ = compFlags;
	TEResult result = TEInstanceSetFrameRate(instance_, fps, 1);
	if (result != TEResultSuccess)
	{
		spdlog::error("Failed to set frame rate: {}", TEResultGetDescription(result));
		throw std::runtime_error("Failed to set frame rate");
	}

	filePath_ = filePath;
	spdlog::debug("Loading tox: {}", filePath_);

	auto timeMode = TETimeInternal;
	if (compFlags_ & CompFlagBits::ExternalTime) timeMode = TETimeExternal;

	result = TEInstanceConfigure(instance_, filePath_.c_str(), timeMode, TEUIWindows);
	if (result != TEResultSuccess)
	{
		spdlog::error("Failed to configure TEInstance: {}", TEResultGetDescription(result));
		throw std::runtime_error("Failed to configure TEInstance");
	}
	
	std::unique_lock<std::mutex> lock(mutex_);
	result = TEInstanceLoad(instance_);
	if (result == TEResultSuccess)
		spdlog::debug("Instance loading...");
	else
	{
		spdlog::error("Failed to initiate loading of TEInstance: {}", TEResultGetDescription(result));
		throw std::runtime_error("Failed to initiate loading of TEInstance");
	}

	spdlog::default_logger()->flush();
	
	if (!onLoadedCallback_) cv_.wait(lock, [this] { return ssLoaded_; });

	return ssLoaded_;
}

void 
Comp::unload()
{
	if (asyncRunning_.load()) stopAsync();
	else if (updateLoopRunning_) stopUpdate();

	

	auto state = getState();

	if (state.loaded)
	{
		spdlog::debug("Unloading TEInstance...");
		spdlog::default_logger()->flush();

		{
			std::lock_guard<std::mutex> guard(mutex_);
			ssUnloading_ = true;
		}

		onLoadedCallback_ = nullptr;
		onLoadedData_ = nullptr;
		onStartCallback_ = nullptr;
		onStartData_ = nullptr;
		onStopCallback_ = nullptr;
		onStopData_ = nullptr;
		onFrameData_ = nullptr;
		onFrameCallback_ = nullptr;
		onLayoutChangeData_ = nullptr;
		onLayoutChangeCallback_ = nullptr;

		cudaStreamSynchronize(cudaStream_);

		TEResult result = TEInstanceUnload(instance_);
		if (result != TEResultSuccess)
		{
			spdlog::error("Failed to initiate unloading of TEInstance: {}", TEResultGetDescription(result));
			throw std::runtime_error("Failed to initiate unloading of TEInstance");
		}

		// waiting seems to cause a deadlock when running async even though the thread is joined... 
		//std::unique_lock<std::mutex> lock(mutex_);
		//cv_.wait(lock, [this] { return !ssLoaded_; });
	}
	
}


bool 
Comp::loaded() const
{
	std::lock_guard<std::mutex> guard(mutex_);
	return ssLoaded_;
}


void 
Comp::eventCallback(TEInstance* instance,
	TEEvent event,
	TEResult result,
	int64_t start_time_value,
	int32_t start_time_scale,
	int64_t end_time_value,
	int32_t end_time_scale,
	void* info)
{
	if (event != TEEventFrameDidFinish)
	{
		spdlog::debug("eventCallback: {} result: {}", teutils::eventToString(event), TEResultGetDescription(result));
		spdlog::default_logger()->flush();
	}

	Comp* comp = static_cast<Comp*>(info);

	switch (event)
	{
	case TEEventInstanceReady:
		comp->onEventInstanceReady(result, comp);
		break;
	case TEEventInstanceDidLoad:
		comp->onEventInstanceDidLoad(result, comp);
		break;
	case TEEventInstanceDidUnload:
		comp->onEventInstanceDidUnload(result, comp);
		break;
	case TEEventFrameDidFinish:
		comp->onEventFrameDidFinish(result, start_time_value, start_time_scale, end_time_value, end_time_scale, comp);
		break;
	case TEEventGeneral:
		comp->onEventGeneral(result, start_time_value, start_time_scale, comp);
		break;
	default:
		break;
	}
}

void 
Comp::onEventInstanceReady(TEResult result, Comp* comp)
{
	if (!comp) return;

	{
		std::lock_guard<std::mutex> lock(comp->mutex_);
		comp->ssReady_ = result == TEResultSuccess;
	}
	spdlog::debug("Instance ready: {}", TEResultGetDescription(result));
}

void 
Comp::onEventInstanceDidLoad(TEResult result, Comp* comp)
{
	{
		std::unique_lock<std::mutex> lock(mutex_);
		comp->ssLoaded_ = true;
		if (onLoadedCallback_) onLoadedCallback_(onLoadedData_);
		else comp->cv_.notify_one(); // notify load() that instance is ready
	}
	spdlog::debug("Instance loaded: {}", TEResultGetDescription(result));
	spdlog::default_logger()->flush();
}

void 
Comp::onEventInstanceDidUnload(TEResult result, Comp* comp)
{
	{
		std::unique_lock<std::mutex> lock(mutex_);
		ssUnloading_ = false;
		ssLoaded_ = false;
		ssReady_ = false;
		if (onUnloadedCallback_) onUnloadedCallback_(onUnloadedData_);
		// waiting in unload() seems to cause a deadlock when running async even though the thread is joined... 
		//else cv_.notify_one(); // notify unload() that instance is unloaded)
	}

	spdlog::debug("Instance unloaded: {}", TEResultGetDescription(result));
	spdlog::default_logger()->flush();
}

void 
Comp::onEventFrameDidFinish(TEResult result, int64_t start_time_value, int32_t start_time_scale, 
	int64_t end_time_value, int32_t end_time_scale, Comp* comp)
{
	//if (result == TEResultSuccess && start_time_value >= 0)
	if (result == TEResultSuccess)
	{
		comp->setInFrame(false, true, end_time_value, end_time_scale);
	}
	else
	{
		if(result != TEResultCancelled)
		{
			if (result == TEResultComponentErrors || result == TEResultComponentWarnings) comp->setInFrame(false, true, end_time_value, end_time_scale);

			else
			{
				// need go through all possible results and handle them accordingly... 
				auto severity = TEResultGetSeverity(result);

				if (severity == TESeverityWarning)
				{
					spdlog::warn("Warning frame did not finish successfully: {} {}", static_cast<int>(result), TEResultGetDescription(result));
					comp->setInFrame(false, true, end_time_value, end_time_scale);
				}
				else if (severity == TESeverityError)
				{
					std::string error = TEResultGetDescription(result);
					error = "Frame did not finish successfully: " + error;
					spdlog::error(error.c_str());

					throw std::runtime_error(error.c_str());
				}
				else
				{
					spdlog::warn("Frame did not finish successfully: {} {}", static_cast<int>(result), TEResultGetDescription(result));
					comp->setInFrame(false, true, end_time_value, end_time_scale);
				}
			}
		}
	}
}

void 
Comp::onEventGeneral(TEResult result, uint64_t start_time, uint64_t end_time, Comp* comp)
{
	//spdlog::info("General event: {}", TEResultGetDescription(result));
}


void 
Comp::linkEventCallback(TEInstance* instance, TELinkEvent event, const char* identifier, void* info)
{
	Comp* comp = static_cast<Comp*>(info);
	switch (event)
	{
	case TELinkEventValueChange:
		comp->onLinkEventValueChange(identifier);
		break;
	case TELinkEventAdded:
		comp->onLinkEventAdded(identifier);
		break;
	case TELinkEventRemoved:
		comp->onLinkEventRemoved(identifier);
		break;
	case TELinkEventModified:
		comp->onLinkEventModified(identifier);
		break;
	case TELinkEventMoved:
		comp->onLinkEventMoved(identifier);
		break;
	case TELinkEventStateChange:
		comp->onLinkEventStateChange(identifier);
		break;
	case TELinkEventChildChange:
		comp->onLinkEventChildChange(identifier);
		break;
	default:
		break;
	}
}

void 
Comp::onLinkLayoutChange(TELinkEvent event, const char* identifier)
{
	std::lock_guard<std::mutex> guard(mutex_);
	ssPendingLayoutChange_ = true;

}

void 
Comp::onLinkEventValueChange(const char* identifier)
{
	TouchObject<TELinkInfo> link;
	TEResult result = TEInstanceLinkGetInfo(instance_, identifier, link.take());
	if (result != TEResultSuccess)
		return;
	
	if (link->scope == TEScopeOutput)
	{
		switch (link->type)
		{
		case TELinkTypeTexture:
		{
			std::lock_guard<std::mutex> guard(mutex_);
			ssPendingOutputTextures_.push_back(identifier);
			break;
		}
		case TELinkTypeFloatBuffer:
		{
			if (asyncActive_)
			{
				auto chopLink = outChopLinks_->getLinkByIdentifier(identifier);
				chopLink->writeBuffer();
			}
			{
				std::lock_guard<std::mutex> guard(mutex_);
				ssPendingOutputFloatBuffers.push_back(identifier);
			}
			break;
		}
		case TELinkTypeStringData:
		{
			if (asyncActive_)
			{
				auto datLink = outDatLinks_->getLinkByIdentifier(identifier);
				datLink->writeBuffer();
			}
			{
				std::lock_guard<std::mutex> guard(mutex_);
				ssPendingOutputStringData.push_back(identifier);
			}
			break;
		}
		default:
			break;
		}
	}
}

const Comp::State
Comp::getState()
{
	State state;
	std::lock_guard<std::mutex> guard(mutex_);
	state.loaded = ssLoaded_;
	state.ready = ssReady_;

	if (ssLoaded_ && ssReady_)
	{
		state.linksLayoutChanged = ssPendingLayoutChange_;
		state.inFrame = ssInFrame_;
		ssPendingLayoutChange_ = false;
	}
	else
	{
		state.linksLayoutChanged = false;
		state.inFrame = false;
	}

	return state;
}

void 
Comp::setInFrame(bool inFrame, bool setTime, int64_t timeValue, int32_t timeScale)
{
	auto rate = frameRate();
	int64_t currentFrame = 0;
	double seconds = 0.0;
	if (timeScale > 0 && rate > 0)
	{
		currentFrame = static_cast<int64_t>(timeValue / (static_cast<float>(timeScale) / rate));
		seconds = static_cast<double>(currentFrame) / rate;
	}

	std::unique_lock<std::mutex> lock(mutex_);
	ssInFrame_ = inFrame;
	if (setTime)
	{
		ssTime.rate = rate;
		ssTime.frame = currentFrame;
		ssTime.seconds = seconds;
		ssTime.value = timeValue;
		ssTime.scale = timeScale;
	}

	if (asyncActive_) cv_.notify_one();
	lock.unlock();
}

void
Comp::setOnLoadedCallback(CallbackFunc callback, CallbackData data)
{
	onLoadedCallback_ = callback;
	onLoadedData_ = data;
	return;
}

void
Comp::setOnUnloadedCallback(CallbackFunc callback, CallbackData data)
{
	onUnloadedCallback_ = callback;
	onUnloadedData_ = data;
	return;
}

void
Comp::setOnStartCallback(CallbackFunc callback, CallbackData data)
{
	onStartCallback_ = callback;
	onStartData_ = data;
	return;
}

void
Comp::setOnStopCallback(CallbackFunc callback, CallbackData data)
{
	onStopCallback_ = callback;
	onStopData_ = data;
	return;
}

void 
Comp::setOnFrameCallback(CallbackFunc callback, CallbackData data)
{
	if (!asyncActive_)
	{
		onFrameCallback_ = nullptr;
		onFrameData_ = nullptr;

		onFrameCallback_ = callback;
		onFrameData_ = data;
		return;
	}
	else
	{
		std::unique_lock<std::mutex> lock(asyncMutex_);
		cv_.wait(lock, [this] { return ssInFrame_; });

		// notify async thread to wait for callback to finish
		// if async thread is running
		if (asyncRunning_.load())
		{
			asyncSettingCallback_ = true;
			asyncCV_.notify_one();
		}

		onFrameCallback_ = nullptr;
		onFrameData_ = nullptr;

		onFrameCallback_ = callback;
		onFrameData_ = data;

		if (asyncRunning_.load())
		{
			asyncSettingCallback_ = false;
			asyncCV_.notify_one();
		}

		lock.unlock();
	}
}


void 
Comp::clearOnFrameCallback()
{
	if (!asyncActive_)
	{
		onFrameCallback_ = nullptr;
		onFrameData_ = nullptr;
		return;
	}
	else
	{
		std::unique_lock<std::mutex> lock(asyncMutex_);
		cv_.wait(lock, [this] { return ssInFrame_; });

		if (asyncRunning_.load())
		{
			asyncSettingCallback_ = true;
			asyncCV_.notify_one();
		}

		onFrameCallback_ = nullptr;
		onFrameData_ = nullptr;

		if (asyncRunning_.load())
		{
			asyncSettingCallback_ = false;
			asyncCV_.notify_one();
		}

		lock.unlock();
	}
}

bool 
Comp::callOnFrameCallback()
{
	if (onFrameCallback_)
	{
		onFrameCallback_(onFrameData_);
		return true;
	}
	return false;
}

void 
Comp::setOnLayoutChangeCallback( CallbackFunc callback, CallbackData data)
{
	if (!asyncActive_)
	{
		onLayoutChangeCallback_ = nullptr;
		onLayoutChangeData_ = nullptr;

		onLayoutChangeCallback_ = callback;
		onLayoutChangeData_ = data;
		return;
	}
	else
	{
		std::unique_lock<std::mutex> lock(asyncMutex_);
		cv_.wait(lock, [this] { return ssInFrame_; });

		if (asyncRunning_.load())
		{
			asyncSettingCallback_ = true;
			asyncCV_.notify_one();
		}

		onLayoutChangeCallback_ = nullptr;
		onLayoutChangeData_ = nullptr;

		onLayoutChangeCallback_ = callback;
		onLayoutChangeData_ = data;

		if (asyncRunning_.load())
		{
			asyncSettingCallback_ = false;
			asyncCV_.notify_one();
		}

		lock.unlock();
	}
}

void 
Comp::clearOnLayoutChangeCallback()
{
	if (!asyncActive_)
	{
		onLayoutChangeCallback_ = nullptr;
		onLayoutChangeData_ = nullptr;
		return;
	}
	else
	{
		std::unique_lock<std::mutex> lock(asyncMutex_);
		cv_.wait(lock, [this] { return ssInFrame_; });

		if (asyncRunning_.load())
		{
			asyncSettingCallback_ = true;
			asyncCV_.notify_one();
		}

		onLayoutChangeCallback_ = nullptr;
		onLayoutChangeData_ = nullptr;

		if (asyncRunning_.load())
		{
			asyncSettingCallback_ = false;
			asyncCV_.notify_one();
		}

		lock.unlock();
	}
}

bool 
Comp::callOnLayoutChangeCallback()
{
	if (onLayoutChangeCallback_)
	{
		onLayoutChangeCallback_(onLayoutChangeData_);
		return true;
	}
	return false;
}

Comp::Time 
Comp::time() const
{	
	Time time__;
	{
		std::lock_guard<std::mutex> guard(mutex_);
		time__ = ssTime;
	}

	return time__;
}

float 
Comp::frameRate() const
{
	float rate = 0.0f;
	auto result = TEInstanceGetFloatFrameRate(instance_, &rate);
	if (result != TEResultSuccess)
	{
		auto severity = TEResultGetSeverity(result);
		if (severity == TESeverityError)
			spdlog::error("Failed to get frame rate: {}", TEResultGetDescription(result));
		else if (severity == TESeverityWarning)
			spdlog::warn("Failed to get frame rate: {}", TEResultGetDescription(result));
		else
			spdlog::info("Failed to get frame rate: {}", TEResultGetDescription(result));
	}
	return rate;
}

void 
Comp::start()
{
	TEResult result = TEInstanceResume(instance_);
	if (result != TEResultSuccess)
	{
		spdlog::error("Failed to resume TEInstance: {}", TEResultGetDescription(result));
		throw std::runtime_error("Failed to resume TEInstance");
	}

	// print out the flags as bits
	//std::cout << "Comp flags: " << std::bitset<32>(compFlags_()) << std::endl;
	//std::cout << "InternalTimeAuto: " << std::bitset<32>(static_cast<uint32_t>(CompFlagBits::InternalTimeAuto)) << std::endl;
	//std::cout << "InternalTimeAsync: " << std::bitset<32>(static_cast<uint32_t>(CompFlagBits::InternalTimeAsync)) << std::endl;

	if (compFlags_ & CompFlagBits::InternalTime && compFlags_ & CompFlagBits::AutoUpdate && !updateLoopRunning_)
	{
		spdlog::debug("Starting auto update");
		autoUpdate();
	}
	else if (compFlags_ & CompFlagBits::InternalTime && compFlags_ & CompFlagBits::AsyncUpdate && !asyncRunning_.load())
	{
		spdlog::debug("Starting async update");
		startAsync();
	}

	if (onStartCallback_) onStartCallback_(onStartData_);
}

void 
Comp::stop()
{
	if (compFlags_ & CompFlagBits::InternalTime && compFlags_ & CompFlagBits::AutoUpdate)
	{
		stopUpdate();
		spdlog::debug("Auto update stopped");
	}
	else if (compFlags_ & CompFlagBits::InternalTime && compFlags_ & CompFlagBits::AsyncUpdate)
	{
		stopAsync();
		spdlog::debug("Async update stopped");
	}

	TEResult result = TEInstanceSuspend(instance_);
	if (result != TEResultSuccess)
	{
		spdlog::error("Failed to suspend TEInstance: {}", TEResultGetDescription(result));
		throw std::runtime_error("Failed to suspend TEInstance");
	}

	spdlog::debug("TEInstance suspended");
	if (onStopCallback_) onStopCallback_(onStopData_);
	spdlog::default_logger()->flush();
}

void 
Comp::autoUpdate()
{
	updateLoopRunning_ = true;
	while (updateLoopRunning_)
	{
		if (frameDidFinish())
		{
			applyValueChanges();
			
			if (!callOnFrameCallback() && updateLoopRunning_) startNextFrame();
		}
	}
}

void 
Comp::stopUpdate()
{
	updateLoopRunning_ = false;
}

void 
Comp::startAsync()
{
	// need to wait before returning from this function until first frame is finished

	asyncActive_ = true;
	asyncRunning_ = true;
	asyncContinueStop_ = false;
	asyncThread_ = std::thread(&Comp::asyncUpdate, this);

	std::unique_lock<std::mutex> lock(asyncMutex_);
	asyncLayoutReadyCV_.wait(lock, [this] { return asyncLayoutReady_; });
	lock.unlock();
}

void 
Comp::stopAsync()
{
	asyncSettingCallback_ = false;
	asyncRunning_.store(false);

	{
		std::unique_lock<std::mutex> lock(asyncMutex_);
		asyncStopCV_.wait(lock, [this] { return asyncContinueStop_; });
		lock.unlock();
	}

	if (asyncThread_.joinable())
		asyncThread_.join();

	asyncActive_ = false;
}

void 
Comp::asyncUpdate()
{
	SPDLOG_DEBUG("asyncUpdate() log in thread successfull");
	SPDLOG_FLUSH_DEBUG

	cudaSetDevice(cudaDevice_);

	static uint64_t counter = 0;
	while (asyncRunning_.load())
	{
		auto state = getState();

		if (!state.loaded || !state.ready) continue;

		if (state.linksLayoutChanged)
		{
			applyLayoutChange();
			SPDLOG_DEBUG("layout changed");
			SPDLOG_FLUSH_DEBUG
			continue;
		}
		
		if (!state.inFrame)
		{
			++counter;

			applyValueChanges();

			std::unique_lock<std::mutex> lock(asyncMutex_);
			asyncCV_.wait(lock, [this] { return !asyncSettingCallback_; });

			if (!callOnFrameCallback()) startNextFrame();

			lock.unlock();
		}
	}

	{
		std::lock_guard<std::mutex> lock(asyncMutex_);
		asyncContinueStop_ = true;
		asyncStopCV_.notify_one();  // Notify stopAsync() that the loop is finished
	}
	//SPDLOG_DEBUG("asyncUpdate() finished");
	//SPDLOG_FLUSH_DEBUG
}

bool 
Comp::frameDidFinish()
{
	auto state = getState();
	if (!state.loaded || !state.ready) return false;

	if (state.linksLayoutChanged)
	{
		applyLayoutChange();
		return false;
	}
	return !state.inFrame;
}

bool 
Comp::startNextFrame(int64_t timeValue, int32_t timeScale)
{
	setInFrame(true);
	TEResult result = TEInstanceStartFrameAtTime(instance_, timeValue, timeScale, false);
	if (result != TEResultSuccess)
	{
		spdlog::error("Frame did not start successfully: {}", TEResultGetDescription(result));
		setInFrame(false);
		return false;
	}
	return true;
}

void 
Comp::applyValueChanges()
{
	changedOutputTextures_.clear();
	changedOutputFloatBuffers_.clear();
	changedOutputStringData_.clear();

	{
		std::lock_guard<std::mutex> guard(mutex_);
		std::swap(ssPendingOutputTextures_, changedOutputTextures_);
		std::swap(ssPendingOutputFloatBuffers, changedOutputFloatBuffers_);
		std::swap(ssPendingOutputStringData, changedOutputStringData_);
	}

	applyOutputTextureChange();
	applyOutputFloatBufferChange();
	applyOutputStringDataChange();
}

void
Comp::applyOutputTextureChange()
{
	for (const auto& identifier : changedOutputTextures_)
	{
		auto& topLink = *outTopLinks_->getLinkByIdentifier(identifier);
		topLink.onOutputTextureChange();
		//topLink.onOutputTextureChange(nullptr);
	}
}

void
Comp::applyOutputFloatBufferChange()
{
	if (!asyncActive_)
	{
		for (const auto& identifier : changedOutputFloatBuffers_)
		{
			auto& chopLink = *outChopLinks_->getLinkByIdentifier(identifier);
			chopLink.resetUpdated();
		}
	}
	else
	{
		for (const auto& identifier : changedOutputFloatBuffers_)
		{
			auto& chopLink = *outChopLinks_->getLinkByIdentifier(identifier);
			if (chopLink.updated())
			{
				chopLink.moveBuffer();
				chopLink.resetUpdated();
			}
		}
	}
}

void
Comp::applyOutputStringDataChange()
{
	if (!asyncActive_)
	{
		for (const auto& identifier : changedOutputStringData_)
		{
			auto& datLink = *outDatLinks_->getLinkByIdentifier(identifier);
			datLink.resetUpdated();
		}
	}
	else
	{
		for (const auto& identifier : changedOutputStringData_)
		{
			auto& datLink = *outDatLinks_->getLinkByIdentifier(identifier);
			if (datLink.updated())
			{
				datLink.moveBuffer();
				datLink.resetUpdated();
			}
		}
	}
}

void 
Comp::applyLayoutChange()
{
	if (asyncActive_)
	{
		{
			std::lock_guard<std::mutex> lock(asyncMutex_);
			asyncLayoutReady_ = false;
		}
		asyncLayoutReadyCV_.notify_one();
	}

	spdlog::debug("Applying layout change");

	inTopLinks_ = std::make_unique<InTopLinks>(instance_, renderer_->teContext(), physicalDevice_, device_, cudaStream_);
	outTopLinks_ = std::make_unique<OutTopLinks>(instance_, renderer_->teContext(), physicalDevice_, device_, cudaStream_);

	inChopLinks_ = std::make_unique<InChopLinks>(instance_);
	outChopLinks_ = std::make_unique<OutChopLinks>(instance_);

	inDatLinks_ = std::make_unique<InDatLinks>(instance_);
	outDatLinks_ = std::make_unique<OutDatLinks>(instance_);

	parLinks_ = std::make_unique <ParLinkCollection>(instance_);


	for (auto scope : { TEScopeInput, TEScopeOutput })
	{
		TouchObject<TEStringArray> groups;
		TEResult result = TEInstanceGetLinkGroups(instance_, scope, groups.take());
		if (result == TEResultSuccess)
		{
			for (int32_t i = 0; i < groups->count; i++)
			{
				TouchObject<TELinkInfo> group;
				result = TEInstanceLinkGetInfo(instance_, groups->strings[i], group.take());
				if (result == TEResultSuccess)
				{
					SPDLOG_DEBUG(teutils::getLinkInfoAsString(group));
				}
				TouchObject<TEStringArray> children;
				if (result == TEResultSuccess)
				{
					result = TEInstanceLinkGetChildren(instance_, groups->strings[i], children.take());
				}
				if (result == TEResultSuccess)
				{
					for (int32_t j = 0; j < children->count; j++)
					{
						TouchObject<TELinkInfo> info;
						result = TEInstanceLinkGetInfo(instance_, children->strings[j], info.take());
						if (result == TEResultSuccess)
						{
							SPDLOG_DEBUG(teutils::getLinkInfoAsString(info));

							if (info->type == TELinkTypeTexture)
							{
								if (info->scope == TEScopeInput)
									inTopLinks_->addLink(info);

								else if (info->scope == TEScopeOutput)
								{
									outTopLinks_->addLink(info);
									(*outTopLinks_)[outTopLinks_->size() - 1].setRequiresCudaMemLock(asyncActive_);
								}
							}

							if (info->type == TELinkTypeFloatBuffer)
							{
								if (info->scope == TEScopeInput)
									inChopLinks_->addLink(info);

								else if (info->scope == TEScopeOutput)
								{
									outChopLinks_->addLink(info);
									(*outChopLinks_)[outChopLinks_->size() - 1].setUsingSwapBuffer(asyncActive_);
								}
							}

							if (info->type == TELinkTypeStringData)
							{
								if (info->scope == TEScopeInput)
									inDatLinks_->addLink(info);

								else if (info->scope == TEScopeOutput)
								{
									outDatLinks_->addLink(info);
									(*outDatLinks_)[outDatLinks_->size() - 1].setUsingSwapBuffer(asyncActive_);
								}
							}

							if (info->domain == TELinkDomainParameter)
							{
								parLinks_->addLink(info);
							}
						}
					}
				}
			}
		}
	}

	if (asyncActive_)
	{
		{
			std::lock_guard<std::mutex> lock(asyncMutex_);
			asyncLayoutReady_ = true;
		}
		asyncLayoutReadyCV_.notify_one();
	}

	callOnLayoutChangeCallback();

	spdlog::default_logger()->flush();
	if (updateLoopRunning_ || asyncRunning_.load()) startNextFrame();
}


