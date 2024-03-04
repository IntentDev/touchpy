
#include "texture.h"
#include <TouchEngine/TEVulkan.h>
#include <Windows.h>
#include <algorithm>

Texture::Texture(VkPhysicalDevice physicalDevice_, VkDevice device, TEInstance* teInstance, TEVulkanTexture* texture)
	:	physicalDevice_(physicalDevice_),
		device_(device),
		flipped_(TETextureGetOrigin(texture) == TETextureOriginBottomLeft)

{
	textureHandle_ = TEVulkanTextureGetHandle(texture);

	std::cout << "TE Texture Handle: " << textureHandle_ << std::endl;

	VkExternalMemoryHandleTypeFlagsKHR handleType = TEVulkanTextureGetHandleType(texture);
	format_ = TEVulkanTextureGetFormat(texture);
	extent_ = {
		static_cast<uint32_t> (TEVulkanTextureGetWidth(texture)),
		static_cast<uint32_t> (TEVulkanTextureGetHeight(texture))
	};

	// need to create function that sets up pitch depending on format and sets the 
	// format for cuda memory allocation
	imagePitch_ = extent_.width * sizeof(uint8_t) * 4;

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

	std::cout << "Image Created: " << image_ << std::endl;

	VkImportMemoryWin32HandleInfoKHR importMemoryInfo = {};
	importMemoryInfo.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR;
	importMemoryInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR;
	importMemoryInfo.handle = textureHandle_; // Handle to the external memory

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image_, &memRequirements);

	uint32_t memoryTypeIndex = vri::findMemoryType(
		physicalDevice_,
		memRequirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);
	imageSize_ = static_cast<size_t>(memRequirements.size);

	std::cout << "Allocating Vk Memory, Size: " << memRequirements.size
		<< " Memory Type Index: " << memoryTypeIndex << std::endl;

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
	memoryAllocateInfo.pNext = &importMemoryInfo;

	// Import the external memory into Vulkan
	VkDeviceMemory externalMemory;
	VK_CHECK(vkAllocateMemory(device_, &memoryAllocateInfo, nullptr, &externalMemory));

	std::cout << "Memory Imported" << std::endl;

	VK_CHECK(vkBindImageMemory(device_, image_, externalMemory, 0));

	std::cout << "Memory Bound to Image" << std::endl;

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

	std::cout	<< "Texture Created (from TE), width: " 
				<< extent_.width << " height: " << extent_.height << std::endl;

	importSemaphore(teInstance, texture);

	setupCudaResources(textureHandle_, semaphoreHandle_, true);
}

Texture::Texture(
	VkPhysicalDevice physicalDevice,
	VkDevice device,
	VkExtent2D extent,
	VkFormat format
)
	:	physicalDevice_(physicalDevice),
		device_(device),
		extent_(extent),
		format_(format)

{
	// need to create function that sets up pitch depending on format and sets the 
	// format for cuda memory allocation
	imagePitch_ = extent_.width * sizeof(uint8_t) * 4;

	std::cout << "Creating Texture to TE" << std::endl;

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

	std::cout << "Image Created" << std::endl;

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

	std::cout	<< "Allocating Memory Size: " << memRequirements.size 
				<< " Memory Type Index: " << memoryTypeIndex << std::endl;

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = &exportMemoryInfo;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = memoryTypeIndex;

	VK_CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &memory_));

	std::cout << "Memory Allocated" << std::endl;

	VK_CHECK(vkBindImageMemory(device, image_, memory_, 0));

	std::cout << "Memory Bound to Image" << std::endl;

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

	std::cout << "Image View Created" << std::endl;

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

	std::cout << "Semaphore Created" << std::endl;

	semaphoreHandle_ = getVkSemaphoreHandle(VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT, semaphore_);

	teVkSemaphore_.set(TEVulkanSemaphoreCreate(
		VK_SEMAPHORE_TYPE_TIMELINE,
		semaphoreHandle_,
		VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT,
		VulkanSemaphoreCallback,
		this));

	ownsSemaphore_ = true;
	ownsImage_ = true;

	setupCudaResources(textureHandle_, semaphoreHandle_, false);

	std::cout	<< "Texture Created (to TE), width: " << extent_.width 
				<< " height: " << extent_.height << std::endl;
}

