
#include "texture.h"
#include <TouchEngine/TEVulkan.h>
#include <Windows.h>
#include <algorithm>
#include <vulkan/vk_enum_string_helper.h>

#include "cudamemory.h"

#include "logging.h"

Texture::Texture(
	VkPhysicalDevice physicalDevice_, 
	VkDevice device, 
	TEInstance* teInstance, 
	TEVulkanTexture* texture,
	cudaStream_t* streamPtr,
	CudaFlags cudaFlags,
	bool requiresCudaMemLock
)
	:	physicalDevice_(physicalDevice_),
		device_(device),
		flipped_(TETextureGetOrigin(texture) == TETextureOriginBottomLeft),
		requiresCudaMemLock_(requiresCudaMemLock),
		cudaStreamPtr_(streamPtr)

{
	format_ = TEVulkanTextureGetFormat(texture);
	numComponents_ = numCompsFromVkFormat(format_);
	componentSize_ = componentSizeFromVkFormat(format_);
	extent_ = {
		static_cast<uint32_t> (TEVulkanTextureGetWidth(texture)),
		static_cast<uint32_t> (TEVulkanTextureGetHeight(texture))
	};

	imagePitch_ = extent_.width * componentSize_ * numComponents_;
	textureHandle_ = TEVulkanTextureGetHandle(texture);
	VkExternalMemoryHandleTypeFlagBits handleType = TEVulkanTextureGetHandleType(texture);

	VkExternalMemoryImageCreateInfo externalMemoryImageCreateInfo = {};
	externalMemoryImageCreateInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
	externalMemoryImageCreateInfo.pNext = nullptr;
	externalMemoryImageCreateInfo.handleTypes = handleType;

	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = &externalMemoryImageCreateInfo;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width = extent_.width;
	imageCreateInfo.extent.height = extent_.height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.format = format_;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK(vkCreateImage(device_, &imageCreateInfo, nullptr, &image_));


	VkImportMemoryWin32HandleInfoKHR importMemoryInfo = {};
	importMemoryInfo.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR;
	importMemoryInfo.handleType = handleType;
	importMemoryInfo.handle = textureHandle_; // Handle to the external memory

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image_, &memRequirements);

	uint32_t memoryTypeIndex = vri::findMemoryType(
		physicalDevice_,
		memRequirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	imageSize_ = static_cast<size_t>(memRequirements.size);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
	memoryAllocateInfo.pNext = &importMemoryInfo;

	// Import the external memory into Vulkan
	// causing validation error in Vulkan 1.3.275.0, need to figure this out but an ImageView is not needed
	// for the texture to be used in CUDA
	// 
	// Validation Error: [ VUID-VkMemoryAllocateInfo-memoryTypeIndex-00645 ] | MessageID = 0xb4ec2301 | vkAllocateMemory(): 
	// pAllocateInfo->memoryTypeIndex is 1 but VkMemoryWin32HandlePropertiesKHR::memoryTypeBits is 0x0. 
	// The Vulkan spec states: If the parameters define an import operation and the external handle is an NT handle or a 
	// global share handle created outside of the Vulkan API, the value of memoryTypeIndex must be one of those returned 
	// by vkGetMemoryWin32HandlePropertiesKHR 
	// (https://vulkan.lunarg.com/doc/view/1.3.275.0/windows/1.3-extensions/vkspec.html#VUID-VkMemoryAllocateInfo-memoryTypeIndex-00645)
	// 
	// This is a bug in Vulkan 1.3.275.0, the memoryTypeIndex is correct!!! Need to update to the next release of Vulkan
	// when it becomes available then uncomment the code below so the texture can be used in Vulkan as well as CUDA
	// 
	VkDeviceMemory externalMemory;
	VK_CHECK(vkAllocateMemory(device_, &memoryAllocateInfo, nullptr, &externalMemory));
	VK_CHECK(vkBindImageMemory(device_, image_, externalMemory, 0));

	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image_;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format_;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VK_CHECK(vkCreateImageView(device_, &viewInfo, nullptr, &imageView_));

	SPDLOG_DEBUG("Texture Created, width: {}, height: {}, format: {}, Size: {}, Memory Type Index: {}", 
		extent_.width, extent_.height, string_VkFormat(format_), memRequirements.size, memoryTypeIndex);
	SPDLOG_FLUSH_DEBUG

	importSemaphore(teInstance, texture);

	setupCudaResources(textureHandle_, semaphoreHandle_, true, cudaFlags);

}

Texture::Texture(
	VkPhysicalDevice physicalDevice,
	VkDevice device,
	VkExtent2D extent,
	VkFormat format, 
	CUDAMemoryDesc cudaMemDesc,
	cudaStream_t* streamPtr
)
	:	physicalDevice_(physicalDevice),
		device_(device),
		extent_(extent),
		format_(format),
		cudaStreamPtr_(streamPtr)


