#pragma once

#include <vri/vri.h>
#include <TouchEngine/TouchEngine.h>
#include <vector>

class Texture
{
public:
	Texture();
	Texture(VkPhysicalDevice physicalDevice_, VkDevice device, const std::vector<uint32_t>& queueFamilyIndices, VkExtent2D extent, VkFormat format);

	Texture(VkDevice device, TEVulkanTexture* teTexture);

	~Texture();

	bool isValid() const { return image_ != VK_NULL_HANDLE; }
	VkExtent2D extent() const { return extent_; }
	VkFormat format() const { return format_; }
	bool flipped() const { return flipped_; }
	VkImage image() const { return image_; }
	VkImageView imageView() const { return imageView_; }
	TEVulkanTexture* teTexture() const { return teTexture_; }

	static void VulkanSemaphoreCallback(HANDLE semaphore, TEObjectEvent event, void* info);
	static void VulkanTextureCallback(HANDLE texture, TEObjectEvent event, void* info);

private:
	VkDevice device_;
	VkPhysicalDevice physicalDevice_;
	VkExtent2D extent_;
	VkFormat format_;

	VmaAllocationCreateFlags allocCreateFlags_ { VMA_MEMORY_USAGE_GPU_ONLY };
	bool flipped_ = false;

	VmaAllocation allocation_ { VK_NULL_HANDLE };
	VmaAllocationInfo* allocInfo_ { VK_NULL_HANDLE };

	VkSemaphore semaphore_ { VK_NULL_HANDLE };
	TEVulkanSemaphore* teSemaphore_ { nullptr };
	HANDLE semaphoreHandle_ { nullptr };
	HANDLE textureHandle_ { nullptr };

	VkImage image_ { VK_NULL_HANDLE };
	VkDeviceMemory memory_ { VK_NULL_HANDLE };
	VkImageView imageView_ { VK_NULL_HANDLE };
	TouchObject<TEVulkanTexture> teTexture_ { nullptr };



};