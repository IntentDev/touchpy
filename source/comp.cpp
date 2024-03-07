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


Comp::Comp(const std::string& filePath)
	: filePath_(filePath)
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
	TE_CHECK(TEInstanceUnload(instance_));
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

void 
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
	TE_CHECK(TEInstanceLoad(instance_));
	std::cout << "\t\tInstance loaded!" << std::endl;

	TE_CHECK(TEInstanceResume(instance_));


}

void 
Comp::loadTox(const std::string& filePath)
{
	filePath_ = filePath;
	unload();
	load();
}


void 
Comp::unload()
{
	
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
		comp->onEventInstanceReady(result);
		break;
	case TEEventInstanceDidLoad:
		comp->onEventInstanceDidLoad(result);
		break;
	case TEEventInstanceDidUnload:
		comp->onEventInstanceDidUnload(result);
		break;
	case TEEventFrameDidFinish:
		comp->onEventFrameDidFinish(result, start_time_value, start_time_scale, end_time_value, end_time_scale);
		break;
	case TEEventGeneral:
		comp->onEventGeneral(result, start_time_value, start_time_scale);
		break;
	default:
		break;
	}
}

void 
Comp::onEventInstanceReady(TEResult result)
{
	bool temp = false;
	{
		std::lock_guard<std::mutex> lock(mutex_);
		ssReady_ = result == TEResultSuccess;
	}

	std::cout << "\t\tInstance Ready: " << TEResultGetDescription(result) << std::endl;
}

void 
Comp::onEventInstanceDidLoad(TEResult result)
{
	std::lock_guard<std::mutex> lock(mutex_);
	ssLoaded_ = true;
}

void 
Comp::onEventInstanceDidUnload(TEResult result)
{
	std::cout << "Instance unloaded" << std::endl;
}

void 
Comp::onEventFrameDidFinish(TEResult result, int64_t start_time_value, int32_t start_time_scale, int64_t end_time_value, int32_t end_time_scale)
{
	//if (doubleBufferOutputs_)
	//{
	//	for (auto& chop : outChopLinks_)
	//	{
	//		chop->updateTeBuffer();
	//		chop->swapTeBuffers();
	//	}
	//}

	if (result == TEResultSuccess && start_time_value >= 0)
	{
		setInFrame(false);
	}
	else
	{
		std::cout << "onEventFrameDidFinish result: " << TEResultGetDescription(result) << ", start_time_value: " << start_time_value << ", start_time_scale : " << start_time_scale << ", end_time_value: " << end_time_value << ", end_time_scale: " << end_time_scale << std::endl;
		setInFrame(true);
		TEResult result = TEInstanceStartFrameAtTime(instance_, 0, 0, false);
		if (result != TEResultSuccess)
		{
			std::cout << "onFrameDidFinish TEInstanceStartFrameAtTime: " << TEResultGetDescription(result) << std::endl;
			setInFrame(false);
		}
	}
}

void 
Comp::onEventGeneral(TEResult result, uint64_t start_time, uint64_t end_time)
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
			if (doubleBufferOutputs_)
			{
				OutChopLink& chopLink = *outChopLinks_->getLinkByIdentifier(link->identifier);
				chopLink.updateTeBuffer();
				chopLink.swapTeBuffers();
			}
			else
			{
				std::lock_guard<std::mutex> guard(mutex_);
				ssPendingOutputFloatBuffers.push_back(identifier);
			}
			break;
		}
		case TELinkTypeStringData:
		{
			std::lock_guard<std::mutex> guard(mutex_);
			ssPendingOutputStringData.push_back(identifier);
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
		// For this example, we are only interested in links after load has completed
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
	std::lock_guard<std::mutex> guard(mutex_);
	ssInFrame_ = inFrame;
	//std::cout << "setInFrame: " << std::boolalpha << inFrame << std::endl;
}