{
	numComponents_ = numCompsFromVkFormat(format_);
	componentSize_ = componentSizeFromVkFormat(format_);
	imagePitch_ = extent_.width * componentSize_ * numComponents_;

	VkExternalMemoryImageCreateInfo externalMemoryImageCreateInfo = {};
	externalMemoryImageCreateInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
	externalMemoryImageCreateInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR;

	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = &externalMemoryImageCreateInfo;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width = extent.width;
	imageCreateInfo.extent.height = extent.height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.format = format;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK(vkCreateImage(device_, &imageCreateInfo, nullptr, &image_));

	VkExportMemoryAllocateInfo exportMemoryInfo = {};
	exportMemoryInfo.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO;
	exportMemoryInfo.pNext = nullptr;
	exportMemoryInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR;

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image_, &memRequirements);

	uint32_t memoryTypeIndex = vri::findMemoryType(
		physicalDevice_,
		memRequirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	imageSize_ = static_cast<size_t>(memRequirements.size);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = &exportMemoryInfo;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = memoryTypeIndex;

	VK_CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &memory_));
	VK_CHECK(vkBindImageMemory(device, image_, memory_, 0));

	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image_;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format_;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VK_CHECK(vkCreateImageView(device_, &viewInfo, nullptr, &imageView_));

	textureHandle_ = getVkMemoryHandle(VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR, memory_);

	teVkTexture_.take(TEVulkanTextureCreate(
						textureHandle_,
						VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR, 
						format_, 
						extent_.width, 
						extent_.height,
						TETextureOriginBottomLeft,
						kTEVkComponentMappingIdentity,
						VulkanTextureCallback,
						this));		

	VkSemaphoreTypeCreateInfoKHR semaphoreTypeCreateInfo{};
	semaphoreTypeCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO_KHR;
	semaphoreTypeCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
	semaphoreTypeCreateInfo.initialValue = 0;

	VkExportSemaphoreCreateInfo exportSemaphoreCreateInfo{};
	exportSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO;
	exportSemaphoreCreateInfo.pNext = &semaphoreTypeCreateInfo;
	exportSemaphoreCreateInfo.handleTypes = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;

	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = &exportSemaphoreCreateInfo;

	VK_CHECK(vkCreateSemaphore(device_, &semaphoreCreateInfo, nullptr, &semaphore_));

	semaphoreHandle_ = getVkSemaphoreHandle(VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT, semaphore_);

	teVkSemaphore_.set(TEVulkanSemaphoreCreate(
		VK_SEMAPHORE_TYPE_TIMELINE,
		semaphoreHandle_,
		VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT,
		VulkanSemaphoreCallback,
		this));

	ownsSemaphore_ = true;
	ownsImage_ = true;

	cudaMemory_.desc = cudaMemDesc;
	setupCudaResources(textureHandle_, semaphoreHandle_, false);
	setCudaCopyToSurfaceFunc();

	SPDLOG_DEBUG("Texture Created, width: {}, height: {}, format: {}, Size: {}, Memory Type Index: {}", 
		extent_.width, extent_.height, string_VkFormat(format_), memRequirements.size, memoryTypeIndex);
	SPDLOG_FLUSH_DEBUG


}

Texture::~Texture()
{
	cudaStreamSynchronize(*cudaStreamPtr_);

	if (cudaBuffer_)
		CUDA_CHECK(cudaFree(cudaBuffer_));
	if (cudaSurface_ != 0)
		CUDA_CHECK(cudaDestroySurfaceObject(cudaSurface_));
	if (cudaArray_)
		CUDA_CHECK(cudaFreeArray(cudaArray_));
	if (cudaMipmappedArray_)
		CUDA_CHECK(cudaFreeMipmappedArray(cudaMipmappedArray_));
	if (cudaExtImageMemory_)
		CUDA_CHECK(cudaDestroyExternalMemory(cudaExtImageMemory_));
	if (cudaExtSemaphore_)
		CUDA_CHECK(cudaDestroyExternalSemaphore(cudaExtSemaphore_));
	
	if (ownsImage_)
		CloseHandle(textureHandle_);

	if (ownsSemaphore_)
		CloseHandle(semaphoreHandle_);

	vkDeviceWaitIdle(device_);

	if (imageView_ != VK_NULL_HANDLE)
		vkDestroyImageView(device_, imageView_, nullptr);

	if (memory_ != VK_NULL_HANDLE)
		vkFreeMemory(device_, memory_, nullptr);

	if (semaphore_ != VK_NULL_HANDLE)
		vkDestroySemaphore(device_, semaphore_, nullptr);

	if (image_ != VK_NULL_HANDLE)
		vkDestroyImage(device_, image_, nullptr);
}

