
#include "vri.h"
#include "vri_initializers.h"
#include "utils/utils.h"
#include "utils/color/color.h"

#include <vulkan/vk_enum_string_helper.h>
#include <iostream>
#include <fstream>
#include <optional>
#include <set>

template<>
void colorSetTo<VkClearColorValue>(const Color& color, VkClearColorValue& vkColor) {
	vkColor.float32[0] = color.r;
	vkColor.float32[1] = color.g;
	vkColor.float32[2] = color.b;
	vkColor.float32[3] = color.a;
}

NAMESPACE_BEGIN(vri)

bool hasStencilComponent(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkFormat findSupportedFormat(
	const VkPhysicalDevice& physicalDevice,
	const std::vector<VkFormat>& candidates, 
	const VkImageTiling& tiling, 
	const VkFormatFeatureFlags& features) {
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}


VkFormat findDepthFormat(const VkPhysicalDevice& physicalDevice) {
	return findSupportedFormat(physicalDevice,
		{ VK_FORMAT_D32_SFLOAT,	VK_FORMAT_D32_SFLOAT_S8_UINT,	VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

VkSampleCountFlagBits getMaxUsableSampleCount(const VkPhysicalDevice& physicalDevice)
{
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
	std::cout << "Device Name: " << physicalDeviceProperties.deviceName << std::endl;
	std::cout << "Max sample count: " << string_VkSampleCountFlags(physicalDeviceProperties.limits.framebufferColorSampleCounts) << std::endl;
	std::cout << "Max depth sample count: " << string_VkSampleCountFlags(physicalDeviceProperties.limits.framebufferDepthSampleCounts) << std::endl;

	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
	if (counts & VK_SAMPLE_COUNT_64_BIT)
		return VK_SAMPLE_COUNT_64_BIT;
	if (counts & VK_SAMPLE_COUNT_32_BIT)
		return VK_SAMPLE_COUNT_32_BIT;
	if (counts & VK_SAMPLE_COUNT_16_BIT)
		return VK_SAMPLE_COUNT_16_BIT;
	if (counts & VK_SAMPLE_COUNT_8_BIT)
		return VK_SAMPLE_COUNT_8_BIT;
	if (counts & VK_SAMPLE_COUNT_4_BIT)
		return VK_SAMPLE_COUNT_4_BIT;
	if (counts & VK_SAMPLE_COUNT_2_BIT)
		return VK_SAMPLE_COUNT_2_BIT;

	return VK_SAMPLE_COUNT_1_BIT;
}

void setMSAASampleCount( VContext& vContext, VkSampleCountFlagBits& sampleCount)
{
	VkSampleCountFlagBits maxUsableSampleCount = getMaxUsableSampleCount(vContext.physicalDevice);
	sampleCount = utils::getMinimumBitMask(VK_SAMPLE_COUNT_4_BIT, maxUsableSampleCount);
}

VkShaderModule createShaderModule(const VkDevice& device, const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();

	// Make sure the size is a multiple of sizeof(uint32_t)
	if (code.size() % sizeof(uint32_t) != 0) {
		throw std::runtime_error("Code size is not a multiple of sizeof(uint32_t)");
	}
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	VK_CHECK(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));
	return shaderModule;
}

std::vector<char> readFileSpvFile(const std::string& filename)
{
	std::string relPath = "spv/" + filename + ".spv";
	std::ifstream file(relPath, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "Failed to open file: " << relPath << std::endl;
		abort();
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	std::cout << "Loaded SPIR-V file: " << relPath << " " << fileSize << " bytes" << std::endl;

	return buffer;
}

VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool)
{
	VkCommandBufferAllocateInfo allocInfo = {};
	{
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;
	}

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	{
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	}

	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	return commandBuffer;
}

void endSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue queue)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

uint32_t findMemoryType(
	const VkPhysicalDevice& physicalDevice, 
	const uint32_t& typeFilter, 
	VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && 
			(memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}
	std::cout << "failed to find suitable memory type!" << std::endl;
	throw std::runtime_error("failed to find suitable memory type!");
}

void createBuffer(
	const VContext& vContext, 
	const VkDeviceSize& size, 
	const VkBufferUsageFlags& usage, 
	VkBuffer& buffer, 
	const VmaAllocationCreateFlags& allocCreateFlags, 
	VmaAllocation& allocation, 
	VmaAllocationInfo* allocInfo
)
{
	VkBufferCreateInfo bufferInfo{};

	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;

	const auto& queueFamilyIndices = vContext.queueFamilyIndices.usedFamilyIndices();
	bufferInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
	bufferInfo.pQueueFamilyIndices = queueFamilyIndices.data();

	VmaAllocationCreateInfo allocCreateInfo = {};
	allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocCreateInfo.flags = allocCreateFlags;

	VK_CHECK(vmaCreateBuffer(
		vContext.allocator, 
		&bufferInfo, 
		&allocCreateInfo, 
		&buffer, 
		&allocation, 
		allocInfo)
	);
}

void copyBuffer(const VContext& vContext, const VkBuffer& srcBuffer, VkBuffer dstBuffer, const VkDeviceSize& size, VkDeviceSize srcOffset, VkDeviceSize dstOffset)
{
	const VkDevice& device = vContext.device;
	const VkCommandPool& commandPool = vContext.transferCommandPool;

	VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

	VkBufferCopy copyRegion{};
	{
		copyRegion.size = size;
		copyRegion.srcOffset = srcOffset;
		copyRegion.dstOffset = dstOffset;
	}
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(device, commandPool, commandBuffer, vContext.transferQueue);
}


void createImage2D(
	const VContext& vContext, 
	VkExtent2D extent, 
	VkFormat format, 
	uint32_t mipLevels, 
	VkSampleCountFlagBits numSamples, 
	VkImageTiling tiling, 
	VkImageUsageFlags usage, 
	VkImage& image, 
	VmaAllocationCreateFlags allocCreateFlags, 
	VmaAllocation& allocation, 
	VmaAllocationInfo* allocInfo)
{
	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width = extent.width;
	imageCreateInfo.extent.height = extent.height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = mipLevels;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.format = format;
	imageCreateInfo.tiling = tiling;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage = usage;
	imageCreateInfo.samples = numSamples;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;

	const auto& queueFamilyIndices = vContext.queueFamilyIndices.usedFamilyIndices();
	imageCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
	imageCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();

	VmaAllocationCreateInfo allocCreateInfo = {};
	{
		allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocCreateInfo.flags = allocCreateFlags;
	}
	VK_CHECK(vmaCreateImage(vContext.allocator, &imageCreateInfo, &allocCreateInfo, &image, &allocation, allocInfo));
}

VkImageView createImageView2D(
	VkDevice device, 
	VkImage image, 
	VkFormat format, 
	VkImageAspectFlags aspectFlags, 
	uint32_t mipLevels) 
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	VK_CHECK(vkCreateImageView(device, &viewInfo, nullptr, &imageView));

	return imageView;
}

void destroyImage(const VContext& vContext, VkImage& image, VmaAllocation allocation)
{
	vmaDestroyImage(vContext.allocator, image, allocation);
}


void copyBufferToImage(
	const VContext& vContext, 
	VkBuffer buffer, 
	VkImage image, 
	uint32_t width, 
	uint32_t height)
{
	const auto& device = vContext.device;
	const auto& commandPool = vContext.graphicsCommandPool;
	const auto& graphicsQueue = vContext.graphicsQueue;


	VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);
	VkBufferImageCopy region{};
	{
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };
	}
	vkCmdCopyBufferToImage(
		commandBuffer,
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	endSingleTimeCommands(device, commandPool, commandBuffer, graphicsQueue);
}

