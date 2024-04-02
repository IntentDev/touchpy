#include "toplink.h"
#include <TouchEngine/TEVulkan.h>


TopLink::TopLink(
	TouchObject<TEInstance> instance,
	TouchObject<TEGraphicsContext> context,
	VkPhysicalDevice physicalDevice,
	VkDevice device,
	TouchObject<TELinkInfo> linkInfo,
	cudaStream_t cudaStream)
	:	Link<TopLink>(instance, linkInfo),
		context_(context),
		physicalDevice_(physicalDevice),
		device_(device),
		cudaStream_(cudaStream){ }

TopLink::~TopLink() { }

std::array<size_t, 3> TopLink::shape()
{
	std::array<size_t, 3> extent { 0, 0, 4u };
	auto currentTex = currentTexture();
	extent[0] = static_cast<size_t>(currentTex->width());
	extent[1] = static_cast<size_t>(currentTex->height());

	return extent;
}

void 
OutTopLink::addOutputTexture(TouchObject<TEInstance> teInstance, TEVulkanTexture* teTexture)
{
	if (scope_ != Link::Scope::Output)
		return;

	textures_.push_back(std::make_unique<Texture>(physicalDevice_, device_, teInstance, teTexture, cudaFlags_, requiresCudaMemLock_));
	auto& texture = textures_.back();
	handleMap_[texture->textureHandle()] = texture.get();
}

void 
OutTopLink::onOutputTextureChange()
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
				texture->copyImageToCudaMem(waitValue, cudaStream_, true);

		currentTextureHandle_ = handle;
	}
	else
		std::cout << "onOutputTextureChange: " << identifier_ << ", " << TEResultGetDescription(result) << std::endl;

}

void 
OutTopLink::setRequiresCudaMemLock(bool requiresCudaMemLock) 
{ 
	requiresCudaMemLock_ = requiresCudaMemLock; 

	for (auto& tex : textures_)
		tex->setRequiresCudaMemLock(requiresCudaMemLock_);
}

void OutTopLink::setCudaFlags(CudaFlags flags)
{
	cudaFlags_ = flags;
	for (auto& tex : textures_)
		tex->configureCudaMemory(cudaFlags_);
}

void 
InTopLink::setInputTexture(VkExtent2D extent, VkFormat format, CUDAMemoryDesc cudaMemDesc)
{
	textures_.resize(1);
	textures_[0] = std::make_unique<Texture>(physicalDevice_, device_, extent, format, cudaMemDesc);
	handleMap_[textures_[0]->textureHandle()] = textures_[0].get();
}

void
InTopLink::transferTextureToInputLink()
{
	if (textures_.size() == 0) return;

	TouchObject<TETexture> teTexture;
	teTexture.set(textures_[0]->teVkTexture());
	TEResult result = TEInstanceLinkSetTextureValue(instance_, identifier_.c_str(), teTexture, context_);
	if (result == TEResultSuccess)
		result = TEInstanceAddTextureTransfer(instance_, teTexture, textures_[0]->teVkSemaphore(), textures_[0]->signalValue());

	if (result != TEResultSuccess)
		std::cout << "transferToInputLink: " << identifier_ << ", " << TEResultGetDescription(result) << std::endl;
}

void
InTopLink::copyCudaMemory(const CUDAMemory& cudaMem, cudaStream_t stream)
{
	if (textures_.size() == 0)
	{
		VkExtent2D extent;
		if (cudaMem.desc.flags & CudaFlagBits::HWC) extent = { cudaMem.desc.shape[1], cudaMem.desc.shape[0] };
		else extent = { cudaMem.desc.shape[2], cudaMem.desc.shape[1] };

		VkFormat format = vkFormatFromCUDAMemoryDesc(cudaMem.desc);
		setInputTexture(extent, format, cudaMem.desc);
	}

	else if (cudaMem.desc != textures_[0]->cudaMemory().desc)
	{
		textures_[0].reset();
		VkExtent2D extent;
		if (cudaMem.desc.flags & CudaFlagBits::HWC) extent = { cudaMem.desc.shape[1], cudaMem.desc.shape[0] };
		else extent = { cudaMem.desc.shape[2], cudaMem.desc.shape[1] };
		VkFormat format = vkFormatFromCUDAMemoryDesc(cudaMem.desc);
		setInputTexture(extent, format, cudaMem.desc);
	}

	uint64_t signalValue;
	VK_CHECK(vkGetSemaphoreCounterValue(device_, textures_[0]->semaphore(), &signalValue));
	textures_[0]->setSignalValue(++signalValue);

	auto copied = textures_[0]->copyCudaMemToImage(cudaMem.ptr, nullptr, textures_[0]->cudaExtSemaphore(), 0, signalValue, stream);

	if (copied) transferTextureToInputLink();
	else std::cout << "copyCudaMemory: " << name_ << ", failed to copy memory" << std::endl;
}

//void
//InTopLink::copyExternalCudaMemory(
//	void* memory,
//	VkFormat format,
//	VkExtent2D extent,
//	cudaExternalSemaphore_t waitSemaphore, 
//	uint64_t waitValue, 
//	cudaStream_t stream, 
//	CudaFlags flags)
//{
//	if (scope_ != Link::Scope::Input)
//		return;
//
//	if (textures_.size() == 0) setInputTexture(extent, format);
//
//	else if (textures_[0]->format() != format || textures_[0]->width() != extent.width || textures_[0]->height() != extent.height)
//	{
//		textures_[0].reset();
//		setInputTexture(extent, format);
//	}
//
//	uint64_t signalValue;
//	VK_CHECK(vkGetSemaphoreCounterValue(device_, textures_[0]->semaphore(), &signalValue));
//	textures_[0]->setSignalValue(++signalValue);
//
//	textures_[0]->copyCudaMemToImage(
//		memory,
//		waitSemaphore,
//		textures_[0]->cudaExtSemaphore(),
//		waitValue,
//		signalValue,
//		stream);
//}