void Texture::importSemaphore(TEInstance* teInstance, TETexture* teTexture)
{
	TouchObject<TESemaphore> teSemaphore;
	uint64_t waitValue = 0;
	TEResult result = TEInstanceGetTextureTransfer(teInstance, teTexture, teSemaphore.take(), &waitValue);

	//SPDLOG_DEBUG("Texture transfer: {}, waitValue: {}", result, waitValue);

	if (result == TEResultSuccess)
	{
		if (TESemaphoreGetType(teSemaphore) == TESemaphoreTypeVulkan)
		{
			TEVulkanSemaphore* teVulkanSemaphore = static_cast<TEVulkanSemaphore*>(teSemaphore.get());

			semaphoreHandle_ = TEVulkanSemaphoreGetHandle(teVulkanSemaphore);
			semaphoreType_ = TEVulkanSemaphoreGetType(teVulkanSemaphore);
			semaphoreHandleType_ = TEVulkanSemaphoreGetHandleType(teVulkanSemaphore);
			
			// need to cast for fmt... 
			//SPDLOG_DEBUG("Semaphore handle: {}, type: {}, handleType: {}", semaphoreHandle_, semaphoreType_, semaphoreHandleType_);

			VkSemaphoreTypeCreateInfoKHR semaphoreTypeCreateInfo {};
			semaphoreTypeCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO_KHR;
			semaphoreTypeCreateInfo.pNext = nullptr;
			semaphoreTypeCreateInfo.semaphoreType = semaphoreType_;
			semaphoreTypeCreateInfo.initialValue = waitValue;

			VkSemaphoreCreateInfo semaphoreCreateInfo = {
				VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, &semaphoreTypeCreateInfo, 0 };

			VK_CHECK(vkCreateSemaphore(
				device_,
				&semaphoreCreateInfo,
				nullptr,
				&semaphore_
			));

			//SPDLOG_DEBUG("Semaphore Created: {}", semaphore_);

			// import semaphore
			VkImportSemaphoreWin32HandleInfoKHR importSemaphoreInfo {};
			importSemaphoreInfo.sType = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR;
			importSemaphoreInfo.pNext = nullptr;
			importSemaphoreInfo.semaphore = semaphore_;
			importSemaphoreInfo.flags = 0;// VK_SEMAPHORE_IMPORT_TEMPORARY_BIT_KHR;
			importSemaphoreInfo.handleType = semaphoreHandleType_;
			importSemaphoreInfo.handle = semaphoreHandle_;
			importSemaphoreInfo.name = nullptr;

			auto vkImportSemaphoreWin32HandleKHR = PFN_vkImportSemaphoreWin32HandleKHR(
				vkGetDeviceProcAddr(device_, "vkImportSemaphoreWin32HandleKHR"));

			VK_CHECK(vkImportSemaphoreWin32HandleKHR(device_, &importSemaphoreInfo));

			//SPDLOG_DEBUG("Semaphore Imported: {}", semaphore_);
		}
	}
	//SPDLOG_FLUSH_DEBUG
}

void Texture::cmdTransitionImageLayout(VkCommandBuffer cmdBuffer, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkImageMemoryBarrier imageMemBarrier = {};
	imageMemBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemBarrier.oldLayout = oldLayout;
	imageMemBarrier.newLayout = newLayout;
	imageMemBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemBarrier.image = image_;
	imageMemBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemBarrier.subresourceRange.baseMipLevel = 0;
	imageMemBarrier.subresourceRange.levelCount = 1;
	imageMemBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemBarrier.subresourceRange.layerCount = 1;


	VkPipelineStageFlags srcStage;
	VkPipelineStageFlags dstStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		imageMemBarrier.srcAccessMask = 0;
		imageMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		imageMemBarrier.srcAccessMask = 0;
		imageMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		imageMemBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		imageMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		imageMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		imageMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageMemBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		imageMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}

	else
		throw std::invalid_argument("Unsupported layout transition!");

	vkCmdPipelineBarrier(
		cmdBuffer,
		srcStage,
		dstStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &imageMemBarrier
	);

	// should be set after it's been transitioned, look into callback for this
	imageLayout_ = newLayout;
}

void Texture::VulkanSemaphoreCallback(HANDLE semaphore, TEObjectEvent event, void* info)
{
	switch (event) 
	{
	case TEObjectEventBeginUse:
		// Handle begin use event  
		break;
	case TEObjectEventEndUse:
		// Handle semaphore end use event
		break;
	case TEObjectEventRelease:
		// Handle semaphore release event
		break;

	}
}

void Texture::VulkanTextureCallback(HANDLE texture, TEObjectEvent event, void* info)
{
	switch (event) 
	{
	case TEObjectEventBeginUse:
		// Handle begin use event  
		break;
	case TEObjectEventEndUse:
		// Handle texture end use event
		break;
	case TEObjectEventRelease:
		// Handle texture release event
		break;

	}
}

HANDLE Texture::getVkSemaphoreHandle(
	VkExternalSemaphoreHandleTypeFlagBitsKHR externalSemaphoreHandleType,
	VkSemaphore& semaphore
)
{
	HANDLE handle;

	VkSemaphoreGetWin32HandleInfoKHR vulkanSemaphoreGetWin32HandleInfoKHR = {};
	vulkanSemaphoreGetWin32HandleInfoKHR.sType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR;
	vulkanSemaphoreGetWin32HandleInfoKHR.pNext = NULL;
	vulkanSemaphoreGetWin32HandleInfoKHR.semaphore = semaphore;
	vulkanSemaphoreGetWin32HandleInfoKHR.handleType = externalSemaphoreHandleType;

	auto vkGetSemaphoreWin32HandleKHR = PFN_vkGetSemaphoreWin32HandleKHR(
		vkGetDeviceProcAddr(device_, "vkGetSemaphoreWin32HandleKHR"));

	vkGetSemaphoreWin32HandleKHR(device_, &vulkanSemaphoreGetWin32HandleInfoKHR,
		&handle);

	return handle;
}