void createTextureSampler(const VContext& vContext, VkSampler& sampler, float maxLod)
{
	const auto& device = vContext.device;
	const auto& physicalDeviceProperties = vContext.physicalDeviceProperties;

	VkSamplerCreateInfo samplerCreateInfo = {};
	{
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		samplerCreateInfo.anisotropyEnable = VK_TRUE;
		samplerCreateInfo.maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
		samplerCreateInfo.compareEnable = VK_FALSE;
		samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = maxLod;
	}

	VK_CHECK(vkCreateSampler(device, &samplerCreateInfo, nullptr, &sampler));
}

void generateMipmaps(const VContext& vContext, VkImage image, int32_t texWidth, int32_t texHeight, VkFormat format, uint32_t mipLevels)
{
	const auto& device = vContext.device;
	const auto& commandPool = vContext.graphicsCommandPool;
	const auto& graphicsQueue = vContext.graphicsQueue;



	// Check if image format supports linear blitting
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(vContext.physicalDevice, format, &formatProperties);
	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		throw std::runtime_error("texture image format does not support linear blitting!");
	}

	VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

	VkImageMemoryBarrier imageMemBarrier = {};
	imageMemBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemBarrier.image = image;
	imageMemBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemBarrier.subresourceRange.layerCount = 1;
	imageMemBarrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	for (uint32_t i = 1; i < mipLevels; i++)
	{
		imageMemBarrier.subresourceRange.baseMipLevel = i - 1;
		imageMemBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageMemBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		imageMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &imageMemBarrier);

		VkImageBlit blit = {};
		{
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;
		}
		vkCmdBlitImage(
			commandBuffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);

		imageMemBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageMemBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageMemBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &imageMemBarrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;

	}

	imageMemBarrier.subresourceRange.baseMipLevel = mipLevels - 1;
	imageMemBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageMemBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageMemBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &imageMemBarrier);

	endSingleTimeCommands(device, commandPool, commandBuffer, graphicsQueue);
}

