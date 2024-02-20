#include "tdtox.h"
#include "teutils.h"
#include <iostream>






TdTox::TdTox(std::string filePath)
	: filePath_(filePath)
{
	createRenderer();
	cudaInit();
}

TdTox::~TdTox()
{
	vkDestroyFence(device_, submitFence_, nullptr);
}

void TdTox::createRenderer()
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


void TdTox::cudaInit()
{
	setCudaDevice();
	CUDA_CHECK(cudaStreamCreate(&cudaStream_));
}

void TdTox::setCudaDevice()
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

void TdTox::load()
{
	std::cout << "Loading tox file: " << std::string(filePath_.begin(), filePath_.end()) << std::endl;

	TEResult teresult = TEInstanceCreate(eventCallback, linkEventCallback, this, instance_.take());

	if (teresult == TEResultSuccess)
	{
		std::cout << "Instance created!" << std::endl;
		teresult = TEInstanceAssociateGraphicsContext(instance_, renderer_->teContext());
	}
	else
	{
		std::cout << "Failed to create instance" << std::endl;
		std::cout << TEResultGetDescription(teresult) << std::endl;
		return;
	}

	if (teresult == TEResultSuccess)
	{
		std::cout << "Instance associated with Graphics Context!" << std::endl;
		teresult = TEInstanceConfigure(instance_, filePath_.c_str(), TETimeInternal);
	}
	else
	{
		std::cout << "Failed to associate instance with Graphics Context" << std::endl;
		std::cout << TEResultGetDescription(teresult) << std::endl;
		return;
	}

	if (teresult == TEResultSuccess)
	{
		std::cout << "Instance configured!" << std::endl;
		teresult = TEInstanceSetFrameRate(instance_, framesPerSecond_, 1);
	}
	else
	{
		std::cout << "Failed to configure instance" << std::endl;
		std::cout << TEResultGetDescription(teresult) << std::endl;
		return;
	}

	if (teresult == TEResultSuccess)
	{
		teresult = TEInstanceLoad(instance_);
	}
	else
	{
		std::cout << "Failed to set frame rate" << std::endl;
		std::cout << TEResultGetDescription(teresult) << std::endl;
		return;
	}

	if (teresult == TEResultSuccess)
	{
		std::cout << "Instance loaded!" << std::endl;
		teresult = TEInstanceResume(instance_);
	}
	else
	{
		std::cout << "Failed to load instance" << std::endl;
		std::cout << TEResultGetDescription(teresult) << std::endl;
		return;
	}

	if (teresult != TEResultSuccess)
	{
		std::cout << "Failed to resume instance" << std::endl;
		std::cout << TEResultGetDescription(teresult) << std::endl;
		return;
	}



	//assert(teresult == TEResultSuccess);

	//const auto interval = static_cast<unsigned int>(std::ceil(1000. / framesPerSecond_ / 2.));


}


void TdTox::didConfigure(TEResult result)
{
	// Configuration can be cancelled by a subsequent configuration or other action
	// - we can ignore the event in that case and await a following one
	if (result != TEResultCancelled)
	{
		std::lock_guard<std::mutex> guard(mutex_);
		configureRenderer_ = true;
		configureResult_ = result;
	}
}

void TdTox::eventCallback(TEInstance* instance,
	TEEvent event,
	TEResult result,
	int64_t start_time_value,
	int32_t start_time_scale,
	int64_t end_time_value,
	int32_t end_time_scale,
	void* info)
{

	TdTox* tdTox = static_cast<TdTox*>(info);

	switch (event)
	{
	case TEEventInstanceReady:
		std::cout << "Instance ready" << std::endl;
		break;
	case TEEventInstanceDidLoad:
		tdTox->loaded();
		break;
	case TEEventInstanceDidUnload:
		break;
	case TEEventFrameDidFinish:
		tdTox->endFrame(start_time_value, start_time_scale, result);
		break;
	case TEEventGeneral:
		// TODO: check result here
		break;
	default:
		break;
	}
}

