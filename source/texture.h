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

	bool                           isValid() const          { return image_ != VK_NULL_HANDLE; }
	VkExtent2D                     extent() const           { return extent_; }
	VkFormat                       format() const           { return format_; }
	bool                           flipped() const          { return flipped_; }
	VkImage                        image() const            { return image_; }
	VkImageView                    imageView() const        { return imageView_; }
	TEVulkanTexture*               teVkTexture() const      { return teVkTexture_; }
	VkImageLayout				   imageLayout() const      { return imageLayout_; }

	HANDLE                         textureHandle() const    { return textureHandle_; }
	HANDLE                         semaphoreHandle() const  { return semaphoreHandle_; }
	VkSemaphore                    semaphore() const        { return semaphore_; }
	uint64_t                       waitValue() const        { return waitValue_; }
	uint64_t                       signalValue() const      { return signalValue_; }
	TouchObject<TEVulkanSemaphore> teVkSemaphore() const    { return teVkSemaphore_; }
	cudaExternalSemaphore_t        cudaExtSemaphore() const { return cudaExtSemaphore_; }

	void                     setSignalValue(uint64_t value) { signalValue_ = value; }

	void importSemaphore(TEInstance* teInstance, TETexture* teTexture);
	void cmdTransitionImageLayout(
		VkCommandBuffer cmdBuffer, 
		VkImageLayout oldLayout, 
		VkImageLayout newLayout
	);

	static void VulkanSemaphoreCallback(HANDLE semaphore, TEObjectEvent event, void* info);
	static void VulkanTextureCallback(HANDLE texture, TEObjectEvent event, void* info);

	void copyImageToCudaMem(uint64_t& waitValue, cudaStream_t stream);

	void copyCudaMemToImage(
		uint8_t* memory,
		cudaExternalSemaphore_t waitSemaphore, 
		cudaExternalSemaphore_t signalSemaphore,
		uint64_t waitValue,
		uint64_t signalValue,
		cudaStream_t stream
	);

	uint8_t* cudaMemory() const { return cudaBuffer_; }

	void transferToInputLink(
		TouchObject<TEInstance> teInstance, 
		TouchObject<TEGraphicsContext> context,
		const char* identifier);

private:
	VkDevice                              device_              { VK_NULL_HANDLE };
	VkPhysicalDevice                      physicalDevice_      { VK_NULL_HANDLE };
	VkExtent2D                            extent_              { 0, 0 };
	VkFormat                              format_              { VK_FORMAT_UNDEFINED };

	bool                                  flipped_             { false };


	VkSemaphore                           semaphore_           { VK_NULL_HANDLE };
	TouchObject<TEVulkanSemaphore>        teVkSemaphore_       { nullptr };

	bool                                  ownsSemaphore_       { false };
	bool                                  ownsImage_           { false };

	// For external semaphore
	HANDLE                                semaphoreHandle_     { nullptr };
	VkSemaphoreType                       semaphoreType_       { VK_SEMAPHORE_TYPE_BINARY };
	VkExternalSemaphoreHandleTypeFlagBits semaphoreHandleType_ { VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT };

	uint64_t                              waitValue_           { 0 };
	uint64_t                              signalValue_         { 0 };

	HANDLE                                textureHandle_       { nullptr };

	VkImage                               image_               { VK_NULL_HANDLE };
	VkDeviceMemory                        memory_              { VK_NULL_HANDLE };
	VkImageView                           imageView_           { VK_NULL_HANDLE };
	TouchObject<TEVulkanTexture>          teVkTexture_         { nullptr };
	VkImageLayout						  imageLayout_         { VK_IMAGE_LAYOUT_UNDEFINED };

	HANDLE getVkSemaphoreHandle(
		VkExternalSemaphoreHandleTypeFlagBitsKHR externalSemaphoreHandleType,
		VkSemaphore& semVkCuda);

	HANDLE getVkMemoryHandle(
		VkExternalMemoryHandleTypeFlagBitsKHR externalMemoryHandleType,
		VkDeviceMemory& memory);

	size_t                  imagePitch_                   { 0 };
	size_t                  imageSize_                    { 0 };

	cudaStream_t            cudaStream_                   { nullptr };

	cudaExternalSemaphore_t cudaExtSemaphore_ { nullptr };

	//VkSemaphore             cudaCudaUpdateVkSemaphore_    { VK_NULL_HANDLE };
	//cudaExternalSemaphore_t cudaExtCudaUpdateVkSemaphore_ { nullptr };

	cudaExternalMemory_t    cudaExtImageMemory_           { nullptr };
	cudaSurfaceObject_t     cudaSurface_                  { 0 };
	cudaMipmappedArray_t    cudaMipmappedArray_			  { nullptr };
	cudaArray_t             cudaArray_                    { nullptr };
	uint8_t*                cudaBuffer_                   { nullptr };


	void setupCudaResources(HANDLE imageHandle, HANDLE semaphoreHandle, bool allocateMemory);
	void cudaImportTimelineSemaphore(HANDLE semaphoreHandle);
	void cudaImportSemaphore(HANDLE semaphoreHandle);
	void cudaImportImageMemory(HANDLE imageHandle);
	void cudaAllocateMemory();

	void cudaVkSemaphoreWait(cudaExternalSemaphore_t semaphore, uint64_t waitValue, cudaStream_t stream);
	void cudaVkSemaphoreSignal(cudaExternalSemaphore_t semaphore, uint64_t signalValue, cudaStream_t stream);


};


cudaError_t
memCopyFromSurfaceCharBRGA(
	uint8_t* dst,
	int width,
	int height,
	cudaSurfaceObject_t src,
	cudaStream_t stream
);

cudaError_t
memCopyToSurfaceCharBRGA(
	cudaSurfaceObject_t output,
	int width,
	int height,
	const uint8_t* src,
	cudaStream_t stream
);