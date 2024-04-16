#include "comp.h"
#include "teutils.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <array>

#include "logging.h"

//#include <bitset>

Comp::Comp()
{
	initComp();
}

Comp::Comp(const std::string& filePath, CompFlags compFlags, int64_t fps) 
	:	compFlags_(compFlags)
{
	spdlog::info("Creating Comp");

	// use macros to log debug and trace messages so they can be turned off in release builds
	//SPDLOG_DEBUG("debug message to touchpy_logger");
	//SPDLOG_TRACE("trace message to touchpy_logger");

	initComp();
	loadTox(filePath, compFlags, fps);

	spdlog::default_logger()->flush();
}

void
Comp::initComp()
{
	createRenderer();
	cudaInit();
	initInstance();
}

Comp::~Comp()
{
	// Calling unload here can cause python to deadlock if it doesn't finish before python is closed... 
	// call unload() before destruction, or not at all but that will cause memory leaks if the object is the global scope
	//unload();

	if (cudaStream_ && compFlags_ & CompFlagBits::CudaStreamInternal) CUDA_CHECK(cudaStreamDestroy(cudaStream_));

	vkDestroyFence(device_, submitFence_, nullptr);

	spdlog::info("Comp resources destroyed");
	spdlog::default_logger()->flush();
}

void 
Comp::createRenderer()
{
	renderer_ = std::make_unique<Renderer>();
	renderer_->createInstance();
	renderer_->init();

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
	setCudaDevice();
	if (compFlags_ & CompFlagBits::CudaStreamInternal) 
	{
		CUDA_CHECK(cudaStreamCreate(&cudaStream_));
		spdlog::info("CUDA stream created: {}", static_cast<void*>(cudaStream_));
	}
}

void 
Comp::setCudaDevice()
{
	int deviceCount;
	CUDA_CHECK(cudaGetDeviceCount(&deviceCount));
	if (deviceCount == 0)
	{
		std::cerr << "No CUDA devices found" << std::endl;
		exit(1);
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
				spdlog::info("Set CUDA device: {} : {} with compute {}", device, deviceProp.name, deviceProp.major, deviceProp.minor);

				cudaDevice_ = device;
				return;
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
		std::cerr << "No Vulkan/CUDA interop capable device found" << std::endl;
		exit(1);
	}

	std::cerr << "No CUDA device found with matching UUID" << std::endl;
	exit(1);
}

bool 
Comp::initInstance()
{
	TE_CHECK(TEInstanceCreate(eventCallback, linkEventCallback, this, instance_.take()));
	spdlog::info("TouchEngine instance created");

	TE_CHECK(TEInstanceAssociateGraphicsContext(instance_, renderer_->teContext()));
	spdlog::info("TouchEngine instance associated with Graphics Context");

	return true;
}

bool Comp::loadTox(const std::string& filePath, int64_t fps)
{
	return loadTox(filePath, compFlags_, fps);
}

bool Comp::loadTox(const std::string& filePath, CompFlags compFlags, int64_t fps)
{
	compFlags_ = compFlags;
	TE_CHECK(TEInstanceSetFrameRate(instance_, fps, 1));

	filePath_ = filePath;
	spdlog::info("Loading tox: {}", filePath_);

	auto timeMode = TETimeInternal;
	if (compFlags_ & CompFlagBits::ExternalTime) timeMode = TETimeExternal;

	TE_CHECK(TEInstanceConfigure(instance_, filePath_.c_str(), timeMode));
	spdlog::info("Instance configured");

	std::unique_lock<std::mutex> lock(mutex_);
	TE_CHECK(TEInstanceLoad(instance_));
	spdlog::info("Instance loading...");

	spdlog::default_logger()->flush();
	// wait for instance to load
	cv_.wait(lock, [this] { return ssReady_; });

	return ssReady_;
}

void 
Comp::unload()
{
	if (asyncRunning_.load()) stopAsync();
	else if (updateLoopRunning_) stopUpdate();


	std::unique_lock<std::mutex> lock(mutex_);
	if (ssLoaded_)
	{
		spdlog::info("Unloading TouchEngine instance...");

		ssUnloading_ = true;
		onFrameCallbackUserData_ = nullptr;
		onFrameCallback_ = nullptr;
		onLayoutChangeCallbackUserData_ = nullptr;
		onLayoutChangeCallback_ = nullptr;

		cudaStreamSynchronize(cudaStream_);


		lock.unlock();
		TE_CHECK(TEInstanceUnload(instance_));

		lock.lock();
		cv_.wait(lock, [this] { return !ssLoaded_; });
	}
	//spdlog::default_logger()->flush();
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
	// std::cout << "eventCallback: " << teutils::eventToString(event) << " result: " << TEResultGetDescription(result) << std::endl;
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
	
	std::unique_lock<std::mutex> lock(mutex_);

	comp->ssReady_ = result == TEResultSuccess;
	comp->cv_.notify_one(); // notify load() that instance is ready
	lock.unlock();
	spdlog::info("Instance Ready: {}", TEResultGetDescription(result));
}