HANDLE Texture::getVkMemoryHandle(VkExternalMemoryHandleTypeFlagBitsKHR externalMemoryHandleType, VkDeviceMemory& memory)
{
	HANDLE handle;
	VkMemoryGetWin32HandleInfoKHR memoryHandleInfo = {};
	memoryHandleInfo.sType = VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR;
	memoryHandleInfo.memory = memory;
	memoryHandleInfo.handleType = externalMemoryHandleType;

	auto vkGetMemoryWin32HandleKHR = PFN_vkGetMemoryWin32HandleKHR(
		vkGetDeviceProcAddr(device_, "vkGetMemoryWin32HandleKHR"));

	VK_CHECK(vkGetMemoryWin32HandleKHR(device_, &memoryHandleInfo, &handle));

	return handle;
}

void Texture::copyImageToCudaMem(uint64_t& waitValue, cudaStream_t stream, bool signal)
{
	if (copySurfaceFunc_)
	{ 
		// TODO: change the sequence of calls so the switch is before the wait
		cudaVkSemaphoreWait(cudaExtSemaphore_, waitValue, stream);
		CUDA_CHECK(copySurfaceFunc_(cudaBuffer_, extent_.width, extent_.height, cudaSurface_, stream));
		cudaVkSemaphoreSignal(cudaExtSemaphore_, ++waitValue, stream);
		waitValue_ = waitValue;
	}
}

bool Texture::copyCudaMemToImage(void* memory,
	cudaExternalSemaphore_t waitSemaphore,
	cudaExternalSemaphore_t signalSemaphore,
	uint64_t waitValue, 
	uint64_t signalValue,
	cudaStream_t stream)
{	
	if (copyToSurfaceFunc_)
	{
		if (waitSemaphore)
			cudaVkSemaphoreWait(waitSemaphore, waitValue, stream);

		CUDA_CHECK(copyToSurfaceFunc_(cudaSurface_, extent_.width, extent_.height, memory, stream));
		cudaVkSemaphoreSignal(signalSemaphore, signalValue, stream);
		return true;
	}
	return false;
}

void Texture::transferToInputLink(TouchObject<TEInstance> teInstance, TouchObject<TEGraphicsContext> context, const char* identifier)
{
	// need create TETexture pool to avoid creating a new TETexture for each transfer
	// 
	// If you are setting a shareable texture type on input links directly, TouchEngine will use the lifetime of the 
	// TETextures you create to manage the lifetime of internal resources. For this reason, performance is improved by 
	// recycling textures in a pool, and keeping the associated TETexture alive for the lifetime of the underlying 
	// resource. To know when a texture is in use by TouchEngine, use the TEObjectEvent parameter of the TETexture's 
	// callback and monitor TEObjectEventBeginUse and TEObjectEventEndUse. When TEObjectEventEndUse is received, the 
	// texture can be returned to your pool for reuse.

	TouchObject<TETexture> texture;
	texture.set(teVkTexture_);
	TEResult result = TEInstanceLinkSetTextureValue(teInstance, identifier, texture, context);
	if (result == TEResultSuccess)
		result = TEInstanceAddTextureTransfer(teInstance, texture, teVkSemaphore_, signalValue_);

	if (result != TEResultSuccess)
	{
		spdlog::error("transferToInputLink: {}, {}", identifier, TEResultGetDescription(result));
		SPDLOG_FLUSH
	}
}

const CUDAMemory& 
Texture::cudaMemory(bool syncCudaStream) const
{ 
	if (syncCudaStream) cudaStreamSynchronize(*cudaStreamPtr_);

	if (!requiresCudaMemLock_) return cudaMemory_;

	std::lock_guard<std::mutex> guard(mutex_);
	return cudaMemory_; 
}

void Texture::setupCudaResources(HANDLE imageHandle, HANDLE semaphoreHandle, bool allocateMemory, CudaFlags flags)
{
	cudaImportTimelineSemaphore(semaphoreHandle_);
	cudaImportImageMemory(textureHandle_);

	// we are only actually allocating memory for the cuda buffer if we are copying from the texture.
	// In the case of allocation, the size is the actual number of bytes in the image, not imageSize_
	// which is the size of the memory requirements for the VkImage
	if (allocateMemory)
	{
		//cudaAllocateMemory(numComponents_);
		configureCudaMemory(flags);
	}
}