void printAvailableValidationLayers()
{
	// Enumerate available layers
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	// Print layer properties
	std::cout << "Available Vulkan Validation Layers:\n";
	for (const auto& layer : availableLayers)
	{
		std::cout << "Name: " << layer.layerName << "\n";
		std::cout << "Spec Version: " << VK_API_VERSION_MAJOR(layer.specVersion) << "."
			<< VK_API_VERSION_MINOR(layer.specVersion) << "."
			<< VK_API_VERSION_PATCH(layer.specVersion) << "\n";
		std::cout << "Layer Version: " << layer.implementationVersion << "\n";
		std::cout << "Description: " << layer.description << "\n\n";
	}
}

bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers)
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (auto layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

void getInstanceExtensions(std::vector<VkExtensionProperties>& extensions)
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	extensions.resize(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
}

void getDeviceExtensions(const VkPhysicalDevice& physicalDevice, std::vector<VkExtensionProperties>& extensions)
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
	extensions.resize(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensions.data());
}

void listInstanceExtensions()
{
	std::vector<VkExtensionProperties> extensions;
	vri::getInstanceExtensions(extensions);
	std::cout << "available Vulkan instance extensions:" << std::endl;
	for (const auto& extension : extensions) {
		std::cout << '\t' << extension.extensionName << std::endl;
	}
}

void listDeviceExtensions(const VkPhysicalDevice& physicalDevice)
{
	std::vector<VkExtensionProperties> extensions;
	vri::getDeviceExtensions(physicalDevice, extensions);
	std::cout << "available Vulkan device extensions:" << std::endl;
	for (const auto& extension : extensions) {
		std::cout << '\t' << extension.extensionName << std::endl;
	}
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	std::cout << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity =
		//VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		//VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = vri::debugCallback;
	//createInfo.pUserData = nullptr; // Optional
}

VkResult createDebugUtilsMessengerEXT(
	const VkInstance& instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
	const VkAllocationCallbacks* pAllocator, 
	VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		instance, "vkCreateDebugUtilsMessengerEXT"
	);

	if (func != nullptr) 
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void destroyDebugUtilsMessengerEXT(
	const VkInstance& instance,
	VkDebugUtilsMessengerEXT debugMessenger, 
	const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		instance, "vkDestroyDebugUtilsMessengerEXT"
	);

	if (func != nullptr)
		func(instance, debugMessenger, pAllocator);
}

