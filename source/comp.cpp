#include "comp.h"
#include "teutils.h"
#include <iostream>




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

void Comp::initComp()
{
	createRenderer();
	cudaInit();
	lastFrameTime_ = std::chrono::high_resolution_clock::now();

}

Comp::~Comp()
{
	TE_CHECK(TEInstanceUnload(instance_));
	vkDestroyFence(device_, submitFence_, nullptr);
}


void Comp::createRenderer()
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


void Comp::cudaInit()
{
	setCudaDevice();
	CUDA_CHECK(cudaStreamCreate(&cudaStream_));
}

void Comp::setCudaDevice()
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

void Comp::load()
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

void Comp::loadTox(const std::string& filePath)
{
	filePath_ = filePath;
	unload();
	load();
}


void Comp::unload()
{
	
}


bool Comp::loaded()
{
	std::lock_guard<std::mutex> guard(mutex_);
	return ssLoaded_;
}


void Comp::eventCallback(TEInstance* instance,
	TEEvent event,
	TEResult result,
	int64_t start_time_value,
	int32_t start_time_scale,
	int64_t end_time_value,
	int32_t end_time_scale,
	void* info)
{
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
		comp->onEventFrameDidFinish(result, start_time_value, start_time_scale);
		break;
	case TEEventGeneral:
		comp->onEventGeneral(result, start_time_value, start_time_scale);
		break;
	default:
		break;
	}
}

void Comp::onEventInstanceReady(TEResult result)
{
	bool temp = false;
	{
		std::lock_guard<std::mutex> lock(mutex_);
		ssReady_ = result == TEResultSuccess;
	}

	std::cout << "\t\tInstance Ready: " << TEResultGetDescription(result) << std::endl;
}

void Comp::onEventInstanceDidLoad(TEResult result)
{
	std::lock_guard<std::mutex> lock(mutex_);
	ssLoaded_ = true;
}

void Comp::onEventInstanceDidUnload(TEResult result)
{
	std::cout << "Instance unloaded" << std::endl;
}

void Comp::onEventFrameDidFinish(TEResult result, int64_t time_value, int32_t time_scale)
{
	setInFrame(false);
	//std::cout << "Frame end: " << TEResultGetDescription(result)
	//	<< " time_value: " << time_value
	//	<< " time_scale: " << time_scale
	//	<< std::endl;
}

void Comp::onEventGeneral(TEResult result, uint64_t start_time, uint64_t end_time)
{
	//std::cout << "General event: " << TEResultGetDescription(result)
	//	<< " start_time: " << start_time
	//	<< " end_time: " << end_time
	//	<< std::endl;
}


void Comp::linkEventCallback(TEInstance* instance, TELinkEvent event, const char* identifier, void* info)
{
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

void Comp::onLinkLayoutChange(TELinkEvent event, const char* identifier)
{
	std::lock_guard<std::mutex> guard(mutex_);
	ssPendingLayoutChange_ = true;

}

void Comp::getState(bool& ready, bool& loaded, bool& linksChanged, bool& inFrame)
{
	std::lock_guard<std::mutex> guard(mutex_);
	loaded = ssLoaded_;
	ready = ssReady_;
	if (ssLoaded_ && ssReady_)
	{
		// For this example, we are only interested in links after load has completed
		linksChanged = ssPendingLayoutChange_;
		inFrame = ssInFrame_;
		ssPendingLayoutChange_ = false;
	}
	else
	{
		linksChanged = false;
		inFrame = false;
	}
}

void Comp::setInFrame(bool inFrame)
{
	std::lock_guard<std::mutex> guard(mutex_);
	ssInFrame_ = inFrame;
}

void Comp::applyLayoutChange()
{
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
							if (result == TEResultSuccess && info->type == TELinkTypeTexture)
							{
								if (scope == TEScopeInput)
								{
									// called before output texture is created so 
									// we can't create the texture here if it is dependent on the 
									// output texture size or format

								}
								else
								{

								}
							}
						}
					}
				}
			}
		}
	}

	//renderer_->endImageLayout();
}