void Texture::cudaImportTimelineSemaphore(HANDLE semaphoreHandle)
{
	cudaExternalSemaphoreHandleDesc cudaExtSemaphoreHandleDesc = {};
	std::memset(&cudaExtSemaphoreHandleDesc, 0, sizeof(cudaExtSemaphoreHandleDesc));
	cudaExtSemaphoreHandleDesc.type = cudaExternalSemaphoreHandleTypeTimelineSemaphoreWin32;
	cudaExtSemaphoreHandleDesc.handle.win32.handle = semaphoreHandle;
	cudaExtSemaphoreHandleDesc.flags = 0;

	cudaImportExternalSemaphore(&cudaExtSemaphore_, &cudaExtSemaphoreHandleDesc);
}



void Texture::cudaImportSemaphore(HANDLE semaphoreHandle)
{
	cudaExternalSemaphoreHandleDesc cudaExtSemaphoreHandleDesc = {};
	std::memset(&cudaExtSemaphoreHandleDesc, 0, sizeof(cudaExtSemaphoreHandleDesc));
	cudaExtSemaphoreHandleDesc.type = cudaExternalSemaphoreHandleTypeOpaqueWin32;
	cudaExtSemaphoreHandleDesc.handle.win32.handle = semaphoreHandle;
	cudaExtSemaphoreHandleDesc.flags = 0;

	cudaImportExternalSemaphore(&cudaExtSemaphore_, &cudaExtSemaphoreHandleDesc);
}

void Texture::cudaImportImageMemory(HANDLE imageHandle)
{
	cudaExternalMemoryHandleDesc cudaExtMemHandleDesc;
	std::memset(&cudaExtMemHandleDesc, 0, sizeof(cudaExtMemHandleDesc));
	cudaExtMemHandleDesc.type = cudaExternalMemoryHandleTypeOpaqueWin32;
	cudaExtMemHandleDesc.handle.win32.handle = imageHandle;
	cudaExtMemHandleDesc.size = imageSize_;
	cudaExtMemHandleDesc.flags = 0;

	CUDA_CHECK(cudaImportExternalMemory(&cudaExtImageMemory_, &cudaExtMemHandleDesc));

	cudaExternalMemoryMipmappedArrayDesc cudaExtMemMipArrayDesc;
	std::memset(&cudaExtMemMipArrayDesc, 0, sizeof(cudaExtMemMipArrayDesc));
	auto chanDesc = cudaChannelFormatDescFromVkFormat(format_);

	cudaExtMemMipArrayDesc.formatDesc = chanDesc; // { 8, 8, 8, 8, cudaChannelFormatKindUnsigned };
	cudaExtMemMipArrayDesc.extent = { extent_.width, extent_.height, 0 }; // depth is 0 for 2D extent...
	cudaExtMemMipArrayDesc.flags = 0;
	cudaExtMemMipArrayDesc.numLevels = 1;

	CUDA_CHECK(cudaExternalMemoryGetMappedMipmappedArray(
		&cudaMipmappedArray_, cudaExtImageMemory_, &cudaExtMemMipArrayDesc));

	CUDA_CHECK(cudaGetMipmappedArrayLevel(&cudaArray_, cudaMipmappedArray_, 0));

	//// Create a surface object from the cudaArray
	cudaResourceDesc resDesc;
	std::memset(&resDesc, 0, sizeof(resDesc));
	resDesc.resType = cudaResourceTypeArray;
	resDesc.res.array.array = cudaArray_;

	CUDA_CHECK(cudaCreateSurfaceObject(&cudaSurface_, &resDesc));
}

void Texture::cudaVkSemaphoreWait(cudaExternalSemaphore_t semaphore, uint64_t waitValue, cudaStream_t stream) {
	cudaExternalSemaphoreWaitParams extSemaphoreWaitParams;
	std::memset(&extSemaphoreWaitParams, 0, sizeof(extSemaphoreWaitParams));
	extSemaphoreWaitParams.params.fence.value = waitValue;
	extSemaphoreWaitParams.flags = 0;

	CUDA_CHECK(cudaWaitExternalSemaphoresAsync(
		&semaphore, &extSemaphoreWaitParams, 1, stream));
}

void Texture::cudaVkSemaphoreSignal(cudaExternalSemaphore_t semaphore, uint64_t signalValue, cudaStream_t stream) {
	cudaExternalSemaphoreSignalParams extSemaphoreSignalParams;
	std::memset(&extSemaphoreSignalParams, 0, sizeof(extSemaphoreSignalParams));
	extSemaphoreSignalParams.params.fence.value = signalValue;
	extSemaphoreSignalParams.flags = 0;

	CUDA_CHECK(cudaSignalExternalSemaphoresAsync(
		&semaphore, &extSemaphoreSignalParams, 1, stream));
}

