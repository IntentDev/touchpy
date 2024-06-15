#pragma once

#include "common/helpers.h"

#include "vri_initializers.h"
#include "vri_commandbuffers.h"
#include "vri/vri_macros.h"	

#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
#include "vri_queuefamilyindices.h"
#include "vri_semaphorepool.h"
#include "vri_fencepool.h"

#include <vector>



NAMESPACE_BEGIN(vri)



struct VContext
{
	VkInstance                   instance                 { VK_NULL_HANDLE };
	VkPhysicalDevice             physicalDevice           { VK_NULL_HANDLE };
	VkPhysicalDeviceProperties  physicalDeviceProperties  { };
	VkDevice                     device                   { VK_NULL_HANDLE };


	QueueFamilyIndices			 queueFamilyIndices		  { };

	std::optional<uint32_t>      graphicsFamily           { };
	std::optional<uint32_t>      computeFamily            { };
	std::optional<uint32_t>      transferFamily           { };
	std::optional<uint32_t>      presentFamily            { };
	VkQueue                      graphicsQueue            { VK_NULL_HANDLE };
	VkQueue                      computeQueue             { VK_NULL_HANDLE };
	VkQueue                      transferQueue            { VK_NULL_HANDLE };
	VkQueue                      presentQueue             { VK_NULL_HANDLE };

	VkCommandPool                graphicsCommandPool      { VK_NULL_HANDLE };
	VkCommandPool                transferCommandPool      { VK_NULL_HANDLE };
	VkCommandPool                computeCommandPool       { VK_NULL_HANDLE };
	VkCommandPool                presentCommandPool       { VK_NULL_HANDLE };

	std::vector<VkCommandBuffer> graphicsCommandBuffers   { };
	std::vector<VkCommandBuffer> transferCommandBuffers	  { };
	uint32_t                     maxFramesInFlight        { 2 };
	uint32_t                     currentFrame             { 0 };

	//std::vector <VkSemaphore>  imageAvailableSemaphores {};
	//std::vector <VkSemaphore>  renderFinishedSemaphores {};
	//std::vector <VkFence>		 frameFences			  {};

	VmaAllocator                 allocator                { VK_NULL_HANDLE };
	};

struct SwapchainSupport
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};


VkFormat findDepthFormat(
	const VkPhysicalDevice&	physicalDevice);

bool hasStencilComponent(
	VkFormat format);

VkFormat findSupportedFormat(
	const VkPhysicalDevice&	physicalDevice, 
	const std::vector<VkFormat>& candidates, 
	const VkImageTiling& tiling, 
	const VkFormatFeatureFlags&	features);

VkSampleCountFlagBits getMaxUsableSampleCount(
	const VkPhysicalDevice&	physicalDevice);

void setMSAASampleCount(
	VContext& vContext, 
	VkSampleCountFlagBits& sampleCount);

VkShaderModule createShaderModule(
	const VkDevice&	device, 
	const std::vector<char>& code);

std::vector<char> readFileSpvFile(
	const std::string& filename);

VkCommandBuffer beginSingleTimeCommands(
	VkDevice device, 
	VkCommandPool commandPool);

void endSingleTimeCommands(
	VkDevice device, 
	VkCommandPool commandPool, 
	VkCommandBuffer commandBuffer, 
	VkQueue queue);

uint32_t findMemoryType(
	const VkPhysicalDevice& physicalDevice, 
	const uint32_t& typeFilter, 
	VkMemoryPropertyFlags properties);

void createBuffer(
	const VContext& vContext, 
	const VkDeviceSize& size, 
	const VkBufferUsageFlags& usage, 
	VkBuffer& buffer, 
	const VmaAllocationCreateFlags& allocCreateFlags, 
	VmaAllocation& allocation, 
	VmaAllocationInfo* allocInfo);

void copyBuffer(
	const VContext& vContext, 
	const VkBuffer& srcBuffer, 
	VkBuffer dstBuffer, 
	const VkDeviceSize& size, 
	VkDeviceSize srcOffset = 0, 
	VkDeviceSize dstOffset = 0);

void createImage2D(
	const VContext& vContext, 
	VkExtent2D extent, VkFormat format, 
	uint32_t mipLevels, 
	VkSampleCountFlagBits numSamples, 
	VkImageTiling tiling, 
	VkImageUsageFlags usage, 
	VkImage& image, 
	VmaAllocationCreateFlags allocCreateFlags, 
	VmaAllocation& allocation, 
	VmaAllocationInfo* allocInfo);

VkImageView createImageView2D(
	VkDevice device, 
	VkImage image, 
	VkFormat format, 
	VkImageAspectFlags aspectFlags, 
	uint32_t mipLevels);

void destroyImage(
	const VContext& vContext, 
	VkImage& image, 
	VmaAllocation allocation);

void copyBufferToImage(
	const VContext& vContext, 
	VkBuffer buffer, 
	VkImage image, 
	uint32_t width, 
	uint32_t height);

void createTextureSampler(
	const VContext& vContext, 
	VkSampler& sampler, 
	float maxLod);