void TdTox::linkEventCallback(TEInstance* instance, TELinkEvent event, const char* identifier, void* info)
{
	//std::cout << "Link event: " << teutils::linkEventToString(event) << " identifier: " << identifier << std::endl;
	TdTox* tdTox = static_cast<TdTox*>(info);
	switch (event)
	{
	case TELinkEventAdded:
		tdTox->linkLayoutDidChange(event, identifier);
		break;
	case TELinkEventValueChange:
		tdTox->linkValueChange(identifier);
		break;
	default:
		break;
	}
}

void TdTox::endFrame(int64_t time_value, int32_t time_scale, TEResult result)
{
	setInFrame(false);
}

void TdTox::getState(bool& configured, bool& loaded, bool& linksChanged, bool& inFrame)
{
	std::lock_guard<std::mutex> guard(mutex_);
	configured = configureRenderer_;
	configureRenderer_ = false;
	loaded = loaded_;
	if (loaded_)
	{
		// For this example, we are only interested in links after load has completed
		linksChanged = pendingLayoutChange_;
		inFrame = inFrame_;
		pendingLayoutChange_ = false;
	}
	else
	{
		linksChanged = false;
		inFrame = false;
	}
}

void TdTox::setInFrame(bool inFrame)
{
	std::lock_guard<std::mutex> guard(mutex_);
	inFrame_ = inFrame;
}


void TdTox::linkLayoutDidChange(TELinkEvent event, const char* identifier)
{
	std::lock_guard<std::mutex> guard(mutex_);
	pendingLayoutChange_ = true;

}

