#pragma once

#include <vri/vri.h>
#include <TouchEngine/TouchEngine.h>
#include <common/cuda_helpers.h>

#include <vector>

class Texture
{
public:
	Texture();

	Texture(
		VkPhysicalDevice physicalDevice_, 
		VkDevice device, 
		TEInstance* teInstance, 
		TEVulkanTexture* teTexture
	);

	Texture(
		VkPhysicalDevice physicalDevice_, 
		VkDevice device, 
		VkExtent2D extent, 
		VkFormat format
	);

	~Texture();

	bool                           isValid() const         { return image_ != VK_NULL_HANDLE; }
	VkExtent2D                     extent() const          { return extent_; }
	VkFormat                       format() const          { return format_; }
	bool                           flipped() const         { return flipped_; }
	VkImage                        image() const           { return image_; }
	VkImageView                    imageView() const       { return imageView_; }
	TEVulkanTexture*               teVkTexture() const     { return teVkTexture_; }

	HANDLE                         textureHandle() const   { return textureHandle_; }
	HANDLE                         semaphoreHandle() const { return semaphoreHandle_; }
	VkSemaphore                    semaphore() const       { return semaphore_; }
	TouchObject<TEVulkanSemaphore> teVkSemaphore() const   { return teVkSemaphore_; }

	void importSemaphore(TEInstance* teInstance, TETexture* teTexture);
	void cmdTransitionImageLayout(
		VkCommandBuffer cmdBuffer, 
		VkImageLayout oldLayout, 
		VkImageLayout newLayout
	);

	static void VulkanSemaphoreCallback(HANDLE semaphore, TEObjectEvent event, void* info);
	static void VulkanTextureCallback(HANDLE texture, TEObjectEvent event, void* info);

	uint8_t* cudaMemory() const;
	void* copyCudaMemToTexture(uint32_t* memory) const;

private:
	VkDevice                              device_              { VK_NULL_HANDLE };
	VkPhysicalDevice                      physicalDevice_      { VK_NULL_HANDLE };
	VkExtent2D                            extent_              { 0, 0 };
	VkFormat                              format_              { VK_FORMAT_UNDEFINED };

	VmaAllocationCreateFlags              allocCreateFlags_    { VMA_MEMORY_USAGE_GPU_ONLY };
	bool                                  flipped_             { false };

	VmaAllocation                         allocation_          { VK_NULL_HANDLE };
	VmaAllocationInfo*                    allocInfo_           { VK_NULL_HANDLE };


	VkSemaphore                           semaphore_           { VK_NULL_HANDLE };
	TouchObject<TEVulkanSemaphore>        teVkSemaphore_       { nullptr };

	// For external semaphore
	HANDLE                                semaphoreHandle_     { nullptr };

	VkSemaphoreType                       semaphoreType_       { VK_SEMAPHORE_TYPE_BINARY };

	VkExternalSemaphoreHandleTypeFlagBits semaphoreHandleType_ { VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT };

	HANDLE                                textureHandle_       { nullptr };

	VkImage                               image_               { VK_NULL_HANDLE };
	VkDeviceMemory                        memory_              { VK_NULL_HANDLE };
	VkImageView                           imageView_           { VK_NULL_HANDLE };
	TouchObject<TEVulkanTexture>          teVkTexture_         { nullptr };

	HANDLE getVkSemaphoreHandle(
		VkExternalSemaphoreHandleTypeFlagBitsKHR externalSemaphoreHandleType,
		VkSemaphore& semVkCuda);

	HANDLE getVkMemoryHandle(
		VkExternalMemoryHandleTypeFlagBitsKHR externalMemoryHandleType,
		VkDeviceMemory& memory);

	size_t                  imagePitch_                   { 0 };
	size_t                  imageSize_                    { 0 };

	cudaStream_t            cudaStream_                   { nullptr };

	VkSemaphore             cudaSemaphore_    { VK_NULL_HANDLE };
	cudaExternalSemaphore_t cudaExtSemaphore_ { nullptr };

	//VkSemaphore             cudaCudaUpdateVkSemaphore_    { VK_NULL_HANDLE };
	//cudaExternalSemaphore_t cudaExtCudaUpdateVkSemaphore_ { nullptr };

	cudaExternalMemory_t    cudaExtImageMemory_           { nullptr };
	cudaSurfaceObject_t     cudaSurface_                  { 0 };
	cudaMipmappedArray_t    cudaMipmappedArray_			  { nullptr };
	cudaArray_t             cudaArray_                    { nullptr };
	uint8_t*                cudaBuffer_                   { nullptr };


	void cudaImportSemaphore();
	void cudaImportImageMemory();
	void cudaAllocateMemory();

	void cudaUpdateImageMemory();

	void cudaVkSemaphoreWait(cudaExternalSemaphore_t& extSemaphore);
	void cudaVkSemaphoreSignal(cudaExternalSemaphore_t& extSemaphore);


};


