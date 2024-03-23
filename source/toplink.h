#pragma once

#include "links.h"
#include "texture.h"
#include "renderer.h"

#include <memory>
#include <array>

class TopLink : public Link<TopLink>
{
public:
	// need a better solution for this, virtual function addLink in Links requires it,
	// not to be used... 
	TopLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) 
		:	 Link<TopLink>(instance, linkInfo) { }

	TopLink(
		TouchObject<TEInstance> instance,
		TouchObject<TEGraphicsContext> context,
		VkPhysicalDevice physicalDevice,
		VkDevice device,
		TouchObject<TELinkInfo> linkInfo);

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

};

class InTopLink : public TopLink
{
public:
	InTopLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : TopLink(instance, linkInfo) { }
	InTopLink(
		TouchObject<TEInstance> instance,
		TouchObject<TEGraphicsContext> context,
		VkPhysicalDevice physicalDevice,
		VkDevice device,
		TouchObject<TELinkInfo> linkInfo)
		: TopLink(instance, context, physicalDevice, device, linkInfo) { }

	~InTopLink() { }

	void setInputTexture(VkExtent2D extent, VkFormat format);

	//template <typename T> make template...
	void copyCudaMemoryToInputTexture(
		void* memory,
		VkFormat format,
		VkExtent2D extent,
		cudaExternalSemaphore_t waitSemaphore,
		uint64_t waitValue,
		cudaStream_t stream);

	void copyCudaMemoryToInputTexture(CUDAMemory memory, cudaStream_t stream);

	void transferTextureToInputLink();

	//void copyCudaMemory(void* memory, uint32_t width, uint32_t height, uint32_t numComponents, cudaStream_t stream);
	void copyCudaMemory(const CUDAMemory& cudaMemory, cudaStream_t stream);

};

class InTopLinks : public Links<InTopLinks, InTopLink>
{
public:
	InTopLinks() = default;
	InTopLinks(
		TouchObject<TEInstance> instance,
		TouchObject<TEGraphicsContext> context,
		VkPhysicalDevice physicalDevice,
		VkDevice device)
		:	Links<InTopLinks, InTopLink>(instance),
			context_(context),
			physicalDevice_(physicalDevice),
			device_(device) { }

	~InTopLinks() {};

	void addLink(TouchObject<TELinkInfo> linkInfo) override
	{
		links_.push_back(std::make_unique<InTopLink>(instance_, context_, physicalDevice_, device_, linkInfo));
		nameMap_[linkInfo->name] = links_.back().get();
		identifierMap_[linkInfo->identifier] = links_.back().get();
	}

private:
	TouchObject<TEGraphicsContext> context_{ nullptr };
	VkPhysicalDevice physicalDevice_{ nullptr };
	VkDevice device_{ nullptr };
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
		TouchObject<TELinkInfo> linkInfo)
		: TopLink(instance, context, physicalDevice, device, linkInfo) { }

	~OutTopLink() { }

	void addOutputTexture(TouchObject<TEInstance> teInstance, TEVulkanTexture* teTexture);
	void onOutputTextureChange(cudaStream_t cudaStream_);
	const CUDAMemory& cudaMemory() { return currentTexture()->cudaMemory(); }
	void setRequiresCudaMemLock(bool requiresCudaMemLock);

private:
	bool requiresCudaMemLock_ { false };

};	

class OutTopLinks : public Links<OutTopLinks, OutTopLink>
{
public:
	OutTopLinks() = default;
	OutTopLinks(
		TouchObject<TEInstance> instance,
		TouchObject<TEGraphicsContext> context,
		VkPhysicalDevice physicalDevice,
		VkDevice device)
		: Links<OutTopLinks, OutTopLink>(instance),
		context_(context),
		physicalDevice_(physicalDevice),
		device_(device) { }

	~OutTopLinks() {};

	void addLink(TouchObject<TELinkInfo> linkInfo) override
	{
		links_.push_back(std::make_unique<OutTopLink>(instance_, context_, physicalDevice_, device_, linkInfo));
		nameMap_[linkInfo->name] = links_.back().get();
		identifierMap_[linkInfo->identifier] = links_.back().get();
	}

private:
	TouchObject<TEGraphicsContext> context_{ nullptr };
	VkPhysicalDevice physicalDevice_{ nullptr };
	VkDevice device_{ nullptr };
};