bool deviceExtensionSupport(
	VkPhysicalDevice device, 
	const std::vector<const char*>& requiredDeviceExtensions)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(requiredDeviceExtensions.begin(), requiredDeviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

void createInstance(VContext& vContext,
	std::vector<const char*>& requiredExtensions,
	const std::vector<const char*>& validationLayers,
	bool enableValidationLayers,
	VkDebugUtilsMessengerEXT& debugMessenger)
{
	if (enableValidationLayers)
	{
		requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		if (!vri::checkValidationLayerSupport(validationLayers))
			throw std::runtime_error("Vulkan Validation layers requested, but not available!");
	}

#ifndef NDEBUG
	std::cout << "Required Vulkan instance extensions:" << std::endl;
	for (auto extension : requiredExtensions)
		std::cout << "\t" << extension << std::endl;

	vri::listInstanceExtensions();
#endif

	VkApplicationInfo appInfo{};
	{
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Vision";
		appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
		appInfo.pEngineName = "VisionEngine";
		appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
		appInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
	}

	VkInstanceCreateInfo createInfo{};
	{
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();
	}


	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		vri::populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

		for (uint32_t i = 0; i < createInfo.enabledLayerCount; i++)
			std::cout << "Enabled validation layer: "
			<< createInfo.ppEnabledLayerNames[i]
			<< std::endl;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	VK_CHECK(vkCreateInstance(&createInfo, nullptr, &vContext.instance));

	if (enableValidationLayers)
		VK_CHECK(vri::createDebugUtilsMessengerEXT(
			vContext.instance,
			&debugCreateInfo,
			nullptr,
			&debugMessenger)
		);

	std::cout << "Successfully created a Vulkan Instance!" << std::endl;

}

SwapchainSupport getSwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR windowSurface)
{
	SwapchainSupport swapchainSupport;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, windowSurface, &swapchainSupport.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, windowSurface, &formatCount, nullptr);

	if (formatCount != 0) {
		swapchainSupport.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, windowSurface, &formatCount, swapchainSupport.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, windowSurface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		swapchainSupport.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, windowSurface, &presentModeCount, swapchainSupport.presentModes.data());
	}

	return swapchainSupport;
}

bool deviceSuitable(
	VkPhysicalDevice physicalDevice,
	const std::vector<const char*>& requiredDeviceExtensions,
	VkSurfaceKHR surface, 
	VkQueueFlags queueFlags)
{
	bool extensionsSupported = deviceExtensionSupport(physicalDevice, requiredDeviceExtensions);

	bool resultQueueFamilyIndices = checkDeviceQueueFamilySupport(
		physicalDevice, surface, queueFlags);
	

	bool swapchainAdequate = false;
	if (extensionsSupported) {
		SwapchainSupport swapchainSupport = getSwapchainSupport(physicalDevice, surface);
		swapchainAdequate = 
			!swapchainSupport.formats.empty() && 
			!swapchainSupport.presentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

	std::cout << "Device Name: " << physicalDevice << std::endl
		<< "\tExtensions Supported: " << std::boolalpha << extensionsSupported << std::endl
		<< "\tQueue Family Indices: " << std::boolalpha << resultQueueFamilyIndices << std::endl
		<< "\tSwapchain Adequate: " << std::boolalpha << swapchainAdequate << std::endl
		<< "\tAnisotropy Supported: " << std::boolalpha << supportedFeatures.samplerAnisotropy << std::endl
		;

	return	extensionsSupported && 
			resultQueueFamilyIndices &&
			swapchainAdequate && 
			supportedFeatures.samplerAnisotropy;
}

bool deviceSuitable(
	VkPhysicalDevice physicalDevice,
	const std::vector<const char*>& requiredDeviceExtensions,
	VkQueueFlags queueFlags)
{
	bool extensionsSupported = deviceExtensionSupport(physicalDevice, requiredDeviceExtensions);

	bool resultQueueFamilyIndices = checkDeviceQueueFamilySupport(
		physicalDevice, VK_NULL_HANDLE, queueFlags);

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

	std::cout << "Device Name: " << physicalDevice << std::endl
		<< "\tExtensions Supported: " << std::boolalpha << extensionsSupported << std::endl
		<< "\tQueue Family Indices: " << std::boolalpha << resultQueueFamilyIndices << std::endl
		;

	return	extensionsSupported &&
		resultQueueFamilyIndices &&
		supportedFeatures.samplerAnisotropy;
}


void getPhysicalDevices(const VkInstance& instance, std::vector<VkPhysicalDevice>& physicalDevices)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	physicalDevices.resize(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());
}

