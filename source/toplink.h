#pragma once

#include "links.h"
#include "texture.h"
#include "renderer.h"

#include <memory>
#include <array>

class TopLink : public Link<TopLink>
{
public:
	TopLink(
		TouchObject<TEInstance> instance,
		TouchObject<TEGraphicsContext> context,
		VkPhysicalDevice physicalDevice,
		VkDevice device,
		TouchObject<TELinkInfo> linkInfo,
		cudaStream_t cudaStream);

	// need a better solution for this, virtual function addLink in Links requires it,
	// not to be used... 
	TopLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : Link<TopLink>(instance, linkInfo) { }

	~TopLink();

	Texture* currentTexture() { return handleMap_[currentTextureHandle_]; }
	const std::vector<std::unique_ptr<Texture>>& textures() const { return textures_; }
	std::array<size_t, 3> shape();

protected:
	TouchObject<TEGraphicsContext> context_ { nullptr };
	VkPhysicalDevice physicalDevice_ { nullptr };
	VkDevice device_ { nullptr };
	std::vector<std::unique_ptr<Texture>> textures_;
	std::unordered_map<HANDLE, Texture*> handleMap_;

	HANDLE currentTextureHandle_ { nullptr };

	cudaStream_t cudaStream_ { nullptr };
};

class InTopLink : public TopLink
{
public:
	InTopLink(
		TouchObject<TEInstance> instance,
		TouchObject<TEGraphicsContext> context,
		VkPhysicalDevice physicalDevice,
		VkDevice device,
		TouchObject<TELinkInfo> linkInfo,
		cudaStream_t cudaStream)
		:	TopLink(instance, context, physicalDevice, device, linkInfo, cudaStream) { }

	// need a better solution for this, virtual function addLink in Links requires it,
	// not to be used...
	InTopLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : TopLink(instance, linkInfo) { }

	~InTopLink() { }

	void copyCudaMemory(const CUDAMemory& cudaMem, cudaStream_t stream);
	void copyCudaMemory(const CUDAMemory& cudaMem);

	// not used at this moment, is useful if we want to copy external memory (outLink) to input texture
	//void copyExternalCudaMemory(
	//	void* memory,
	//	VkFormat format,
	//	VkExtent2D extent,
	//	cudaExternalSemaphore_t waitSemaphore,
	//	uint64_t waitValue,
	//	cudaStream_t stream,
	//	CudaFlags flags);

private:
	void setInputTexture(VkExtent2D extent, VkFormat format, CUDAMemoryDesc cudaMemDesc);
	void transferTextureToInputLink();
};

class InTopLinks : public Links<InTopLinks, InTopLink>
{
public:
	InTopLinks() = default;
	InTopLinks(
		TouchObject<TEInstance> instance,
		TouchObject<TEGraphicsContext> context,
		VkPhysicalDevice physicalDevice,
		VkDevice device,
		cudaStream_t compCudaStream)
		:	Links<InTopLinks, InTopLink>(instance),
			context_(context),
			physicalDevice_(physicalDevice),
			device_(device),
			compCudaStream_(compCudaStream) { }

	~InTopLinks() {};

	void addLink(TouchObject<TELinkInfo> linkInfo) override
	{
		links_.push_back(std::make_unique<InTopLink>(instance_, context_, physicalDevice_, device_, linkInfo, compCudaStream_));
		nameMap_[linkInfo->name] = links_.back().get();
		identifierMap_[linkInfo->identifier] = links_.back().get();
	}

private:
	TouchObject<TEGraphicsContext> context_{ nullptr };
	VkPhysicalDevice physicalDevice_{ nullptr };
	VkDevice device_{ nullptr };
	cudaStream_t compCudaStream_{ nullptr };
};


class OutTopLink : public TopLink
{
public:
	OutTopLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : TopLink(instance, linkInfo) { }
	OutTopLink(
		TouchObject<TEInstance> instance,
		TouchObject<TEGraphicsContext> context,
		VkPhysicalDevice physicalDevice,
		VkDevice device,
		TouchObject<TELinkInfo> linkInfo,
		cudaStream_t cudaStream)
		:	TopLink(instance, context, physicalDevice, device, linkInfo, cudaStream) { }

	~OutTopLink() { }

	void addOutputTexture(TouchObject<TEInstance> teInstance, TEVulkanTexture* teTexture);
	void onOutputTextureChange();
	void setRequiresCudaMemLock(bool requiresCudaMemLock);

	const CUDAMemory& cudaMemory() { return currentTexture()->cudaMemory(); }
	void setCudaFlags(CudaFlags flags);
	void setCudaStream(cudaStream_t stream) { cudaStream_ = stream; }

private:
	bool requiresCudaMemLock_ { false };
	CudaFlags cudaFlags_ { CudaFlagBits::None };

};	

class OutTopLinks : public Links<OutTopLinks, OutTopLink>
{
public:
	OutTopLinks() = default;
	OutTopLinks(
		TouchObject<TEInstance> instance,
		TouchObject<TEGraphicsContext> context,
		VkPhysicalDevice physicalDevice,
		VkDevice device,
		cudaStream_t compCudaStream)
		: Links<OutTopLinks, OutTopLink>(instance),
		context_(context),
		physicalDevice_(physicalDevice),
		device_(device),
		compCudaStream_(compCudaStream) { }

	~OutTopLinks() {};

	void addLink(TouchObject<TELinkInfo> linkInfo) override
	{
		links_.push_back(std::make_unique<OutTopLink>(instance_, context_, physicalDevice_, device_, linkInfo, compCudaStream_));
		nameMap_[linkInfo->name] = links_.back().get();
		identifierMap_[linkInfo->identifier] = links_.back().get();
	}

private:
	TouchObject<TEGraphicsContext> context_{ nullptr };
	VkPhysicalDevice physicalDevice_{ nullptr };
	VkDevice device_{ nullptr };
	cudaStream_t compCudaStream_{ nullptr };
};
