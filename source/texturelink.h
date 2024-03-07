#pragma once

#include "links.h"
#include "texture.h"
#include "renderer.h"
#include "common/cuda_helpers.h"
#include <memory>
#include <array>

class TextureLink : public Link<TextureLink>
{
public:
	// need a better solution for this, virtual function addLink in Links requires it,
	// not to be used... 
	TextureLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) 
		:	 Link<TextureLink>(instance, linkInfo) { }

	TextureLink(
		TouchObject<TEInstance> instance,
		TouchObject<TEGraphicsContext> context,
		VkPhysicalDevice physicalDevice,
		VkDevice device,
		TouchObject<TELinkInfo> linkInfo);

	~TextureLink();

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

class InTextureLink : public TextureLink
{
public:
	InTextureLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : TextureLink(instance, linkInfo) { }
	InTextureLink(
		TouchObject<TEInstance> instance,
		TouchObject<TEGraphicsContext> context,
		VkPhysicalDevice physicalDevice,
		VkDevice device,
		TouchObject<TELinkInfo> linkInfo)
		: TextureLink(instance, context, physicalDevice, device, linkInfo) { }

	~InTextureLink() { }

	void setInputTexture(VkExtent2D extent, VkFormat format);

	//template <typename T> make template...
	void copyCudaMemoryToInputTexture(
		uint8_t* memory,
		VkFormat format,
		VkExtent2D extent,
		cudaExternalSemaphore_t waitSemaphore,
		uint64_t waitValue,
		cudaStream_t stream);

	void transferTextureToInputLink(TouchObject<TEGraphicsContext> context);


};

class InTextureLinks : public Links<InTextureLinks, InTextureLink>
{
public:
	InTextureLinks() = default;
	InTextureLinks(
		TouchObject<TEInstance> instance,
		TouchObject<TEGraphicsContext> context,
		VkPhysicalDevice physicalDevice,
		VkDevice device)
		:	Links<InTextureLinks, InTextureLink>(instance),
			context_(context),
			physicalDevice_(physicalDevice),
			device_(device) { }

	~InTextureLinks() {};

	void addLink(TouchObject<TELinkInfo> linkInfo) override
	{
		links_.push_back(std::make_unique<InTextureLink>(instance_, context_, physicalDevice_, device_, linkInfo));
		nameMap_[linkInfo->name] = links_.back().get();
		identifierMap_[linkInfo->identifier] = links_.back().get();
	}

private:
	TouchObject<TEGraphicsContext> context_{ nullptr };
	VkPhysicalDevice physicalDevice_{ nullptr };
	VkDevice device_{ nullptr };
};


class OutTextureLink : public TextureLink
{
public:
	OutTextureLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : TextureLink(instance, linkInfo) { }
	OutTextureLink(
		TouchObject<TEInstance> instance,
		TouchObject<TEGraphicsContext> context,
		VkPhysicalDevice physicalDevice,
		VkDevice device,
		TouchObject<TELinkInfo> linkInfo)
		: TextureLink(instance, context, physicalDevice, device, linkInfo) { }

	~OutTextureLink() { }

	void addOutputTexture(TouchObject<TEInstance> teInstance, TEVulkanTexture* teTexture);
	void onOutputTextureChange(cudaStream_t cudaStream_);
	void* cudaMemory() { return static_cast<void*>(currentTexture()->cudaMemory()); }

};	

class OutTextureLinks : public Links<OutTextureLinks, OutTextureLink>
{
public:
	OutTextureLinks() = default;
	OutTextureLinks(
		TouchObject<TEInstance> instance,
		TouchObject<TEGraphicsContext> context,
		VkPhysicalDevice physicalDevice,
		VkDevice device)
		: Links<OutTextureLinks, OutTextureLink>(instance),
		context_(context),
		physicalDevice_(physicalDevice),
		device_(device) { }

	~OutTextureLinks() {};

	void addLink(TouchObject<TELinkInfo> linkInfo) override
	{
		links_.push_back(std::make_unique<OutTextureLink>(instance_, context_, physicalDevice_, device_, linkInfo));
		nameMap_[linkInfo->name] = links_.back().get();
		identifierMap_[linkInfo->identifier] = links_.back().get();
	}

private:
	TouchObject<TEGraphicsContext> context_{ nullptr };
	VkPhysicalDevice physicalDevice_{ nullptr };
	VkDevice device_{ nullptr };
};