void Texture::setCudaCopySurfaceFunc(CudaFlags flags)
{
	if (!(flags & CudaFlagBits::CHW || flags & CudaFlagBits::HWC)) flags |= CudaFlagBits::CHW;

	if (flags & CudaFlagBits::CHW)
	{
		switch (format_)
		{
		case VK_FORMAT_B8G8R8A8_UNORM:
		{
			if (flags & CudaFlagBits::BGRA)		 copySurfaceFunc_ = [](void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream) {return memCopySurfaceToPlanar<uchar4, uint8_t, 0, 1, 2, 3>(dst, width, height, src, stream); };
			else if (flags & CudaFlagBits::BGR)  copySurfaceFunc_ = [](void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream) {return memCopySurfaceToPlanar<uchar4, uint8_t, 0, 1, 2>(dst, width, height, src, stream); };
			else if (flags & CudaFlagBits::RGBA) copySurfaceFunc_ = [](void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream) {return memCopySurfaceToPlanar<uchar4, uint8_t, 2, 1, 0, 3>(dst, width, height, src, stream); };
			else if (flags & CudaFlagBits::RGB)  copySurfaceFunc_ = [](void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream) {return memCopySurfaceToPlanar<uchar4, uint8_t, 2, 1, 0>(dst, width, height, src, stream); };
			else								 copySurfaceFunc_ = [](void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream) {return memCopySurfaceToPlanar<uchar4, uint8_t, 2, 1, 0, 3>(dst, width, height, src, stream); };
			return;
		}
		case VK_FORMAT_R32G32B32A32_SFLOAT:
		{
			if (flags & CudaFlagBits::RGBA)		 copySurfaceFunc_ = [](void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream) {return memCopySurfaceToPlanar<float4, float, 0, 1, 2, 3>(dst, width, height, src, stream); };
			else if (flags & CudaFlagBits::RGB)  copySurfaceFunc_ = [](void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream) {return memCopySurfaceToPlanar<float4, float, 0, 1, 2>(dst, width, height, src, stream); };
			else if (flags & CudaFlagBits::BGRA) copySurfaceFunc_ = [](void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream) {return memCopySurfaceToPlanar<float4, float, 2, 1, 0, 3>(dst, width, height, src, stream); };
			else if (flags & CudaFlagBits::BGR)  copySurfaceFunc_ = [](void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream) {return memCopySurfaceToPlanar<float4, float, 2, 1, 0>(dst, width, height, src, stream); };
			else								 copySurfaceFunc_ = [](void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream) {return memCopySurfaceToPlanar<float4, float, 0, 1, 2, 3>(dst, width, height, src, stream); };
			return;
		}
		case VK_FORMAT_R32G32_SFLOAT:
		{
			copySurfaceFunc_ = [](void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream) {return memCopySurfaceToPlanar<float2, float, 0, 1>(dst, width, height, src, stream); };
			return;
		}
		case VK_FORMAT_R32_SFLOAT:
		{
			copySurfaceFunc_ = [](void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream) {return memCopySurface<float>(dst, width, height, src, stream); };
			return;
		}
		default:
			copySurfaceFunc_ = nullptr;
			return;
		}
	}
	else
	{
		switch (format_)
		{
		case VK_FORMAT_B8G8R8A8_UNORM:
		{
			if (flags & CudaFlagBits::BGRA)		 copySurfaceFunc_ = [](void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream) { return memCopySurface<uchar4>(dst, width, height, src, stream); };
			else if (flags & CudaFlagBits::BGR)  copySurfaceFunc_ = [](void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream) { return memCopySurface<uchar4, uint8_t, 0, 1, 2>(dst, width, height, src, stream); };
			else if (flags & CudaFlagBits::RGBA) copySurfaceFunc_ = [](void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream) { return memCopySurface<uchar4, uint8_t, 2, 1, 0, 3>(dst, width, height, src, stream); };
			else if (flags & CudaFlagBits::RGB)  copySurfaceFunc_ = [](void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream) { return memCopySurface<uchar4, uint8_t, 2, 1, 0>(dst, width, height, src, stream); };
			else								 copySurfaceFunc_ = [](void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream) { return memCopySurface<uchar4, uint8_t, 2, 1, 0, 3>(dst, width, height, src, stream); };
			return;
		}
		case VK_FORMAT_R32G32B32A32_SFLOAT:
		{
			if (flags & CudaFlagBits::RGBA)		 copySurfaceFunc_ = [](void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream) { return memCopySurface<float4>(dst, width, height, src, stream); };
			else if (flags & CudaFlagBits::RGB)  copySurfaceFunc_ = [](void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream) { return memCopySurface<float4, float, 0, 1, 2>(dst, width, height, src, stream); };
			else if (flags & CudaFlagBits::BGRA) copySurfaceFunc_ = [](void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream) { return memCopySurface<float4, float, 2, 1, 0, 3>(dst, width, height, src, stream); };
			else if (flags & CudaFlagBits::BGR)  copySurfaceFunc_ = [](void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream) { return memCopySurface<float4, float, 2, 1, 0>(dst, width, height, src, stream); };
			else								 copySurfaceFunc_ = [](void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream) { return memCopySurface<float4>(dst, width, height, src, stream); };
			return;
		}
		case VK_FORMAT_R32G32_SFLOAT:
		{
			copySurfaceFunc_ = [](void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream) { return memCopySurface<float2>(dst, width, height, src, stream); };
			return;
		}
		case VK_FORMAT_R32_SFLOAT:
		{
			copySurfaceFunc_ = [](void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream) { return memCopySurface<float>(dst, width, height, src, stream); };
			return;
		}
		default:
			copySurfaceFunc_ = nullptr;
			return;
		}
	}

}

