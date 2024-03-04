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
	//	for (auto& chop : outputChopLinks_)
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
				ChopLink& chopLink = *outputChopLinks_->getLinkByIdentifier(link->identifier);
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

	outputTextureLinks_ = std::make_unique<TextureLinks>(instance_, renderer_->teContext(), physicalDevice_, device_);
	inputTextureLinks_ = std::make_unique<TextureLinks>(instance_, renderer_->teContext(), physicalDevice_, device_);

	outputChopLinks_ = std::make_unique<ChopLinks>(instance_);
	inputChopLinks_ = std::make_unique<ChopLinks>(instance_);
	
	outputDatLinks_ = std::make_unique<DatLinks>(instance_);
	inputDatLinks_ = std::make_unique<DatLinks>(instance_);

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
								if (info->scope == TEScopeOutput)
									outputTextureLinks_->addLink(info);
									//outputTextureLinks_->addLink(info);
								
								else if (info->scope == TEScopeInput)
									inputTextureLinks_->addLink(info);
									//inputTextureLinks_->addLink(info);
							}

							if (info->type == TELinkTypeFloatBuffer)
							{
								if (info->scope == TEScopeOutput)
									outputChopLinks_->addLink(info);
								
								else if (info->scope == TEScopeInput)
									inputChopLinks_->addLink(info);
							}

							if (info->type == TELinkTypeStringData)
							{
								if (info->scope == TEScopeOutput)
									outputDatLinks_->addLink(info);
								
								else if (info->scope == TEScopeInput)
									inputDatLinks_->addLink(info);
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

	//for (auto& par : parLinks_->getLinks())
	//{
	//	std::cout << "Par: " << par.first << std::endl;
	//}

	//auto scale = std::visit(visitor<double>, (*parLinks_)["Scale"].get());
	//if (scale)
	//	std::cout << "Scale: " << scale.value() << std::endl;
	//else
	//	std::cout << "Scale: " << "not found" << std::endl;

	//double s = 2.0;
	//(*parLinks_)["Scale"].set(s);

	//// not safe
	//double scale2 = std::get<double>((*parLinks_)["Scale"].get());
	//std::cout << "Scale: " << scale2 << std::endl;

	//std::cout << "setInFrame true after layout change" << std::endl;
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
	//std::cout << "Loaded: " << std::boolalpha << loaded << " Ready: " << ready 
	// << " LinksLayoutChanged: " << linksLayoutChanged << " InFrame: " << inFrame << std::endl;

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
		applyOutputStringDataChange();

		if (texToTE_.get())
		{
			std::string identifier = "op/topIn1";
			texToTE_->transferToInputLink(instance_, renderer_->teContext(), identifier.c_str());
		}

		for (size_t i = 0; i < inputChopLinks_->size() && i < outputChopLinks_->size(); ++i)
		{
			auto& outputChop = (*outputChopLinks_)[i];
			auto& inputChop = (*inputChopLinks_)[i];
			if (outputChop.isUpdated())
			{
				inputChop.set(outputChop.channelData(), outputChop.valueCount(), outputChop.rate(), outputChop.names());
			}
		}

		for (size_t i = 0; i < inputDatLinks_->size() && i < outputDatLinks_->size(); ++i)
		{
			auto& outputDatLink = (*outputDatLinks_)[i];
			auto& inputDatLink = (*inputDatLinks_)[i];

			if (outputDatLink.type() == DatLink::DatLinkType::Table)
				inputDatLink.set(outputDatLink.getTable());
			else
				inputDatLink.set(outputDatLink.getString());
		}

		static float testFloat = 0.0f;
		(*parLinks_)["Float"].set(testFloat);
		testFloat += 1.1f;

		//std::cout << "setInFrame true after update" << std::endl;
		setInFrame(true);
		TEResult result = TEInstanceStartFrameAtTime(instance_, 0.0, 0.0, false);
		if (result != TEResultSuccess)
		{
			std::cout << "update() TEInstanceStartFrameAtTime: " << TEResultGetDescription(result) << std::endl;
			setInFrame(false);
		}

		//std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
		//std::cout << "Frame time: " << std::chrono::duration_cast<std::chrono::milliseconds>(
		//	now - lastFrameTime_).count() << "ms" << std::endl;
		//lastFrameTime_ = now;

	}


}