void TdTox::applyLayoutChange()
{
	//renderer_->beginImageLayout();

	//renderer_->clearInputImages();
	//renderer_->clearOutputImages();
	outputLinkTextureMap_.clear();

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

									/*if (texFromTE_ && texToTE_.get() == nullptr)
									{
										VkExtent2D extent = texFromTE_->extent();
										VkFormat format = texFromTE_->format();
										std::cout << "Texture extent: " << extent.width << " x " << extent.height
											<< " format: " << string_VkFormat(format) << std::endl;

										texToTE_ = std::make_unique<Texture>(
											physicalDevice_,
											device_,
											extent,
											format
										);
									}*/
								}
								else
								{
									//renderer_->addOutputImage();
									//outputLinkTextureMap_[info->identifier] = renderer_->getRightSideImageCount() - 1;
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

bool TdTox::applyOutputTextureChange()
{
	// Only hold the lock briefly
	std::vector<std::string> changes;
	{
		std::lock_guard<std::mutex> guard(mutex_);
		std::swap(pendingOutputTextures_, changes);
	}

	for (const auto& identifier : changes)
	{
		size_t imageIndex = outputLinkTextureMap_[identifier];

		if (identifier == "op/topOut1")
		{
			TouchObject <TETexture> teTex;
			TEResult result = TEInstanceLinkGetTextureValue(
				instance_, identifier.c_str(), TELinkValueCurrent, teTex.take());

			if (result == TEResultSuccess && TEInstanceHasTextureTransfer(instance_, teTex))
			{
				if (texFromTE_.get() == nullptr)
				{
					texFromTE_ = std::make_unique<Texture>(
						renderer_->vContext().physicalDevice,
						renderer_->vContext().device,
						instance_,
						static_cast<TEVulkanTexture*>(teTex.get())
					);
					return true; // we need to wait for the texture to be ready
				}

				if (texToTE_.get() == nullptr && texFromTE_)
				{
					texToTE_ = std::make_unique<Texture>(
						physicalDevice_,
						device_,
						texFromTE_->extent(),
						texFromTE_->format()
					);
				}


				TouchObject<TESemaphore> teSemaphore;
				teSemaphore.set(texFromTE_->teVkSemaphore());

				uint64_t waitValue = 0;
				result = TEInstanceGetTextureTransfer(
					instance_, 
					teTex, 
					teSemaphore.take(),
					&waitValue);

				//std::cout << "TESemaphore: " << teSemaphore.get() << " waitValue: " << waitValue << std::endl;

				if (result == TEResultSuccess)
				{
					//std::cout << "Texture transfer: " << identifier << " : " << waitValue << std::endl;
					if (TESemaphoreGetType(teSemaphore) == TESemaphoreTypeVulkan)
					{
						//texFromTE_->copyImageToCudaMem(waitValue, cudaStream_);

						//texToTE_->copyCudaMemToImage(
						//	texFromTE_->cudaMemory(), 
						//	texFromTE_->cudaExtSemaphore(),
						//	waitValue,
						//	cudaStream_);
						//
						//// sleep for a while
						//std::this_thread::sleep_for(std::chrono::milliseconds(10));


						// wait for semaphore
						VkSemaphoreWaitInfoKHR waitInfo = {};
						waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO_KHR;
						waitInfo.pNext = nullptr;
						waitInfo.flags = 0;
						waitInfo.semaphoreCount = 1;
						VkSemaphore importSemaphore = texFromTE_->semaphore();
						waitInfo.pSemaphores = &importSemaphore;
						waitInfo.pValues = &waitValue;

						VK_CHECK(vkWaitSemaphores(device_, &waitInfo, UINT64_MAX));
			
						// copy to texToTE_
						VkCommandBufferBeginInfo beginInfo = {};
						beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
						beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
						beginInfo.pInheritanceInfo = nullptr;

						VK_CHECK(vkBeginCommandBuffer(commandBuffer_, &beginInfo));

						if (texFromTE_ && !srcInitialized_)
						{
							// transition to transfer src optimal

							texFromTE_->cmdTransitionImageLayout(
								commandBuffer_,
								VK_IMAGE_LAYOUT_UNDEFINED,
								VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
							);

							srcInitialized_ = true;
							std::cout << "srcInitialized_" << std::endl;
						}


						if (texToTE_ && !dstInitialized_)
						{
							// transition to transfer dst optimal
							texToTE_->cmdTransitionImageLayout(
								commandBuffer_,
								VK_IMAGE_LAYOUT_UNDEFINED,
								VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
							);


							dstInitialized_ = true;
							std::cout << "dstInitialized_" << std::endl;
						}

						VkImageSubresourceLayers subresourceLayers = {};
						subresourceLayers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
						subresourceLayers.mipLevel = 0;
						subresourceLayers.baseArrayLayer = 0;
						subresourceLayers.layerCount = 1;

						VkImageCopy imageCopy = {};
						imageCopy.srcSubresource = subresourceLayers;
						imageCopy.dstSubresource = subresourceLayers;
						imageCopy.extent = { texToTE_->extent().width, texToTE_->extent().height, 1 };


						vkCmdCopyImage(
							commandBuffer_,
							texFromTE_->image(),
							VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
							texToTE_->image(),
							VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
							1,
							&imageCopy
						);

						VK_CHECK(vkEndCommandBuffer(commandBuffer_));

						uint64_t signalValue = waitValue + 1;

						VkTimelineSemaphoreSubmitInfo timelineSemaphoreSubmitInfo = {};
						timelineSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
						timelineSemaphoreSubmitInfo.pNext = nullptr;
						timelineSemaphoreSubmitInfo.waitSemaphoreValueCount = 1; 
						timelineSemaphoreSubmitInfo.pWaitSemaphoreValues = &waitValue; 
						timelineSemaphoreSubmitInfo.signalSemaphoreValueCount = 1; 
						timelineSemaphoreSubmitInfo.pSignalSemaphoreValues = &signalValue; 

						VkSubmitInfo submitInfo = {};
						submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
						submitInfo.pNext = &timelineSemaphoreSubmitInfo; 
						submitInfo.commandBufferCount = 1;
						submitInfo.pCommandBuffers = &commandBuffer_;

						submitInfo.waitSemaphoreCount = 1;
						submitInfo.pWaitSemaphores = &importSemaphore;
						VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_TRANSFER_BIT }; 
						submitInfo.pWaitDstStageMask = waitStages;

						//	
						submitInfo.signalSemaphoreCount = 1;
						VkSemaphore signalSemaphores[] = { texToTE_.get()->semaphore() };
						submitInfo.pSignalSemaphores = signalSemaphores;

						VK_CHECK(vkQueueSubmit(queue_, 1, &submitInfo, submitFence_));
						VK_CHECK(vkWaitForFences(device_, 1, &submitFence_, VK_TRUE, UINT64_MAX));
						VK_CHECK(vkResetFences(device_, 1, &submitFence_));

					}
				}
			}
		}
	}

	return !changes.empty();
}

