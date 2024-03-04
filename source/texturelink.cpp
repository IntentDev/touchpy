#include "texturelink.h"
#include <TouchEngine/TEVulkan.h>




TextureLink::TextureLink(
	TouchObject<TEInstance> instance,
	TouchObject<TEGraphicsContext> context,
	VkPhysicalDevice physicalDevice,
	VkDevice device,
	TouchObject<TELinkInfo> linkInfo)
	:	Link<TextureLink>(instance, linkInfo),
		context_(context),
		physicalDevice_(physicalDevice),
		device_(device) { }

TextureLink::~TextureLink() { }

void TextureLink::addOutputTexture(TouchObject<TEInstance> teInstance, TEVulkanTexture* teTexture)
{
	if (scope_ != Link::Scope::Output)
		return;

	textures_.push_back(std::make_unique<Texture>(physicalDevice_, device_, teInstance, teTexture));
	handleMap_[textures_.back()->textureHandle()] = textures_.back().get();
}

void TextureLink::setInputTexture(VkExtent2D extent, VkFormat format)
{
	if (scope_ != Link::Scope::Input)
		return;

	// using just the first texture in the vector for now
	textures_.resize(1);
	textures_[0] = std::make_unique<Texture>(physicalDevice_, device_, extent, format);
	handleMap_[textures_[0]->textureHandle()] = textures_[0].get();
}


void
TextureLink::transferTextureToInputLink(TouchObject<TEGraphicsContext> context)
{
	if (textures_.size() == 0 || scope_ != Link::Scope::Input)
		return;

	TouchObject<TETexture> teTexture;
	teTexture.set(textures_[0]->teVkTexture());
	TEResult result = TEInstanceLinkSetTextureValue(instance_, identifier_.c_str(), teTexture, context);
	if (result == TEResultSuccess)
		result = TEInstanceAddTextureTransfer(instance_, teTexture, textures_[0]->teVkSemaphore(), textures_[0]->signalValue());

	if (result != TEResultSuccess)
		std::cout << "transferToInputLink: " << identifier_ << ", " << TEResultGetDescription(result) << std::endl;
}

void
TextureLink::copyCudaMemoryToInputTexture(uint8_t* memory, cudaExternalSemaphore_t waitSemaphore, uint64_t waitValue, cudaStream_t stream)
{
	if (textures_.size() == 0 || scope_ != Link::Scope::Input)
		return;

	uint64_t signalValue;
	VK_CHECK(vkGetSemaphoreCounterValue(device_, textures_[0]->semaphore(), &signalValue));
	textures_[0]->setSignalValue(++signalValue);

	textures_[0]->copyCudaMemToImage(
		memory,
		waitSemaphore,
		textures_[0]->cudaExtSemaphore(),
		waitValue,
		signalValue,
		stream);
}


void TextureLink::onOutputTextureChange(cudaStream_t cudaStream_)
{
	if (scope_ != Link::Scope::Output)
		return;

	TouchObject <TETexture> teTex;
	TEResult result = TEInstanceLinkGetTextureValue(instance_, identifier_.c_str(), TELinkValueCurrent, teTex.take());

	if (result == TEResultSuccess && TEInstanceHasTextureTransfer(instance_, teTex))
	{
		HANDLE handle = TEVulkanTextureGetHandle(static_cast<TEVulkanTexture*>(teTex.get()));

		auto it = handleMap_.find(handle);
		if (it == handleMap_.end())
			addOutputTexture(instance_, static_cast<TEVulkanTexture*>(teTex.get()));

		auto texture = handleMap_[handle];
		TouchObject<TESemaphore> teSemaphore;
		teSemaphore.set(texture->teVkSemaphore());

		uint64_t waitValue = 0;
		result = TEInstanceGetTextureTransfer(instance_, teTex, teSemaphore.take(), &waitValue);

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

		if (result == TEResultSuccess)
			if (TESemaphoreGetType(teSemaphore) == TESemaphoreTypeVulkan)
				texture->copyImageToCudaMem(waitValue, cudaStream_);
	}
	else
		std::cout << "onOutputTextureChange: " << identifier_ << ", " << TEResultGetDescription(result) << std::endl;
	
}