void 
Comp::applyLayoutChange()
{
	std:: cout << "Applying layout change" << std::endl;

	inTextureLinks_ = std::make_unique<InTextureLinks>(instance_, renderer_->teContext(), physicalDevice_, device_);
	outTextureLinks_ = std::make_unique<OutTextureLinks>(instance_, renderer_->teContext(), physicalDevice_, device_);

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
									inTextureLinks_->addLink(info);

								else if (info->scope == TEScopeOutput)
									outTextureLinks_->addLink(info);
							}

							if (info->type == TELinkTypeFloatBuffer)
							{
								if (info->scope == TEScopeInput)
									inChopLinks_->addLink(info);

								else if (info->scope == TEScopeOutput)
									outChopLinks_->addLink(info);
							}

							if (info->type == TELinkTypeStringData)
							{
								if (info->scope == TEScopeInput)
									inDatLinks_->addLink(info);

								else if (info->scope == TEScopeOutput)
									outDatLinks_->addLink(info);
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

void 
Comp::update()
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

		applyOutputTextureChange();
		applyOutputFloatBufferChange();
		//applyOutputStringDataChange();

		if (onFrameStartCallback_)
			onFrameStartCallback_(*this, onFrameStartCallbackUserData_);

		//if(updateCallback_)
		//	updateCallback_(updateCallbackUserData_);

		for (size_t i = 0; i < inTextureLinks_->size() && i < outTextureLinks_->size(); ++i)
		{
			auto outTex = (*outTextureLinks_)[i].currentTexture();
			auto& inTexLink = (*inTextureLinks_)[i];

			inTexLink.copyCudaMemoryToInputTexture(
				outTex->cudaMemory(),
				outTex->format(),
				outTex->extent(),
				outTex->cudaExtSemaphore(),
				outTex->signalValue(),
				cudaStream_);
			
			inTexLink.transferTextureToInputLink(renderer_->teContext());
		}

		//for (size_t i = 0; i < inChopLinks_->size() && i < outChopLinks_->size(); ++i)
		//{
		//	auto& outputChop = (*outChopLinks_)[i];
		//	auto& inputChop = (*inChopLinks_)[i];
		//	if (outputChop.isUpdated())
		//	{
		//		inputChop.set(outputChop.channelData(), outputChop.valueCount(), outputChop.rate(), outputChop.names());
		//	}
		//}

		//for (size_t i = 0; i < inputDatLinks_->size() && i < outputDatLinks_->size(); ++i)
		//{
		//	auto& outputDatLink = (*outputDatLinks_)[i];
		//	auto& inputDatLink = (*inputDatLinks_)[i];

		//	//if (outputDatLink.type() == DatLink::DatLinkType::Table)
		//	//	inputDatLink.set(outputDatLink.getTable());
		//	//else
		//	//	inputDatLink.set(outputDatLink.getString());

		//	if (outputDatLink.type() == DatLink::DatLinkType::Table)
		//		//inputDatLink.set(outputDatLink.asTable());
		//		inputDatLink.set(outputDatLink.asString());
		//}

		//static float testFloat = 0.0f;
		//(*parLinks_)["Float"].set(testFloat);
		//testFloat += 1.1f;

		//std::cout << "setInFrame true after update" << std::endl;
		setInFrame(true);
		TEResult result = TEInstanceStartFrameAtTime(instance_, 0.0, 0.0, false);
		if (result != TEResultSuccess)
		{
			std::cout << "update() TEInstanceStartFrameAtTime: " << TEResultGetDescription(result) << std::endl;
			setInFrame(false);
			return;
		}

		
		//std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
		//std::cout << "Frame time: " << std::chrono::duration_cast<std::chrono::milliseconds>(
		//	now - lastFrameTime_).count() << "ms" << std::endl;
		//lastFrameTime_ = now;

		++frameCount_;
	}


}

void Comp::setOnFrameStartCallback(
	std::function<void(Comp&, std::shared_ptr<void>)> callback,
	std::shared_ptr<void> userData)
{
	onFrameStartCallback_ = callback;
	onFrameStartCallbackUserData_ = userData;
}

//void Comp::setUpdateCallback(void(*callback)(void*), void* userData)
//{
//	updateCallback_ = callback;
//	updateCallbackUserData_ = userData;
//}

void Comp::runUpdateLoop()
{
	updateLoopRunning_ = true;
	while (updateLoopRunning_)
	{
		update();
	}

}

void Comp::stopUpdateLoop()
{
	updateLoopRunning_ = false;
}

void 
Comp::applyOutputTextureChange()
{
	for (const auto& identifier : changedOutputTextures_)
	{
		auto& textureLink = *outTextureLinks_->getLinkByIdentifier(identifier);
		textureLink.onOutputTextureChange(cudaStream_);
	}
}

void 
Comp::applyOutputFloatBufferChange()
{
	if (!doubleBufferOutputs_)
	{
		for (const auto& identifier : changedOutputFloatBuffers_)
		{
			auto& chop = *outChopLinks_->getLinkByIdentifier(identifier);
			chop.onOuputValueChange();
		}
	}
	else
	{
		for (const auto& identifier : changedOutputFloatBuffers_)
		{
			auto& chop = *outChopLinks_->getLinkByIdentifier(identifier);
			chop.readTeBuffer();
		}
	}
}

void 
Comp::applyOutputStringDataChange()
{
	for (const auto& identifier : changedOutputStringData_)
	{
		//std::cout << "OutputStringDataChange: " << identifier << std::endl;
		auto& datLink = *outDatLinks_->getLinkByIdentifier(identifier);
		datLink.onOuputValueChange();
	}
}

void 
Comp::setInputTextures()
{
}

void 
Comp::setInputFloatBuffers()
{
}

void 
Comp::setInputStringData()
{
}