void 
Comp::onEventInstanceDidLoad(TEResult result, Comp* comp)
{
	std::lock_guard<std::mutex> lock(comp->mutex_);
	comp->ssLoaded_ = true;
}

void 
Comp::onEventInstanceDidUnload(TEResult result, Comp* comp)
{
	ssUnloading_ = false;
	ssLoaded_ = false;
	ssReady_ = false;
	comp->cv_.notify_one(); // notify unload() that instance is unloaded
	spdlog::info("Instance Unloaded: {}", TEResultGetDescription(result));
}

void 
Comp::onEventFrameDidFinish(TEResult result, int64_t start_time_value, int32_t start_time_scale, 
	int64_t end_time_value, int32_t end_time_scale, Comp* comp)
{
	
	if (result == TEResultSuccess && start_time_value >= 0)
	{
		comp->setInFrame(false);
	}
	else
	{
		if(result != TEResultCancelled)
		{
			if (result == TEResultComponentErrors || result == TEResultComponentWarnings) comp->setInFrame(false);


			else
			{
				// need go through all possible results and handle them accordingly... 
				// create switch...
				// 
				//std::string error = TEResultGetDescription(result);
				//error = "Frame did not finish successfully: " + error;
				//throw std::runtime_error("Frame did not finish successfully");
				spdlog::error("Frame did not finish successfully: {}", TEResultGetDescription(result));
				startNextFrame(prevTimeValue_, prevTimeScale_);
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

void
Comp::getState(bool& ready, bool& loaded, bool& linksLayoutChanged, bool& inFrame)
{
	std::lock_guard<std::mutex> guard(mutex_);
	loaded = ssLoaded_;
	ready = ssReady_;
	if (ssLoaded_ && ssReady_)
	{
		linksLayoutChanged = ssPendingLayoutChange_;
		inFrame = ssInFrame_;
		ssPendingLayoutChange_ = false;
	}
	else
	{
		linksLayoutChanged = false;
		inFrame = false;
	}
}

void 
Comp::setInFrame(bool inFrame)
{
	std::unique_lock<std::mutex> lock(mutex_);
	ssInFrame_ = inFrame;

	if (asyncActive_) cv_.notify_one();
	lock.unlock();
}

void Comp::setOnFrameCallback(std::function<void(Comp&, std::shared_ptr<void>)> callback, std::shared_ptr<void> userData)
{
	if (!asyncActive_)
	{
		onFrameCallback_ = nullptr;
		onFrameCallbackUserData_ = nullptr;

		onFrameCallback_ = callback;
		onFrameCallbackUserData_ = userData;
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
		onFrameCallbackUserData_ = nullptr;

		onFrameCallback_ = callback;
		onFrameCallbackUserData_ = userData;

		if (asyncRunning_.load())
		{
			asyncSettingCallback_ = false;
			asyncCV_.notify_one();
		}

		lock.unlock();
	}
}


void Comp::clearOnFrameCallback()
{
	if (!asyncActive_)
	{
		onFrameCallback_ = nullptr;
		onFrameCallbackUserData_ = nullptr;
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
		onFrameCallbackUserData_ = nullptr;

		if (asyncRunning_.load())
		{
			asyncSettingCallback_ = false;
			asyncCV_.notify_one();
		}

		lock.unlock();
	}
}

bool Comp::callOnFrameCallback()
{
	if (onFrameCallback_)
	{
		onFrameCallback_(*this, onFrameCallbackUserData_);
		return true;
	}
	return false;
}

void Comp::setOnLayoutChangeCallback( std::function<void(Comp&, std::shared_ptr<void>)> callback, std::shared_ptr<void> userData)
{
	if (!asyncActive_)
	{
		onLayoutChangeCallback_ = nullptr;
		onLayoutChangeCallbackUserData_ = nullptr;

		onLayoutChangeCallback_ = callback;
		onLayoutChangeCallbackUserData_ = userData;
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
		onLayoutChangeCallbackUserData_ = nullptr;

		onLayoutChangeCallback_ = callback;
		onLayoutChangeCallbackUserData_ = userData;

		if (asyncRunning_.load())
		{
			asyncSettingCallback_ = false;
			asyncCV_.notify_one();
		}

		lock.unlock();
	}
}

void Comp::clearOnLayoutChangeCallback()
{
	if (!asyncActive_)
	{
		onLayoutChangeCallback_ = nullptr;
		onLayoutChangeCallbackUserData_ = nullptr;
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
		onLayoutChangeCallbackUserData_ = nullptr;

		if (asyncRunning_.load())
		{
			asyncSettingCallback_ = false;
			asyncCV_.notify_one();
		}

		lock.unlock();
	}
}

bool Comp::callOnLayoutChangeCallback()
{
	if (onLayoutChangeCallback_)
	{
		onLayoutChangeCallback_(*this, onLayoutChangeCallbackUserData_);
		return true;
	}
	return false;
}

void Comp::start()
{
	TE_CHECK(TEInstanceResume(instance_));

	// print out the flags as bits
	//std::cout << "Comp flags: " << std::bitset<32>(compFlags_()) << std::endl;
	//std::cout << "InternalTimeAuto: " << std::bitset<32>(static_cast<uint32_t>(CompFlagBits::InternalTimeAuto)) << std::endl;
	//std::cout << "InternalTimeAsync: " << std::bitset<32>(static_cast<uint32_t>(CompFlagBits::InternalTimeAsync)) << std::endl;

	if (compFlags_ & CompFlagBits::InternalTime && compFlags_ & CompFlagBits::AutoUpdate && !updateLoopRunning_)
	{
		spdlog::info("Starting auto update");
		autoUpdate();
	}
	else if (compFlags_ & CompFlagBits::InternalTime && compFlags_ & CompFlagBits::AsyncUpdate && !asyncRunning_.load())
	{
		spdlog::info("Starting async update");
		startAsync();
	}
}

void Comp::stop()
{
	if (compFlags_ & CompFlagBits::InternalTime && compFlags_ & CompFlagBits::AutoUpdate)
	{
		stopUpdate();
		spdlog::info("auto update stopped");
	}
	else if (compFlags_ & CompFlagBits::InternalTime && compFlags_ & CompFlagBits::AsyncUpdate)
	{
		stopAsync();
		spdlog::info("async update stopped");
	}

	TE_CHECK(TEInstanceSuspend(instance_));
	spdlog::default_logger()->flush();
}

void Comp::autoUpdate()
{
	updateLoopRunning_ = true;
	while (updateLoopRunning_)
	{
		if (frameDidFinish())
		{
			applyValueChanges();
			
			if (!callOnFrameCallback() && updateLoopRunning_) startNextFrame(prevTimeValue_, prevTimeScale_);
		}
	}
}

void Comp::stopUpdate()
{
	updateLoopRunning_ = false;
}

void Comp::startAsync()
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

void Comp::stopAsync()
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

void Comp::asyncUpdate()
{
	SPDLOG_DEBUG("asyncUpdate() log in thread successfull");
	SPDLOG_FLUSH_DEBUG
	static uint64_t counter = 0;
	while (asyncRunning_.load())
	{
		bool ready, loaded, linksLayoutChanged, inFrame;
		getState(ready, loaded, linksLayoutChanged, inFrame);

		if (!loaded || !ready) continue;

		if (linksLayoutChanged)
		{
			applyLayoutChange();
			SPDLOG_DEBUG("layout changed");
			SPDLOG_FLUSH_DEBUG
			continue;
		}
		
		if (!inFrame)
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
}

bool 
Comp::frameDidFinish()
{
	bool ready, loaded, linksLayoutChanged, inFrame;
	getState(ready, loaded, linksLayoutChanged, inFrame);

	if (!loaded || !ready) return false;

	if (linksLayoutChanged)
	{
		applyLayoutChange();
		return false;
	}
	return !inFrame;
}

bool Comp::startNextFrame(int64_t timeValue, int32_t timeScale)
{
	prevTimeValue_ = timeValue;
	prevTimeScale_ = timeScale;

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

void Comp::applyValueChanges()
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

	spdlog::info("Applying layout change");

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
					// Use group info here
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
							spdlog::debug(getLinkInfoAsString(info));

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
	if (updateLoopRunning_ || asyncRunning_.load()) startNextFrame(prevTimeValue_, prevTimeScale_);
}

std::string Comp::getLinkInfoAsString(TouchObject<TELinkInfo> info)
{
	std::stringstream ss;
	ss << std::left
		<< std::setw(6) << "Link:" << std::setw(16) << info->identifier
		<< std::setw(6) << "name:" << std::setw(16) << info->name
		<< std::setw(7) << "label:" << std::setw(16) << info->label
		<< std::setw(7) << "scope:" << std::setw(16) << teutils::scopeToString(info->scope)
		<< std::setw(8) << "intent:" << std::setw(28) << teutils::linkIntentToString(info->intent)
		<< std::setw(8) << "domain:" << std::setw(24) << teutils::linkDomainToString(info->domain)
		<< std::setw(7) << "count:" << std::setw(5) << info->count
		<< std::setw(6) << "type:" << std::setw(16) << teutils::linkTypeToString(info->type)
		;

	return ss.str();
}