void Comp::update()
{

	bool ready, loaded, linksChanged, inFrame;
	getState(ready, loaded, linksChanged, inFrame);

	if (!loaded || !ready) return;

	bool changed = linksChanged;

	if (linksChanged) applyLayoutChange();


	if (!inFrame)
	{
		changed = changed || applyOutputTextureChange();

		// Examples of setting input links
		TouchObject<TEStringArray> groups;
		TEResult result = TEInstanceGetLinkGroups(instance_, TEScopeInput, groups.take());
		if (result == TEResultSuccess)
		{
			int textureCount = 0;
			for (int32_t i = 0; i < groups->count; i++)
			{
				TouchObject<TEStringArray> children;
				result = TEInstanceLinkGetChildren(instance_, groups->strings[i], children.take());
				if (result == TEResultSuccess)
				{
					for (int32_t j = 0; j < children->count; j++)
					{
						TouchObject<TELinkInfo> info;
						result = TEInstanceLinkGetInfo(instance_, children->strings[j], info.take());
						if (result == TEResultSuccess)
						{
							switch (info->type)
							{
							case TELinkTypeDouble:
							{
								//double d = fmod(myLastFloatValue, 1.0);
								//result = TEInstanceLinkSetDoubleValue(myInstance, info->identifier, &d, 1);
								break;
							}
							case TELinkTypeInt:
							{
								//int v = static_cast<int>(myLastFloatValue * 100) % 100;
								//result = TEInstanceLinkSetIntValue(myInstance, info->identifier, &v, 1);
								break;
							}
							case TELinkTypeString:
								//result = TEInstanceLinkSetStringValue(instance_, info->identifier, "test input");
								break;
							case TELinkTypeTexture:
							{
								//if (pendingInputTexHandles_.size() > 0)
								//{
								//	auto internalTex = texturesInternal_.find(pendingInputTexHandles_[0]);
								//	if (internalTex != texturesInternal_.end())
								//	{
								//		TouchObject<TETexture> teTex;
								//		teTex.set(internalTex->second->teVkTexture());

								//		result = TEInstanceLinkSetTextureValue(
								//			instance_, info->identifier, teTex, renderer_->teContext());

								//		if (result == TEResultSuccess)
								//		{
								//			result = TEInstanceAddTextureTransfer(
								//				instance_,
								//				teTex,
								//				internalTex->second->teVkSemaphore(),
								//				internalTex->second->signalValue()
								//			);
								//		}

								//		pendingInputTexHandles_.erase(pendingInputTexHandles_.begin());
								//	}

								//	std::cout << "TELinkTypeTexture: " << info->identifier << std::endl;
								//}

								if (texToTE_.get())
								{
									TouchObject<TETexture> texture;
									texture.set(texToTE_->teVkTexture());
									result = TEInstanceLinkSetTextureValue(
										instance_, info->identifier, texture, renderer_->teContext());

									////std::cout << "TEInstanceLinkSetTextureValue: " << TEResultGetDescription(result) << std::endl;
									if (result == TEResultSuccess)
									{
										result = TEInstanceAddTextureTransfer(
											instance_, 
											texture, 
											texToTE_->teVkSemaphore(),
											texToTE_->signalValue()
										);
										std::cout << "TEInstanceAddTextureTransfer: " << info->identifier 
											<< ", " << TEResultGetDescription(result) << std::endl;
									}
								}
								break;
							}
							case TELinkTypeFloatBuffer:
							{
								//TouchObject<TEFloatBuffer> buffer;
								//// Creating a copy of an existing buffer is more efficient than creating a new one every time
								//result = TEInstanceLinkGetFloatBufferValue(instance_, info->identifier, TELinkValueCurrent, buffer.take());
								//if (result == TEResultSuccess)
								//{
								//	// You might want to check more properties of the buffer than this
								//	if (buffer && TEFloatBufferGetCapacity(buffer) < 1 || TEFloatBufferGetChannelCount(buffer) != 2)
								//	{
								//		buffer.reset();
								//	}
								//	if (buffer)
								//	{
								//		TouchObject<TEFloatBuffer> copied;
								//		copied.take(TEFloatBufferCreateCopy(buffer));
								//		buffer = copied;
								//	}
								//	else
								//	{
								//		// Two channels, capacity of one sample per channel, no channel names
								//		// This buffer is not time-dependent, see TEFloatBuffer.h for handling time-dependent samples such
								//		// as audio.
								//		buffer.take(TEFloatBufferCreate(-1, 2, 1, nullptr));
								//	}
								//	float value = static_cast<float>(fmod(myLastFloatValue, 1.0));
								//	std::array<const float*, 2> channels{ &value, &value };
								//	TEFloatBufferSetValues(buffer, channels.data(), 1);

								//	result = TEInstanceLinkSetFloatBufferValue(instance_, info->identifier, buffer);
								//}
								break;
							}
							case TELinkTypeStringData:
							{
								//String data can be either tabular, in which case set a TETable, or a single string - here we set a table
								//(use TEInstanceLinkSetStringValue() to set a string value)

								//It is more efficient to create a copy of an existing table than to create a new one, so check
								//for an existing table to re-use first.
							   //TouchObject<TEObject> value;
							   //result = TEInstanceLinkGetObjectValue(instance_, info->identifier, TELinkValueCurrent, value.take());

							   //if (result == TEResultSuccess)
							   //{
							   //	TouchObject<TETable> table;
							   //	if (value && TEGetType(value) == TEObjectTypeTable)
							   //	{
							   //		table.take(TETableCreateCopy(static_cast<TETable*>(value.get())));
							   //	}
							   //	else
							   //	{
							   //		table.take(TETableCreate());
							   //	}
							   //	TETableResize(table, 3, 2);
							   //	for (int column = 0; column < 2; column++)
							   //	{
							   //		for (int row = 0; row < 3; row++)
							   //		{
							   //			TETableSetStringValue(table, row, column, "test");
							   //		}
							   //	}
							   //	result = TEInstanceLinkSetTableValue(instance_, info->identifier, table);
							   //}
								break;
							}
							default:
								break;
							}
						}
					}
				}
			}
		}

		setInFrame(true);
		result = TEInstanceStartFrameAtTime(instance_, 0.0, 0.0, false);
		if (result != TEResultSuccess)
		{
			setInFrame(false);
		}

		std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
		std::cout << "Frame time: " << std::chrono::duration_cast<std::chrono::milliseconds>(
			now - lastFrameTime_).count() << "ms" << std::endl;

		lastFrameTime_ = now;

	}


}

