#include "comp.h"
#include "teutils.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <array>

Comp::Comp()
{
	initComp();
}


Comp::Comp(const std::string& filePath, bool freeRunning)
	: filePath_(filePath), freeRunning_(freeRunning)
{	
	initComp();
	load();
}

void
Comp::initComp()
{
	createRenderer();
	cudaInit();
	lastFrameTime_ = std::chrono::high_resolution_clock::now();

	// print thread id
	// std::cout << "Comp thread id: " << std::this_thread::get_id() << std::endl;

}

Comp::~Comp()
{
	unload();

	CUDA_CHECK(cudaStreamDestroy(cudaStream_));

	vkDestroyFence(device_, submitFence_, nullptr);
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
	CUDA_CHECK(cudaStreamCreate(&cudaStream_));
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
				std::cout << "CUDA device: " << device << " : " << deviceProp.name
					<< " with compute " << deviceProp.major << deviceProp.minor << std::endl;

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
Comp::load()
{
	std::cout << "Loading tox: \t" << std::string(filePath_.begin(), filePath_.end()) << std::endl;

	TE_CHECK(TEInstanceCreate(eventCallback, linkEventCallback, this, instance_.take()));
	std::cout << "\t\tInstance created!" << std::endl;

	TE_CHECK(TEInstanceAssociateGraphicsContext(instance_, renderer_->teContext()));
	std::cout << "\t\tInstance associated with Graphics Context!" << std::endl;

	TE_CHECK(TEInstanceConfigure(instance_, filePath_.c_str(), TETimeInternal));
	std::cout << "\t\tInstance configured!" << std::endl;

	TE_CHECK(TEInstanceSetFrameRate(instance_, framesPerSecond_, 1));

	std::unique_lock<std::mutex> lock(mutex_);
	TE_CHECK(TEInstanceLoad(instance_));
	std::cout << "\t\tInstance loading..." << std::endl;

	// wait for instance to load
	cv_.wait(lock, [this] { return ssReady_; });

	return ssReady_;
}

bool
Comp::loadTox(const std::string& filePath)
{
	filePath_ = filePath;
	unloadTox();
	return load();
}

bool
Comp::unloadTox()
{
	// need to implement this
	return true;
}

void 
Comp::unload()
{
	clearOnFrameStartCallback();

	if (freeRunning_) stopFreeRunning();
	else if (updateLoopRunning_) stopUpdateLoop();

	std::unique_lock<std::mutex> lock(mutex_);
	if (ssLoaded_)
	{

		std::cout << "Unloading TouchEngine instance..." << std::endl;
		ssUnloading_ = true;

		lock.unlock();
		TE_CHECK(TEInstanceUnload(instance_));

		lock.lock();
		cv_.wait(lock, [this] { return !ssLoaded_; });
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
	//std::cout << "eventCallback thread id: " << std::this_thread::get_id() << std::endl;
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

	std::cout << "\t\tInstance Ready: " << TEResultGetDescription(result) << std::endl;

	TE_CHECK(TEInstanceResume(instance_));
	if (comp->freeRunning_)
	{
		comp->startFreeRunning();
	}
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
	std::cout << "Unloaded TouchEngine!" << std::endl;
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
			std::cout << "onEventFrameDidFinish result: " << TEResultGetDescription(result) 
				<< ", start_time_value: " << start_time_value << ", start_time_scale : " << start_time_scale 
				<< ", end_time_value: " << end_time_value << ", end_time_scale: " << end_time_scale << std::endl;

			comp->setInFrame(true);
			TEResult result = TEInstanceStartFrameAtTime(comp->instance_, 0, 0, false);
			if (result != TEResultSuccess)
			{
				std::cout << "onFrameDidFinish TEInstanceStartFrameAtTime: " << TEResultGetDescription(result) << std::endl;
				comp->setInFrame(false);
			}
		}
	}
	//if (comp->freeRunning_)
	//{
	//	TEResult result = TEInstanceStartFrameAtTime(instance_, 0, 0, false);
	//}
}

void 
Comp::onEventGeneral(TEResult result, uint64_t start_time, uint64_t end_time, Comp* comp)
{
	//std::cout << "General event: " << TEResultGetDescription(result)
	//	<< " start_time: " << start_time
	//	<< " end_time: " << end_time
	//	<< std::endl;
}


void 
Comp::linkEventCallback(TEInstance* instance, TELinkEvent event, const char* identifier, void* info)
{
	//std::cout << "linkEventCallback thread id: " << std::this_thread::get_id() << std::endl;
	//std::cout << "Link event: " << teutils::linkEventToString(event) << " identifier: " << identifier << std::endl;
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
			if (usingSwapBuffer_)
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
			if (usingSwapBuffer_)
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

void Comp::printLinkInfo(TouchObject<TELinkInfo> info)
{
	std::cout << std::left
		<< std::setw(6) << "Link:" << std::setw(16) << info->identifier
		<< std::setw(6) << "name:" << std::setw(16) << info->name
		<< std::setw(7) << "label:" << std::setw(16) << info->label
		<< std::setw(7) << "scope:" << std::setw(16) << teutils::scopeToString(info->scope)
		<< std::setw(8) << "intent:" << std::setw(28) << teutils::linkIntentToString(info->intent)
		<< std::setw(8) << "domain:" << std::setw(24) << teutils::linkDomainToString(info->domain)
		<< std::setw(7) << "count:" << std::setw(5) << info->count
		<< std::setw(6) << "type:" << std::setw(16) << teutils::linkTypeToString(info->type)
		<< std::endl;
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

	if (usingSwapBuffer_) cv_.notify_one();
	
}


void Comp::setOnFrameStartCallback(
	std::function<void(Comp&, std::shared_ptr<void>)> callback,
	std::shared_ptr<void> userData)
{
	std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);

	if (usingSwapBuffer_) 
	{
		lock.lock();
		cv_.wait(lock, [this] { return ssInFrame_; });
	}

	
	onFrameStartCallback_ = callback;
	onFrameStartCallbackUserData_ = userData;

}