void getPhysicalDeviceProperties(const VkPhysicalDevice& physicalDevice, VkPhysicalDeviceProperties& physicalDeviceProperties)
{
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
}

void getPhysicalDeviceFeatures(const VkPhysicalDevice& physicalDevice, VkPhysicalDeviceFeatures& physicalDeviceFeatures)
{
	vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
}

void setPrimaryPhysicalDevice(
	VContext& vContext,
	const std::vector<const char*>& requiredDeviceExtensions,
	VkSurfaceKHR surface,
	VkQueueFlags queueFlags)
{
	std::vector<VkPhysicalDevice> physicalDevices;
	vri::getPhysicalDevices(vContext.instance, physicalDevices);

	for (auto device : physicalDevices)
	{
		std::cout << "checking device: " << device << std::endl;

		if (deviceSuitable(device, requiredDeviceExtensions, surface, queueFlags))
		{	
			std::cout << "found suitable device: " << device << std::endl;
			vContext.physicalDevice = device;
			break;
		}
	}
}

void setPrimaryPhysicalDevice(
	VContext& vContext,
	const std::vector<const char*>& requiredDeviceExtensions,
	VkQueueFlags queueFlags)
{
	std::vector<VkPhysicalDevice> physicalDevices;
	vri::getPhysicalDevices(vContext.instance, physicalDevices);

	for (auto device : physicalDevices)
	{
		std::cout << "checking device: " << device << std::endl;

		if (deviceSuitable(device, requiredDeviceExtensions, queueFlags))
		{
			std::cout << "found suitable device: " << device << std::endl;
			vContext.physicalDevice = device;
			break;
		}
	}
}


bool checkDeviceQueueFamilySupport(
	VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkQueueFlags queueFlags)
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	std::cout << "Queue family count: " << queueFamilyCount << std::endl;

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
	
	VkBool32 presentSupport = false;
	for (uint32_t i = 0; i < 32; i++)
	{
		VkQueueFlags flag = queueFlags & (1 << i);
		if (flag)
		{
			bool flagSupported = false;
			for (uint32_t j = 0; j < queueFamilyCount; j++)
			{
				if (queueFamilies[j].queueFlags & flag)
				{
					flagSupported = true;
					if (surface != VK_NULL_HANDLE && !presentSupport)
					{
						vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, j, surface, &presentSupport);
					}
				}
			}
			if (!flagSupported)
			{
				std::cout << "GPU Device: " << physicalDevice 
					<< " does not support required operation : "
					<< string_VkQueueFlags(flag) << std::endl;
				return false;
			}
		}
	}
	if (surface != VK_NULL_HANDLE)
		return true && presentSupport;
	else
		return true;
}

void createDevice(
	VContext& vContext, 
	VkSurfaceKHR windowSurface,
	const std::vector<const char*>& deviceExtensions,
	const std::vector<const char*>& validationLayers,
	bool enableValidationLayers
	)
{
	vContext.queueFamilyIndices = QueueFamilyIndices{ vContext.physicalDevice, windowSurface };

	// get queue family indices for createDevice then use them after device creation to get queues
	std::vector<deviceQueueInfo> deviceQueueInfos;

	deviceQueueInfos.push_back(
		vContext.queueFamilyIndices.setDeviceQueueInfo(VK_QUEUE_GRAPHICS_BIT, &vContext.graphicsQueue));
	vContext.graphicsFamily = std::get<0>(deviceQueueInfos.back());

	deviceQueueInfos.push_back(
		vContext.queueFamilyIndices.setDeviceQueueInfo(VK_QUEUE_COMPUTE_BIT, &vContext.computeQueue));
	vContext.computeFamily = std::get<0>(deviceQueueInfos.back());

	deviceQueueInfos.push_back(
		vContext.queueFamilyIndices.setDeviceQueueInfo(VK_QUEUE_TRANSFER_BIT, &vContext.transferQueue));
	vContext.transferFamily = std::get<0>(deviceQueueInfos.back());

	deviceQueueInfos.push_back(
		vContext.queueFamilyIndices.setPresentQueueInfo(&vContext.presentQueue, std::nullopt, .9f));
	vContext.presentFamily = std::get<0>(deviceQueueInfos.back());

	VkDeviceCreateInfo createInfo{ .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };

	auto queueCreateInfos = vContext.queueFamilyIndices.queueCreateInfos();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	// look this up - adds overhead... 
	deviceFeatures.sampleRateShading = VK_TRUE; // enable sample shading feature for the device
	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers) 
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
		//listDeviceExtensions(vContext.physicalDevice);
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	VK_CHECK(vkCreateDevice(vContext.physicalDevice, &createInfo, nullptr, &vContext.device));

	for (auto  deviceQueueInfo : deviceQueueInfos)
	{
		vkGetDeviceQueue(vContext.device, 
			std::get<0>(deviceQueueInfo).value(), 
			std::get<1>(deviceQueueInfo), 
			std::get<2>(deviceQueueInfo)
		);
	}
}