void Texture::setCudaCopyToSurfaceFunc()
{
	SPDLOG_DEBUG("Setting CopyToSurfaceFunc");
	SPDLOG_FLUSH_DEBUG

	auto flags = cudaMemory_.desc.flags;
	if (!(flags & CudaFlagBits::CHW || flags & CudaFlagBits::HWC)) flags |= CudaFlagBits::CHW;

	if (flags & CudaFlagBits::CHW)
	{
		uint32_t numComponents = cudaMemory_.desc.shape[0];

		switch (format_)
		{
		case VK_FORMAT_B8G8R8A8_UNORM: // default is RGBA
		{
			if (flags & CudaFlagBits::BGRA)      copyToSurfaceFunc_ = [](cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream) { return memCopyPlanarToSurface<uchar4, uint8_t, 0, 1, 2, 3>(dst, width, height, src, stream); };
			else if (flags & CudaFlagBits::BGR)  copyToSurfaceFunc_ = [](cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream) { return memCopyPlanarToSurface<uchar4, uint8_t, 0, 1, 2>(dst, width, height, src, stream); };
			else if (flags & CudaFlagBits::RGBA) copyToSurfaceFunc_ = [](cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream) { return memCopyPlanarToSurface<uchar4, uint8_t, 2, 1, 0, 3>(dst, width, height, src, stream); };
			else if (flags & CudaFlagBits::RGB)  copyToSurfaceFunc_ = [](cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream) { return memCopyPlanarToSurface<uchar4, uint8_t, 2, 1, 0>(dst, width, height, src, stream); };
			else
			{
				if (numComponents == 4)			 copyToSurfaceFunc_ = [](cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream) { return memCopyPlanarToSurface<uchar4, uint8_t, 2, 1, 0, 3>(dst, width, height, src, stream); };
				else if (numComponents == 3)	 copyToSurfaceFunc_ = [](cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream) { return memCopyPlanarToSurface<uchar4, uint8_t, 2, 1, 0>(dst, width, height, src, stream); };
			}
			return;
		}
		case VK_FORMAT_R32G32B32A32_SFLOAT: // default is RGBA
		{
			if (flags & CudaFlagBits::RGBA)      copyToSurfaceFunc_ = [](cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream) { return memCopyPlanarToSurface<float4, float, 0, 1, 2, 3>(dst, width, height, src, stream); };
			else if (flags & CudaFlagBits::RGB)  copyToSurfaceFunc_ = [](cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream) { return memCopyPlanarToSurface<float4, float, 0, 1, 2>(dst, width, height, src, stream); };
			else if (flags & CudaFlagBits::BGRA) copyToSurfaceFunc_ = [](cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream) { return memCopyPlanarToSurface<float4, float, 2, 1, 0, 3>(dst, width, height, src, stream); };
			else if (flags & CudaFlagBits::BGR)  copyToSurfaceFunc_ = [](cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream) { return memCopyPlanarToSurface<float4, float, 2, 1, 0>(dst, width, height, src, stream); };
			else
			{
				if (numComponents == 4)			 copyToSurfaceFunc_ = [](cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream) { return memCopyPlanarToSurface<float4, float, 0, 1, 2, 3>(dst, width, height, src, stream); };
				else if (numComponents == 3)	 copyToSurfaceFunc_ = [](cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream) { return memCopyPlanarToSurface<float4, float, 0, 1, 2>(dst, width, height, src, stream); };
			}

			return;
		}
		case VK_FORMAT_R32G32_SFLOAT:
		{
			copyToSurfaceFunc_ = [](cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream) { return memCopyPlanarToSurface<float2, float, 0, 1>(dst, width, height, src, stream); };
			return;
		}
		case VK_FORMAT_R32_SFLOAT:
		{
			copyToSurfaceFunc_ = [](cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream) { return memCopyToSurface<float>(dst, width, height, src, stream); };
			return;
		}
		default:
			copyToSurfaceFunc_ = nullptr;
			return;
		}
	}
	else // HWC - interleaved functions
	{
		uint32_t numComponents = cudaMemory_.desc.shape[2];

		//std::cout << "Copying HWC to Surface, numComponents: " << numComponents << ", format: " << string_VkFormat(format_) << std::endl;

		switch(format_)
		{
		case VK_FORMAT_B8G8R8A8_UNORM: // default is RGBA
		{
			if		(flags & CudaFlagBits::BGRA) copyToSurfaceFunc_ = [](cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream) { return memCopyToSurface<uchar4>(dst, width, height, src, stream); };
			else if (flags & CudaFlagBits::BGR)  copyToSurfaceFunc_ = [](cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream) { return memCopyToSurface<uchar4, uint8_t, 0, 1, 2>(dst, width, height, src, stream); };
			else if (flags & CudaFlagBits::RGBA) copyToSurfaceFunc_ = [](cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream) { return memCopyToSurface<uchar4, uint8_t, 2, 1, 0, 3>(dst, width, height, src, stream); };
			else if (flags & CudaFlagBits::RGB)  copyToSurfaceFunc_ = [](cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream) { return memCopyToSurface<uchar4, uint8_t, 2, 1, 0>(dst, width, height, src, stream); };
			else
			{
				if (numComponents == 4)			 copyToSurfaceFunc_ = [](cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream) { return memCopyToSurface<uchar4, uint8_t, 2, 1, 0, 3>(dst, width, height, src, stream); };
				else if (numComponents == 3)	 copyToSurfaceFunc_ = [](cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream) { return memCopyToSurface<uchar4, uint8_t, 2, 1, 0>(dst, width, height, src, stream); };
			}
			return;
		}
		case VK_FORMAT_R32G32B32A32_SFLOAT: // default is RGBA
		{
			if		(flags & CudaFlagBits::RGBA) copyToSurfaceFunc_ = [](cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream) { return memCopyToSurface<float4>(dst, width, height, src, stream); };
			else if (flags & CudaFlagBits::RGB)  copyToSurfaceFunc_ = [](cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream) { return memCopyToSurface<float4, float, 0, 1, 2>(dst, width, height, src, stream); };
			else if (flags & CudaFlagBits::BGRA) copyToSurfaceFunc_ = [](cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream) { return memCopyToSurface<float4, float, 2, 1, 0, 3>(dst, width, height, src, stream); };
			else if (flags & CudaFlagBits::BGR)  copyToSurfaceFunc_ = [](cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream) { return memCopyToSurface<float4, float, 2, 1, 0>(dst, width, height, src, stream); };
			else								 
			{
				if (numComponents == 4)			 copyToSurfaceFunc_ = [](cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream) { return memCopyToSurface<float4>(dst, width, height, src, stream); };
				else if (numComponents == 3)	 copyToSurfaceFunc_ = [](cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream) { return memCopyToSurface<float4, float, 2, 1, 0>(dst, width, height, src, stream); };
			}
			return;
		}
		case VK_FORMAT_R32G32_SFLOAT:
		{
			copyToSurfaceFunc_ = [](cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream) { return memCopyToSurface<float2>(dst, width, height, src, stream); };
			return;
		}
		case VK_FORMAT_R32_SFLOAT:
		{
			copyToSurfaceFunc_ = [](cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream) { return memCopyToSurface<float>(dst, width, height, src, stream); };
			return;
		}
		default:
			copyToSurfaceFunc_ = nullptr;
			return;
		}
		
	}

}