Texture::~Texture()
{
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
	//if (cudaExtCudaUpdateVkSemaphore_)
	//	CUDA_CHECK(cudaDestroyExternalSemaphore(cudaExtCudaUpdateVkSemaphore_));

	//if (cudaCudaUpdateVkSemaphore_ != VK_NULL_HANDLE)
	//	vkDestroySemaphore(device_, cudaCudaUpdateVkSemaphore_, nullptr);

	if (ownsImage_)
		CloseHandle(textureHandle_);

	if (ownsSemaphore_)
		CloseHandle(semaphoreHandle_);

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

	//std::cout << "TESemaphore: " << teSemaphore.get() << " waitValue: " << waitValue << std::endl;

	if (result == TEResultSuccess)
	{
		//std::cout << "Texture transfer: " << identifier << " : " << waitValue << std::endl;
		if (TESemaphoreGetType(teSemaphore) == TESemaphoreTypeVulkan)
		{
			TEVulkanSemaphore* teVulkanSemaphore = static_cast<TEVulkanSemaphore*>(teSemaphore.get());

			semaphoreHandle_ = TEVulkanSemaphoreGetHandle(teVulkanSemaphore);
			semaphoreType_ = TEVulkanSemaphoreGetType(teVulkanSemaphore);
			semaphoreHandleType_ = TEVulkanSemaphoreGetHandleType(teVulkanSemaphore);

			//std::cout << "Semaphore handle: " << handle 
			// << " type: " << type << " handleType: " << handleType << std::endl;

			VkSemaphoreTypeCreateInfoKHR semaphoreTypeCreateInfo {};
			semaphoreTypeCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO_KHR;
			semaphoreTypeCreateInfo.pNext = nullptr;
			semaphoreTypeCreateInfo.semaphoreType = semaphoreType_;
			semaphoreTypeCreateInfo.initialValue = waitValue;

			VkSemaphoreCreateInfo semaphoreCreateInfo = {
				VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, &semaphoreTypeCreateInfo, 0 };

			//VkSemaphore importSemaphore;
			VK_CHECK(vkCreateSemaphore(
				device_,
				&semaphoreCreateInfo,
				nullptr,
				&semaphore_
			));

			//std::cout << "vkCreateSemaphore: " << string_VkResult(vkResult) << std::endl;


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

			//std::cout << "vkImportSemaphoreWin32HandleKHR: " << string_VkResult(vkResult) << std::endl;
		}
	}
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

//extern cudaError_t
//memCopyFromSurfaceCharBRGA(
//	unsigned char* dst,
//	int width,
//	int height,
//	cudaSurfaceObject_t input,
//	cudaStream_t stream);

void Texture::copyImageToCudaMem(uint64_t& waitValue, cudaStream_t stream)
{
	cudaVkSemaphoreWait(cudaExtSemaphore_, waitValue, stream);
	CUDA_CHECK(memCopyFromSurfaceCharBRGA(cudaBuffer_, extent_.width, extent_.height, cudaSurface_, stream));
	cudaVkSemaphoreSignal(cudaExtSemaphore_, ++waitValue, stream);
	waitValue_ = waitValue;
}

void Texture::copyCudaMemToImage(uint8_t* memory,
	cudaExternalSemaphore_t waitSemaphore,
	cudaExternalSemaphore_t signalSemaphore,
	uint64_t waitValue, 
	uint64_t signalValue,
	cudaStream_t stream)
{	
	
	cudaVkSemaphoreWait(waitSemaphore, waitValue, stream);
	CUDA_CHECK(memCopyToSurfaceCharBRGA(cudaSurface_, extent_.width, extent_.height, memory, stream));
	//waitValue_ = ++waitValue;


	cudaVkSemaphoreSignal(signalSemaphore, signalValue, stream);

}

void Texture::transferToInputLink(TouchObject<TEInstance> teInstance, TouchObject<TEGraphicsContext> context, const char* identifier)
{
	TouchObject<TETexture> texture;
	texture.set(teVkTexture_);
	TEResult result = TEInstanceLinkSetTextureValue( teInstance, identifier, texture, context);
	if (result == TEResultSuccess)
		result = TEInstanceAddTextureTransfer(teInstance, texture, teVkSemaphore_, signalValue_);
		
	if (result != TEResultSuccess)
		std::cout << "transferToInputLink: " << identifier << ", " << TEResultGetDescription(result) << std::endl;
}


void Texture::setupCudaResources(HANDLE imageHandle, HANDLE semaphoreHandle, bool allocateMemory)
{
	cudaImportTimelineSemaphore(semaphoreHandle_);
	cudaImportImageMemory(textureHandle_);

	if (allocateMemory)
		cudaAllocateMemory();
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

	std::cout << "Cuda Imported External Memory:" << cudaExtImageMemory_ << std::endl;

	cudaExternalMemoryMipmappedArrayDesc cudaExtMemMipArrayDesc;
	std::memset(&cudaExtMemMipArrayDesc, 0, sizeof(cudaExtMemMipArrayDesc));
	cudaExtMemMipArrayDesc.formatDesc = { 8, 8, 8, 8, cudaChannelFormatKindUnsigned };
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

void Texture::cudaAllocateMemory()
{
	CUDA_CHECK(cudaMalloc((void**)&cudaBuffer_, imageSize_));
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