void Comp::clearOnFrameStartCallback()
{
	std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);

	if (usingSwapBuffer_)
	{
		lock.lock();
		cv_.wait(lock, [this] { return ssInFrame_; });
	}

	onFrameStartCallback_ = nullptr;
	onFrameStartCallbackUserData_ = nullptr;
}

void Comp::runUpdateLoop(bool updateStartsNextFrame)
{
	updateLoopRunning_ = true;
	while (updateLoopRunning_)
	{
		update(updateStartsNextFrame);
	}
}

void Comp::stopUpdateLoop()
{
	updateLoopRunning_ = false;
}

void Comp::startFreeRunning()
{
	usingSwapBuffer_ = true;
	frRunning_ = true;
	frThread_ = std::thread(&Comp::frUpdateLoop, this);

}


void Comp::stopFreeRunning()
{
	frRunning_.store(false);
	if (frThread_.joinable())
		frThread_.join();
	freeRunning_ = false;
}

//extern void
//safeCallPythonCallback(Comp* comp, std::function<void(Comp&, std::shared_ptr<void>)> callback, std::shared_ptr<void> userData);

void Comp::frUpdateLoop()
{
	while (frRunning_.load())
	{
		bool ready, loaded, linksLayoutChanged, inFrame;
		getState(ready, loaded, linksLayoutChanged, inFrame);

		if (!loaded || !ready) continue;

		if (linksLayoutChanged)
		{
			applyLayoutChange();
			ready_ = ready;
			continue;
		}
		
		if (!inFrame)
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

			applyValueChanges();

			if (onFrameStartCallback_)
				onFrameStartCallback_(*this, onFrameStartCallbackUserData_);

			startNextFrame();
		}
	}
}

void
Comp::update(bool callStartNextFrame)
{
	bool ready, loaded, linksLayoutChanged, inFrame;
	getState(ready, loaded, linksLayoutChanged, inFrame);

	if (!loaded || !ready) return;

	if (linksLayoutChanged)
	{
		applyLayoutChange();
		ready_ = ready;
		return;
	}

	if (!inFrame)
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

		applyValueChanges();

		if (onFrameStartCallback_)
			onFrameStartCallback_(*this, onFrameStartCallbackUserData_);

		if (callStartNextFrame)
			startNextFrame();
	}
}

bool Comp::startNextFrame()
{
	setInFrame(true);
	TEResult result = TEInstanceStartFrameAtTime(instance_, 0.0, 0.0, false);
	if (result != TEResultSuccess)
	{
		std::cout << "update() TEInstanceStartFrameAtTime: " << TEResultGetDescription(result) << std::endl;
		setInFrame(false);
		return false;
	}

	++frameCount_;
	return true;
}

void 
Comp::applyLayoutChange()
{
	std:: cout << "Applying layout change" << std::endl;

	inTopLinks_ = std::make_unique<InTopLinks>(instance_, renderer_->teContext(), physicalDevice_, device_);
	outTopLinks_ = std::make_unique<OutTopLinks>(instance_, renderer_->teContext(), physicalDevice_, device_);

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
							printLinkInfo(info);
							
							if (info->type == TELinkTypeTexture)
							{
								if (info->scope == TEScopeInput)
									inTopLinks_->addLink(info);

								else if (info->scope == TEScopeOutput)
								{
									outTopLinks_->addLink(info);
									(*outTopLinks_)[outTopLinks_->size() - 1].setRequiresCudaMemLock(usingSwapBuffer_);
								}
							}

							if (info->type == TELinkTypeFloatBuffer)
							{
								if (info->scope == TEScopeInput)
									inChopLinks_->addLink(info);

								else if (info->scope == TEScopeOutput)
								{
									outChopLinks_->addLink(info);
									(*outChopLinks_)[outChopLinks_->size() - 1].setUsingSwapBuffer(usingSwapBuffer_);
								}
							}

							if (info->type == TELinkTypeStringData)
							{
								if (info->scope == TEScopeInput)
									inDatLinks_->addLink(info);

								else if (info->scope == TEScopeOutput)
								{
									outDatLinks_->addLink(info);
									(*outDatLinks_)[outDatLinks_->size() - 1].setUsingSwapBuffer(usingSwapBuffer_);
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

	setInFrame(true);
	TEResult result = TEInstanceStartFrameAtTime(instance_, 0, 0, false);
	if (result != TEResultSuccess)
	{
		std::cout << "Layout Change TEInstanceStartFrameAtTime: " << TEResultGetDescription(result) << std::endl;
		setInFrame(false);
	}
}

void Comp::applyValueChanges()
{
	applyOutputTextureChange();
	applyOutputFloatBufferChange();
	applyOutputStringDataChange(); 
}

void 
Comp::applyOutputTextureChange()
{
	for (const auto& identifier : changedOutputTextures_)
	{
		auto& textureLink = *outTopLinks_->getLinkByIdentifier(identifier);
		//textureLink.onOutputTextureChange(cudaStream_);
		textureLink.onOutputTextureChange(nullptr);
	}
}

void 
Comp::applyOutputFloatBufferChange()
{
	if (!usingSwapBuffer_)
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
	if (!usingSwapBuffer_)
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