bool Comp::applyOutputTextureChange()
{
	// Only hold the lock briefly
	std::vector<std::string> changes;
	{
		std::lock_guard<std::mutex> guard(mutex_);
		std::swap(pendingOutputTextures_, changes);
	}

	for (const auto& identifier : changes)
	{
		TouchObject <TETexture> teTex;
		TEResult result = TEInstanceLinkGetTextureValue(
			instance_, identifier.c_str(), TELinkValueCurrent, teTex.take());

		if (result == TEResultSuccess && TEInstanceHasTextureTransfer(instance_, teTex))
		{

			HANDLE handle = TEVulkanTextureGetHandle(static_cast<TEVulkanTexture*>(teTex.get()));
			std::cout << "Has Texture Transfer: " << identifier
				<< ", Texture Handle: " << handle << std::endl;

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
			


			//if (texFromTE_.get() == nullptr)
			//{
			//	texFromTE_ = std::make_unique<Texture>(
			//		renderer_->vContext().physicalDevice,
			//		renderer_->vContext().device,
			//		instance_,
			//		static_cast<TEVulkanTexture*>(teTex.get())
			//	);

			//	// temporary - 
			//	if (texToTE_.get() == nullptr && texFromTE_)
			//	{
			//		texToTE_ = std::make_unique<Texture>(
			//			physicalDevice_,
			//			device_,
			//			texFromTE_->extent(),
			//			texFromTE_->format()
			//		);
			//	}

			//	return true; // we need to wait for the texture to be ready
			//}

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
				std::cout << "Texture transfer: " << identifier << " : " << waitValue << std::endl;
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

	return !changes.empty();
}

void Comp::onLinkEventValueChange(const char* identifier)
{
	TouchObject<TELinkInfo> link;
	TEResult result = TEInstanceLinkGetInfo(instance_, identifier, link.take());
	if (result == TEResultSuccess && link->scope == TEScopeOutput)
	{
		switch (link->type)
		{
		case TELinkTypeTexture:
		{
			// Stash the state, we don't do any actual renderer work from this thread
			std::lock_guard<std::mutex> guard(mutex_);
			pendingOutputTextures_.push_back(identifier);

			break;
		}
		case TELinkTypeFloatBuffer:
		{
			if (strcmp(identifier, "op/chopOut3") == 0)
			{
				TouchObject<TEFloatBuffer> buffer;
				result = TEInstanceLinkGetFloatBufferValue(instance_, identifier, TELinkValueCurrent, buffer.take());

				if (result == TEResultSuccess)
				{
					uint32_t valueCount = TEFloatBufferGetValueCount(buffer);
					int32_t channelCount = TEFloatBufferGetChannelCount(buffer);
					if (buffer && channelCount > 0 && valueCount > 0)
					{
						const float* const* data = TEFloatBufferGetValues(buffer);

						std::cout << "Frame: " << data[0][0] << ", second: " << data[1][0] << std::endl;

						//static int printCount = 0;
						//if (printCount++ < 5)
						//{
						//	std::cout << "Float buffer values: ";

						//	for (int channel = 0; channel < channelCount; channel++)
						//	{
						//		// Here we just grab the first sample in the channel
						//		float value = data[channel][0];
						//		std::cout << value << ", ";
						//	}

						//	std::cout << std::endl;
						//}
					}
				}
			}
			break;
		}
		case TELinkTypeStringData:
		{
			TouchObject<TEObject> value;
			result = TEInstanceLinkGetObjectValue(instance_, identifier, TELinkValueCurrent, value.take());
			// String data can be a TETable or TEString, so check the type
			if (value && TEGetType(value) == TEObjectTypeTable)
			{
				TouchObject<TETable> table;
				table.set(static_cast<TETable*>(value.get()));
				// do something with the table 
			}
			else if (value && TEGetType(value) == TEObjectTypeString)
			{
				TouchObject<TEString> string;
				string.set(static_cast<TEString*>(value.get()));
				// do something with the string
			}
			break;
		}
		default:
			break;
		}
	}
}


