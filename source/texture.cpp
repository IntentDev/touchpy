
#include "texture.h"
#include <TouchEngine/TEVulkan.h>
#include <Windows.h>
#include <algorithm>

Texture::Texture()
{
}

Texture::~Texture()
{
	if (imageView_ != VK_NULL_HANDLE)
		vkDestroyImageView(device_, imageView_, nullptr);

	if (memory_ != VK_NULL_HANDLE)
		vkFreeMemory(device_, memory_, nullptr);
	
	if (semaphore_ != VK_NULL_HANDLE)
		vkDestroySemaphore(device_, semaphore_, nullptr);
	
}

Texture::Texture(VkPhysicalDevice physicalDevice_, VkDevice device, TEVulkanTexture* texture)
	:	physicalDevice_(physicalDevice_),
		device_(device),
		flipped_(TETextureGetOrigin(texture) == TETextureOriginBottomLeft)
{

	VkSemaphoreCreateInfo externalSemaphoreCreateInfo{};
	externalSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO;
	externalSemaphoreCreateInfo.flags = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;

	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = &externalSemaphoreCreateInfo;

	std::cout << "Creating Texture from TE" << std::endl;

	VK_CHECK(vkCreateSemaphore(device_, &semaphoreCreateInfo, nullptr, &semaphore_));

	std::cout << "Semaphore Created" << std::endl;

	HANDLE externalSemaphoreHandle;

	VkSemaphoreGetWin32HandleInfoKHR externalSemaphoreHandleInfo{};
	externalSemaphoreHandleInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR;
	externalSemaphoreHandleInfo.semaphore = semaphore_;
	externalSemaphoreHandleInfo.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;

	auto vkGetSemaphoreWin32HandleKHR = PFN_vkGetSemaphoreWin32HandleKHR(
									vkGetDeviceProcAddr(device, "vkGetSemaphoreWin32HandleKHR"));

	if (vkGetSemaphoreWin32HandleKHR == nullptr) 
		std::cout << "vkGetSemaphoreWin32HandleKHR is null" << std::endl;
	
	VK_CHECK(vkGetSemaphoreWin32HandleKHR(
		device_,
		&externalSemaphoreHandleInfo,
		&externalSemaphoreHandle));

	std::cout << "External Semaphore Handle: " << externalSemaphoreHandle << std::endl;

	teSemaphore_ = TEVulkanSemaphoreCreate(
		VK_SEMAPHORE_TYPE_BINARY,
		externalSemaphoreHandle,
		VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT,
		VulkanSemaphoreCallback,
		this);

	semaphoreHandle_ = TEVulkanSemaphoreGetHandle(teSemaphore_);

	std::cout << "TE Semaphore Handle: " << semaphoreHandle_ << std::endl;

	//CloseHandle(externalSemaphoreHandle);

	textureHandle_ = TEVulkanTextureGetHandle(texture);

	std::cout << "TE Texture Handle: " << textureHandle_ << std::endl;

	VkExternalMemoryHandleTypeFlagsKHR handleType = TEVulkanTextureGetHandleType(texture);
	format_ = TEVulkanTextureGetFormat(texture);
	extent_ = {
		static_cast<uint32_t> (TEVulkanTextureGetWidth(texture)),
		static_cast<uint32_t> (TEVulkanTextureGetHeight(texture))
	};


	VkExternalMemoryImageCreateInfo externalMemoryImageCreateInfo = {};
	externalMemoryImageCreateInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
	externalMemoryImageCreateInfo.pNext = nullptr;
	externalMemoryImageCreateInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR;

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

	std::cout << "Allocating Memory Size: " << memRequirements.size
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

	//uint32_t extensionCount = 0;
	//vkEnumerateDeviceExtensionProperties(physicalDevice_, nullptr, &extensionCount, nullptr);
	//std::vector<VkExtensionProperties> extensions(extensionCount);
	//vkEnumerateDeviceExtensionProperties(physicalDevice_, nullptr, &extensionCount, extensions.data());

	//bool extensionFound = std::any_of(extensions.begin(), extensions.end(), [](const VkExtensionProperties& extension) {
	//	return strcmp(extension.extensionName, VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME) == 0;
	//	});

	//if (!extensionFound) {
	//	std::cout << "VK_KHR_external_memory_win32 extension not supported." << std::endl;
	//}
	//else {
	//	std::cout << "VK_KHR_external_memory_win32 extension supported." << std::endl;
	//}

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
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
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

	HANDLE exportTextureHandle;
	VkMemoryGetWin32HandleInfoKHR memoryHandleInfo = {};
	memoryHandleInfo.sType = VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR;
	memoryHandleInfo.memory = memory_;
	memoryHandleInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR;

	std::cout << "Getting vkGetMemoryWin32HandleKHR" << std::endl;
	// need to get the function pointer for vkGetMemoryWin32HandleKHR
	auto vkGetMemoryWin32HandleKHR = PFN_vkGetMemoryWin32HandleKHR(
										vkGetDeviceProcAddr(device_, "vkGetMemoryWin32HandleKHR"));

	if (vkGetMemoryWin32HandleKHR == nullptr) {
		std::cout << "vkGetMemoryWin32HandleKHR is null" << std::endl;
	}

	VK_CHECK(vkGetMemoryWin32HandleKHR(
		device_,
		&memoryHandleInfo,
		&exportTextureHandle));

	std::cout << "Memory Handle: " << exportTextureHandle << std::endl;

	teVkTexture_.take(TEVulkanTextureCreate(
						exportTextureHandle, 
						VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR, 
						format_, 
						extent_.width, 
						extent_.height,
						TETextureOriginBottomLeft,
						kTEVkComponentMappingIdentity,
						VulkanTextureCallback,
						this));		

	// need to check this 
	// CloseHandle(exportTextureHandle);

	VkSemaphoreCreateInfo exportSemaphoreCreateInfo{};
	exportSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO;
	exportSemaphoreCreateInfo.flags = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;


	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = &exportSemaphoreCreateInfo;

	VK_CHECK(vkCreateSemaphore(device_, &semaphoreCreateInfo, nullptr, &semaphore_));

	std::cout << "Semaphore Created" << std::endl;

	HANDLE exportSemaphoreHandle;

	VkSemaphoreGetWin32HandleInfoKHR exportSemaphoreHandleInfo{};
	exportSemaphoreHandleInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR;
	exportSemaphoreHandleInfo.semaphore = semaphore_;
	exportSemaphoreHandleInfo.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;

	auto vkGetSemaphoreWin32HandleKHR = PFN_vkGetSemaphoreWin32HandleKHR(
									vkGetDeviceProcAddr(device, "vkGetSemaphoreWin32HandleKHR"));

	if (vkGetSemaphoreWin32HandleKHR == nullptr) {
		std::cout << "vkGetSemaphoreWin32HandleKHR is null" << std::endl;
	}

	VK_CHECK(vkGetSemaphoreWin32HandleKHR(
				device_, 
				&exportSemaphoreHandleInfo, 
				&exportSemaphoreHandle));

	std::cout << "Semaphore Handle: " << exportSemaphoreHandle << std::endl;

	teSemaphore_ = TEVulkanSemaphoreCreate(
		VK_SEMAPHORE_TYPE_BINARY,
		exportSemaphoreHandle,
		VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT,
		VulkanSemaphoreCallback,
		this);
	// need to check this 
	// CloseHandle(exportSemaphoreHandle);

	std::cout	<< "Texture Created (to TE), width: " << extent_.width 
				<< " height: " << extent_.height << std::endl;
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