bool 
Comp::applyOutputTextureChange()
{

	for (const auto& identifier : changedOutputTextures_)
	{
		TouchObject <TETexture> teTex;
		TEResult result = TEInstanceLinkGetTextureValue(
			instance_, identifier.c_str(), TELinkValueCurrent, teTex.take());

		if (result == TEResultSuccess && TEInstanceHasTextureTransfer(instance_, teTex))
		{

			HANDLE handle = TEVulkanTextureGetHandle(static_cast<TEVulkanTexture*>(teTex.get()));
			//std::cout << "Has Texture Transfer: " << identifier << ", Texture Handle: " << handle << std::endl;

			auto it = texturesExternal_.find(handle);
			if (it == texturesExternal_.end())
			{
				texturesExternal_[handle] = std::make_unique<Texture>(
					renderer_->vContext().physicalDevice,
					renderer_->vContext().device,
					instance_,
					static_cast<TEVulkanTexture*>(teTex.get())
				);

				//// temporary for copy of output to input
				//texturesInternal_[handle] = std::make_unique<Texture>(
				//	physicalDevice_,
				//	device_,
				//	texturesExternal_[handle]->extent(),
				//	texturesExternal_[handle]->format()
				//);

				if (texToTE_.get() == nullptr && texturesExternal_[handle])
				{
					texToTE_ = std::make_unique<Texture>(
						physicalDevice_,
						device_,
						texturesExternal_[handle]->extent(),
						texturesExternal_[handle]->format()
					);
				}

				std::cout << "Texture created for handle: " << handle << std::endl;
				
				return true; // we need to wait for the texture to be ready
			}
			
			auto texExternal = texturesExternal_[handle].get();

			TouchObject<TESemaphore> teSemaphore;
			//teSemaphore.set(texFromTE_->teVkSemaphore());
			teSemaphore.set(texExternal->teVkSemaphore());

			uint64_t waitValue = 0;
			result = TEInstanceGetTextureTransfer(
				instance_, 
				teTex, 
				teSemaphore.take(),
				&waitValue);


			// use if using the texture after transfer is complete, such as for display but need
			// use queue index that supports graphics... 
			//VkImageLayout srcLayout;
			//VkImageLayout dstLayout;

			//result = TEInstanceGetVulkanTextureTransfer(
			//	instance_,
			//	teTex,
			//	&srcLayout,
			//	&dstLayout,
			//	teSemaphore.take(),
			//	&waitValue
			//);

			//std::cout << "srcLayout: " << string_VkImageLayout(srcLayout)
			//	<< " dstLayout: " << string_VkImageLayout(dstLayout) << std::endl;

			//std::cout << "TESemaphore: " << teSemaphore.get() << " waitValue: " << waitValue << std::endl;

			if (result == TEResultSuccess)
			{
				//std::cout << "Texture transfer: " << identifier << " : " << waitValue << std::endl;
				if (TESemaphoreGetType(teSemaphore) == TESemaphoreTypeVulkan)
				{
					texExternal->copyImageToCudaMem(waitValue, cudaStream_);

					uint64_t signalValue;
					VK_CHECK(vkGetSemaphoreCounterValue(device_, texToTE_->semaphore(), &signalValue));
					texToTE_->setSignalValue(++signalValue);

					texToTE_->copyCudaMemToImage(
						texExternal->cudaMemory(),
						texExternal->cudaExtSemaphore(),
						texToTE_->cudaExtSemaphore(),
						waitValue,
						signalValue,
						cudaStream_);
						

					//// wait for semaphore
					//VkSemaphoreWaitInfoKHR waitInfo = {};
					//waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO_KHR;
					//waitInfo.pNext = nullptr;
					//waitInfo.flags = 0;
					//waitInfo.semaphoreCount = 1;
					////VkSemaphore importSemaphore = texFromTE_->semaphore();
					//VkSemaphore importSemaphore = texExternal->semaphore();
					//waitInfo.pSemaphores = &importSemaphore;
					//waitInfo.pValues = &waitValue;

					//VK_CHECK(vkWaitSemaphores(device_, &waitInfo, UINT64_MAX));

					//// reset command buffer
					//VK_CHECK(vkResetCommandBuffer(commandBuffer_, 0));
			
					//// copy to texToTE_
					//VkCommandBufferBeginInfo beginInfo = {};
					//beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
					//beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
					//beginInfo.pInheritanceInfo = nullptr;


					//VK_CHECK(vkBeginCommandBuffer(commandBuffer_, &beginInfo));

					////if (texFromTE_ && !srcInitialized_)
					//if (texExternal && texExternal->imageLayout() != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
					//{
					//	// transition to transfer src optimal

					//	//texFromTE_->cmdTransitionImageLayout(
					//	texExternal->cmdTransitionImageLayout(
					//		commandBuffer_,
					//		VK_IMAGE_LAYOUT_UNDEFINED,
					//		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
					//	);

					//	//srcInitialized_ = true;
					//	//std::cout << "srcInitialized_" << std::endl;
					//}
					////else
					////{
					////	texFromTE_->cmdTransitionImageLayout(
					////		commandBuffer_,
					////		srcLayout,
					////		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
					////	);
					////}


					//if (texToTE_->imageLayout() != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
					////if (texInternal && texInternal->imageLayout() != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
					//{
					//	// transition to transfer dst optimal
					//	texToTE_->cmdTransitionImageLayout(
					//		commandBuffer_,
					//		VK_IMAGE_LAYOUT_UNDEFINED,
					//		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
					//	);
					//	//dstInitialized_ = true;
					//	//std::cout << "dstInitialized_" << std::endl;
					//}

					//VkImageSubresourceLayers subresourceLayers = {};
					//subresourceLayers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					//subresourceLayers.mipLevel = 0;
					//subresourceLayers.baseArrayLayer = 0;
					//subresourceLayers.layerCount = 1;

					//VkImageCopy imageCopy = {};
					//imageCopy.srcSubresource = subresourceLayers;
					//imageCopy.dstSubresource = subresourceLayers;
					//imageCopy.extent = { texToTE_->extent().width, texToTE_->extent().height, 1 };


					//vkCmdCopyImage(
					//	commandBuffer_,
					//	//texFromTE_->image(),
					//	texExternal->image(),
					//	VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					//	texToTE_->image(),
					//	//texInternal->image(),
					//	VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					//	1,
					//	&imageCopy
					//);

					//VK_CHECK(vkEndCommandBuffer(commandBuffer_));

					//uint64_t signalValue;
					//VK_CHECK(vkGetSemaphoreCounterValue(device_, texToTE_->semaphore(), &signalValue));
					//texToTE_->setSignalValue(++signalValue);
					////texInternal->setSignalValue(++signalValue);


					//VkTimelineSemaphoreSubmitInfo timelineSemaphoreSubmitInfo = {};
					//timelineSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
					//timelineSemaphoreSubmitInfo.pNext = nullptr;
					//timelineSemaphoreSubmitInfo.waitSemaphoreValueCount = 1; 
					//timelineSemaphoreSubmitInfo.pWaitSemaphoreValues = &waitValue; 
					//timelineSemaphoreSubmitInfo.signalSemaphoreValueCount = 1; 
					//timelineSemaphoreSubmitInfo.pSignalSemaphoreValues = &signalValue; 

					//VkSubmitInfo submitInfo = {};
					//submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
					//submitInfo.pNext = &timelineSemaphoreSubmitInfo; 
					//submitInfo.commandBufferCount = 1;
					//submitInfo.pCommandBuffers = &commandBuffer_;

					//submitInfo.waitSemaphoreCount = 1;
					//submitInfo.pWaitSemaphores = &importSemaphore;
					//VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_TRANSFER_BIT }; 
					//submitInfo.pWaitDstStageMask = waitStages;

					////	
					//submitInfo.signalSemaphoreCount = 1;
					//VkSemaphore signalSemaphores[] = { texToTE_->semaphore() };
					////VkSemaphore signalSemaphores[] = { texInternal->semaphore() };
					//submitInfo.pSignalSemaphores = signalSemaphores;

					//VK_CHECK(vkQueueSubmit(queue_, 1, &submitInfo, submitFence_));
					//VK_CHECK(vkWaitForFences(device_, 1, &submitFence_, VK_TRUE, UINT64_MAX));
					//VK_CHECK(vkResetFences(device_, 1, &submitFence_));
					



					//std::cout << " copied texture ";

				}
			}
		}
	}

	return !changedOutputTextures_.empty();
}

void 
Comp::applyOutputFloatBufferChange()
{
	if (!doubleBufferOutputs_)
	{
		for (const auto& identifier : changedOutputFloatBuffers_)
		{
			auto& chop = *outputChopLinks_->getLinkByIdentifier(identifier);
			chop.onOuputValueChange();
		}
	}
	else
	{
		for (const auto& identifier : changedOutputFloatBuffers_)
		{
			auto& chop = *outputChopLinks_->getLinkByIdentifier(identifier);
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
		auto& datLink = *outputDatLinks_->getLinkByIdentifier(identifier);
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