void generateMipmaps(
	const VContext& vContext, 
	VkImage image, 
	int32_t texWidth, 
	int32_t texHeight, 
	VkFormat format, 
	uint32_t mipLevels);



//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Extenstions and Validation		

void printAvailableValidationLayers();

bool checkValidationLayerSupport(
	const std::vector<const char*>& validationLayers);

void getInstanceExtensions(
	std::vector<VkExtensionProperties>& extensions);

void getDeviceExtensions(
	const VkPhysicalDevice& physicalDevice, 
	std::vector<VkExtensionProperties>& extensions);

void listInstanceExtensions();

void listDeviceExtensions(const VkPhysicalDevice& physicalDevice);

void populateDebugMessengerCreateInfo(
	VkDebugUtilsMessengerCreateInfoEXT& createInfo);

VkResult createDebugUtilsMessengerEXT(
	const VkInstance& instance, 
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
	const VkAllocationCallbacks* pAllocator, 
	VkDebugUtilsMessengerEXT* pDebugMessenger);

void destroyDebugUtilsMessengerEXT(
	const VkInstance& instance, 
	VkDebugUtilsMessengerEXT debugMessenger, 
	const VkAllocationCallbacks* pAllocator);

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// instance

void createInstance(VContext& vContext,
	std::vector<const char*>& requiredExtensions,
	const std::vector<const char*>& validationLayers,
	bool enableValidationLayers,
	VkDebugUtilsMessengerEXT& debugMessenger);

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Physical Devices

bool deviceExtensionSupport(
	VkPhysicalDevice device, 
	const std::vector<const char*>& requiredDeviceExtensions);

SwapchainSupport getSwapchainSupport(
	VkPhysicalDevice device, 
	VkSurfaceKHR windowSurface);

bool checkDeviceQueueFamilySupport(
	VkPhysicalDevice physicalDevice,
	VkSurfaceKHR surface,
	VkQueueFlags queueFlags);

bool deviceSuitable(
	VkPhysicalDevice physicalDevice,
	const std::vector<const char*>& requiredDeviceExtensions, 
	VkSurfaceKHR surface,
	VkQueueFlags queueFlags);

bool deviceSuitable(
	VkPhysicalDevice physicalDevice,
	const std::vector<const char*>& requiredDeviceExtensions,
	VkQueueFlags queueFlags);

void getPhysicalDevices(
	const VkInstance& instance, 
	std::vector<VkPhysicalDevice>& physicalDevices);

void getPhysicalDeviceProperties(
	const VkPhysicalDevice& physicalDevice, 
	VkPhysicalDeviceProperties& properties);

void getPhysicalDeviceFeatures(
	const VkPhysicalDevice& physicalDevice, 
	VkPhysicalDeviceFeatures& features);

void setPrimaryPhysicalDevice(
	VContext& vContext,
	const std::vector<const char*>& requiredDeviceExtensions,
	VkSurfaceKHR surface,
	VkQueueFlags queueFlags);

void setPrimaryPhysicalDevice(
	VContext& vContext,
	const std::vector<const char*>& requiredDeviceExtensions,
	VkQueueFlags queueFlags);

bool setPrimaryPhysicalDevice(
	VContext& vContext,
	const std::vector<const char*>& requiredDeviceExtensions,
	VkQueueFlags queueFlags,
	uint8_t uuid[16]);

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Device



void createDevice(
	VContext&							vContext,
	VkSurfaceKHR						windowSurface,
	const std::vector<const char*>&		deviceExtensions,
	const std::vector<const char*>&		validationLayers,
	bool								enableValidationLayers);

void createDevice(
	VContext& vContext,
	const std::vector<const char*>& deviceExtensions,
	const std::vector<const char*>& validationLayers,
	bool								enableValidationLayers);

void createVmaAllocator(
	VContext& vContext);

void createDescriptorPool(
	VContext&				vContext, 
	VkDescriptorPool&		descriptorPool,
	uint8_t					framesInFlight);

void createCommandPool(
	VkDevice				device,
	uint32_t				queueFamilyIndex,
	const VkFlags&			flags,
	VkCommandPool&			commandPool);

void allocateGraphicsCommandBuffers(
	VContext& vContext);

void allocateCommandBuffers(
	VkDevice device,
	VkCommandPool commandPool,
	std::vector<VkCommandBuffer>& commandBuffers,
	uint32_t bufferCount);

VkCommandBuffer allocateCommandBuffer(
	VkDevice device, 
	VkCommandPool commandPool);

void freeCommandBuffer(
	VkDevice device, 
	VkCommandPool commandPool, 
	VkCommandBuffer commandBuffer);

//std::vector<VkCommandBuffer> allocateCommandBuffers(
//	VkDevice device, 
//	VkCommandPool commandPool, 
//	uint32_t count);

void freeCommandBuffers(
	VkDevice device, 
	VkCommandPool commandPool, 
	std::vector<VkCommandBuffer> commandBuffers);

void transitionImageLayout(
	VkDevice device,
	VkCommandPool commandPool,
	VkQueue queue,
	VkImage image,
	VkFormat format,
	VkImageLayout oldLayout,
	VkImageLayout newLayout,
	uint32_t mipLevels = 1);


NAMESPACE_END(vri)