void TdTox::update()
{
	TEInstanceStartFrameAtTime(instance_, TETimeInternal, 0, 0);

	bool configured, loaded, linksChanged, inFrame;
	getState(configured, loaded, linksChanged, inFrame);

	if (configured)
	{
		std::string message;
		if (TEResultGetSeverity(configureResult_) == TESeverityError)
		{
			const char* description = TEResultGetDescription(configureResult_);

			message = "There was an error configuring TouchEngine: ";
			if (description)
			{
				message += description;
			}
			else
			{
				message += std::to_string(configureResult_);
			}
			configureError_ = true;
		}
		else
		{
			configureError_ = !renderer_->configureTEInstance(instance_, message);
		}
		if (configureError_)
		{
			std::cout << message << std::endl;
		}
	}

	if (configureError_)
	{
		return;
	}

	bool changed = linksChanged;

	// Make any pending renderer state updates
	if (linksChanged)
	{
		applyLayoutChange();
	}

	if (loaded && !inFrame)
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
								if (texToTE_)
								{
									//std::cout << "TELinkTypeTexture: " << info->identifier << std::endl;

									TouchObject<TETexture> texture;
									texture.set(texToTE_->teVkTexture());

									result = TEInstanceLinkSetTextureValue(
										instance_, info->identifier, texture, renderer_->teContext());

									//std::cout << "TEInstanceLinkSetTextureValue: " << TEResultGetDescription(result) << std::endl;

									if (result == TEResultSuccess)
									{
										result = TEInstanceAddTextureTransfer(
											instance_, texture, texFromTE_->teVkSemaphore(), texToTE_->waitValue());
									}

								//}
								//textureCount++;
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

		//setInFrame(true);

		//int64_t time = getRenderTime();
		//myLastResult = TEInstanceStartFrameAtTime(instance_, time, TimeRate, false);
		//if (myLastResult == TEResultSuccess)
		//{
		//	myLastFloatValue += 1.0 / (60.0 * 8.0);
		//}
		//else
		//{
		//	setInFrame(false);
		//}
	}
	if (changed)
	{
		
		render(loaded);
	}

}



void TdTox::render(bool loaded)
{
	if (loaded)
	{
		renderer_->renderFrame();
	}
}


void TdTox::linkValueChange(const char* identifier)
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

			//if (strcmp(link->identifier, "op/topOut1") == 0)
			//{
			//	TouchObject <TETexture> tex;
			//	TEResult result = TEInstanceLinkGetTextureValue(
			//		instance_, identifier, TELinkValueCurrent, tex.take());

			//	if (result == TEResultSuccess)
			//	{
			//		TEVulkanTexture* vkTex = static_cast<TEVulkanTexture*>(tex.get());

			//		if (texFromTE_.get() == nullptr)
			//		{
			//			texFromTE_ = std::make_unique<Texture>(
			//				renderer_->vContext().device,
			//				vkTex
			//			);
			//		}

			//		VkExtent2D extent = {
			//			static_cast<uint32_t> (TEVulkanTextureGetWidth(vkTex)),
			//			static_cast<uint32_t> (TEVulkanTextureGetHeight(vkTex))
			//		};

			//		VkFormat format = TEVulkanTextureGetFormat(vkTex);

			//		std::cout << "Texture extent: " << extent.width << " x " << extent.height 
			//				<< " format: " << string_VkFormat(format) << std::endl;


			//		if (texToTE_.get() == nullptr)
			//		{
			//			texToTE_ = std::make_unique<Texture>(
			//				renderer_->vContext().physicalDevice,
			//				renderer_->vContext().device,
			//				renderer_->vContext().queueFamilyIndices.usedFamilyIndices(),
			//				extent,
			//				format
			//			);
			//		}
			//	}
			//	else
			//	{
			//		std::cout << TEResultGetDescription(result) << std::endl;
			//	}
			//}

			break;
		}
		case TELinkTypeFloatBuffer:
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

					static int printCount = 0;

					if (printCount++ < 5)
					{
						std::cout << "Float buffer values: ";

						for (int channel = 0; channel < channelCount; channel++)
						{
							// Here we just grab the first sample in the channel
							float value = data[channel][0];
							std::cout << value << ", ";
						}

						std::cout << std::endl;
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