void createDevice(
	VContext& vContext,
	const std::vector<const char*>& deviceExtensions,
	const std::vector<const char*>& validationLayers,
	bool enableValidationLayers
)
{
	vContext.queueFamilyIndices = QueueFamilyIndices{ vContext.physicalDevice, VK_NULL_HANDLE };

	// get queue family indices for createDevice then use them after device creation to get queues
	std::vector<deviceQueueInfo> deviceQueueInfos;

	deviceQueueInfos.push_back(
		vContext.queueFamilyIndices.setDeviceQueueInfo(VK_QUEUE_GRAPHICS_BIT, &vContext.graphicsQueue));
	vContext.graphicsFamily = std::get<0>(deviceQueueInfos.back());

	deviceQueueInfos.push_back(
		vContext.queueFamilyIndices.setDeviceQueueInfo(VK_QUEUE_COMPUTE_BIT, &vContext.computeQueue));
	vContext.computeFamily = std::get<0>(deviceQueueInfos.back());

	deviceQueueInfos.push_back(
		vContext.queueFamilyIndices.setDeviceQueueInfo(VK_QUEUE_TRANSFER_BIT, &vContext.transferQueue));
	vContext.transferFamily = std::get<0>(deviceQueueInfos.back());

	VkDeviceCreateInfo createInfo{ .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };

	// get support for timeline semaphore
	VkPhysicalDeviceTimelineSemaphoreFeatures timelineSemaphoreFeatures{};
	timelineSemaphoreFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
	timelineSemaphoreFeatures.timelineSemaphore = VK_TRUE;

	VkPhysicalDeviceFeatures2 deviceFeatures2{};
	deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	deviceFeatures2.pNext = &timelineSemaphoreFeatures;

	createInfo.pNext = &deviceFeatures2;


	auto queueCreateInfos = vContext.queueFamilyIndices.queueCreateInfos();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	//VkPhysicalDeviceFeatures deviceFeatures{};
	//deviceFeatures.samplerAnisotropy = VK_TRUE;

	// look this up - adds overhead... 
	//deviceFeatures.sampleRateShading = VK_TRUE; // enable sample shading feature for the device
	//createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
		//listDeviceExtensions(vContext.physicalDevice);
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	VK_CHECK(vkCreateDevice(vContext.physicalDevice, &createInfo, nullptr, &vContext.device));

	for (auto deviceQueueInfo : deviceQueueInfos)
	{
		vkGetDeviceQueue(vContext.device,
			std::get<0>(deviceQueueInfo).value(),
			std::get<1>(deviceQueueInfo),
			std::get<2>(deviceQueueInfo)
		);
	}
}


void createVmaAllocator(
	VContext&		vContext)
{
	VmaAllocatorCreateInfo allocatorInfo = {};
	{
		allocatorInfo.physicalDevice = vContext.physicalDevice;
		allocatorInfo.device = vContext.device;
		allocatorInfo.instance = vContext.instance;
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
		//allocatorInfo.flags;
		allocatorInfo.preferredLargeHeapBlockSize = 0; // 0 sets default of 256MiB 
		allocatorInfo.pAllocationCallbacks = nullptr;
		allocatorInfo.pDeviceMemoryCallbacks = nullptr;
	}

	VK_CHECK(vmaCreateAllocator(&allocatorInfo, &vContext.allocator));

}

