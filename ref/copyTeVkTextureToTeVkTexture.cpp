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

					// wait for semaphore
					VkSemaphoreWaitInfoKHR waitInfo = {};
					waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO_KHR;
					waitInfo.pNext = nullptr;
					waitInfo.flags = 0;
					waitInfo.semaphoreCount = 1;
					//VkSemaphore importSemaphore = texFromTE_->semaphore();
					VkSemaphore importSemaphore = texExternal->semaphore();
					waitInfo.pSemaphores = &importSemaphore;
					waitInfo.pValues = &waitValue;

					VK_CHECK(vkWaitSemaphores(device_, &waitInfo, UINT64_MAX));

					// reset command buffer
					VK_CHECK(vkResetCommandBuffer(commandBuffer_, 0));
			
					// copy to texToTE_
					VkCommandBufferBeginInfo beginInfo = {};
					beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
					beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
					beginInfo.pInheritanceInfo = nullptr;


					VK_CHECK(vkBeginCommandBuffer(commandBuffer_, &beginInfo));

					//if (texFromTE_ && !srcInitialized_)
					if (texExternal && texExternal->imageLayout() != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
					{
						// transition to transfer src optimal

						//texFromTE_->cmdTransitionImageLayout(
						texExternal->cmdTransitionImageLayout(
							commandBuffer_,
							VK_IMAGE_LAYOUT_UNDEFINED,
							VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
						);

						//srcInitialized_ = true;
						//std::cout << "srcInitialized_" << std::endl;
					}

					if (texToTE_->imageLayout() != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
					//if (texInternal && texInternal->imageLayout() != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
					{
						// transition to transfer dst optimal
						texToTE_->cmdTransitionImageLayout(
							commandBuffer_,
							VK_IMAGE_LAYOUT_UNDEFINED,
							VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
						);
						//dstInitialized_ = true;
						//std::cout << "dstInitialized_" << std::endl;
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
						//texFromTE_->image(),
						texExternal->image(),
						VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						texToTE_->image(),
						//texInternal->image(),
						VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						1,
						&imageCopy
					);

					VK_CHECK(vkEndCommandBuffer(commandBuffer_));

					uint64_t signalValue;
					VK_CHECK(vkGetSemaphoreCounterValue(device_, texToTE_->semaphore(), &signalValue));
					texToTE_->setSignalValue(++signalValue);
					//texInternal->setSignalValue(++signalValue);


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
					VkSemaphore signalSemaphores[] = { texToTE_->semaphore() };
					//VkSemaphore signalSemaphores[] = { texInternal->semaphore() };
					submitInfo.pSignalSemaphores = signalSemaphores;

					VK_CHECK(vkQueueSubmit(queue_, 1, &submitInfo, submitFence_));
					VK_CHECK(vkWaitForFences(device_, 1, &submitFence_, VK_TRUE, UINT64_MAX));
					VK_CHECK(vkResetFences(device_, 1, &submitFence_));
					
					std::cout << " copied texture ";

				}
			}
		}
	}
}