void Texture::configureCudaMemory(CudaFlags flags)
{
	std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
	if (requiresCudaMemLock_) lock.lock();

	setCudaCopySurfaceFunc(flags);
	cudaDeviceSynchronize();

	auto numComponents = numComponents_;
	if ((flags & CudaFlagBits::BGR || flags & CudaFlagBits::RGB) &&
		(format_ == VK_FORMAT_B8G8R8A8_UNORM || format_ == VK_FORMAT_R32G32B32A32_SFLOAT))
	{
		numComponents = 3;
	}

	cudaBufferSize_ = extent_.width * extent_.height * numComponents * componentSize_;

	if (cudaBuffer_) CUDA_CHECK(cudaFree(cudaBuffer_));
	CUDA_CHECK(cudaMalloc((void**)&cudaBuffer_, cudaBufferSize_));

	if (flags & CudaFlagBits::HWC)
	{
		cudaMemory_.desc.shape[0] = extent_.height;
		cudaMemory_.desc.shape[1] = extent_.width;
		cudaMemory_.desc.shape[2] = numComponents;

		// Interleaved memory layout
		cudaMemory_.desc.strides[0] = extent_.width * numComponents; //  num elements to next row
		cudaMemory_.desc.strides[1] = numComponents; // num elements to next pixel
		cudaMemory_.desc.strides[2] = 1; // num elements to next component
	}
	else // CHW
	{
		cudaMemory_.desc.shape[0] = numComponents;
		cudaMemory_.desc.shape[1] = extent_.height;
		cudaMemory_.desc.shape[2] = extent_.width;

		// Planar memory layout
		cudaMemory_.desc.strides[0] = extent_.width * extent_.height; // component stride // num elements to next component
		cudaMemory_.desc.strides[1] = extent_.width; // row stride // num elements to row
		cudaMemory_.desc.strides[2] = 1; // column stride // num elements to column
	}

	cudaMemory_.desc.componentSize = componentSize_;
	cudaMemory_.desc.flags = flags; // TODO: need to set this base on the format if it's not set... 
	cudaMemory_.desc.dataType = cudaDataTypeFromVkFormat(format_);



	cudaMemory_.ptr = cudaBuffer_;
	cudaMemory_.size = cudaBufferSize_;

}