// temp for comparison with push descriptors 
void createDescriptorPool(
	VContext&			vContext, 
	VkDescriptorPool&		descriptorPool,
	uint8_t					framesInFlight) 
{

	// 20 is an arbitrary number, this should be the number of unique descriptors of each type

	std::vector<VkDescriptorPoolSize> poolSizes = {};
	poolSizes.resize(3);

	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = 20 * static_cast<uint32_t>(framesInFlight);
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = 20 * static_cast<uint32_t>(framesInFlight);
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSizes[2].descriptorCount = 20 * static_cast<uint32_t>(framesInFlight);
	uint32_t maxSets = 20 * static_cast<uint32_t>(framesInFlight) + 1;

	VkDescriptorPoolCreateInfo poolCreateInfo = descriptorPoolCreateInfo(poolSizes, maxSets);
	VK_CHECK(vkCreateDescriptorPool(vContext.device, &poolCreateInfo, nullptr, &descriptorPool));

}

void createCommandPool(
	VkDevice			device, 
	uint32_t			queueFamilyIndex, 
	const VkFlags&		flags, 
	VkCommandPool&		commandPool)
{
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndex;
	poolInfo.flags = flags;
	VK_CHECK(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool));
}

void allocateGraphicsCommandBuffers(VContext& vContext)
{
	vContext.graphicsCommandBuffers.resize(vContext.maxFramesInFlight);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = vri::commandBufferAllocateInfo(
		vContext.graphicsCommandPool,
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		vContext.maxFramesInFlight);

	VK_CHECK(vkAllocateCommandBuffers(
		vContext.device,
		&commandBufferAllocateInfo,
		vContext.graphicsCommandBuffers.data()));
}

void allocateCommandBuffers(
	VkDevice device,
	VkCommandPool commandPool,
	std::vector<VkCommandBuffer>& commandBuffers,
	uint32_t bufferCount)
{
	commandBuffers.resize(bufferCount);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = vri::commandBufferAllocateInfo(
		commandPool,
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		bufferCount);


	VK_CHECK(vkAllocateCommandBuffers(	device,
										&commandBufferAllocateInfo,
										commandBuffers.data()));
}

VkCommandBuffer allocateCommandBuffer(VkDevice device, VkCommandPool commandPool) 
{
	VkCommandBufferAllocateInfo allocInfo = vri::commandBufferAllocateInfo(
		commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);

	VkCommandBuffer commandBuffer;
	VK_CHECK(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));
	return commandBuffer;
}

void freeCommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer) 
{
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

//std::vector<VkCommandBuffer> allocateCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t count)
//{
//	VkCommandBufferAllocateInfo allocInfo = vri::commandBufferAllocateInfo(
//		commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, count);
//
//	std::vector<VkCommandBuffer> commandBuffers(count);
//	VK_CHECK(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()));
//	return commandBuffers;
//}

void freeCommandBuffers(VkDevice device, VkCommandPool commandPool, std::vector<VkCommandBuffer> commandBuffers) 
{
	vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
}

void transitionImageLayout(
	VkDevice device,
	VkCommandPool commandPool,
	VkQueue queue,
	VkImage image,
	VkFormat format,
	VkImageLayout oldLayout,
	VkImageLayout newLayout,
	uint32_t mipLevels)
{
	// should be split up into two functions, one for depth and one for color...
	/////
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

	VkImageMemoryBarrier imageMemBarrier{};
	imageMemBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemBarrier.oldLayout = oldLayout;
	imageMemBarrier.newLayout = newLayout;
	imageMemBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemBarrier.image = image;
	imageMemBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemBarrier.subresourceRange.baseMipLevel = 0;
	imageMemBarrier.subresourceRange.levelCount = mipLevels;
	imageMemBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemBarrier.subresourceRange.layerCount = 1;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		imageMemBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (hasStencilComponent(format)) {
			imageMemBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else {
		imageMemBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}


	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags dstStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		imageMemBarrier.srcAccessMask = 0;
		imageMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		imageMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		imageMemBarrier.srcAccessMask = 0;
		imageMemBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
		throw std::invalid_argument("Unsupported layout transition!");


	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, dstStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &imageMemBarrier);

	endSingleTimeCommands(device, commandPool, commandBuffer, queue);
}




NAMESPACE_END(vri)