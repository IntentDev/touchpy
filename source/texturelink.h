#pragma once

#include "links.h"
#include "texture.h"
#include "renderer.h"
#include "common/cuda_helpers.h"
#include <memory>

class TextureLink : public Link<TextureLink>
{
public:
	// need a better solution for this, virtual function addLink in Links requires it...
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

	void addOutputTexture(TouchObject<TEInstance> teInstance, TEVulkanTexture* teTexture);
	void onOutputTextureChange(cudaStream_t cudaStream_);

	void setInputTexture(VkExtent2D extent, VkFormat format);
	void transferTextureToInputLink(TouchObject<TEGraphicsContext> context);

	//template <typename T> make template...
	void copyCudaMemoryToInputTexture(
		uint8_t* memory,
		cudaExternalSemaphore_t waitSemaphore,
		uint64_t waitValue,
		cudaStream_t stream);


	const std::vector<std::unique_ptr<Texture>>& textures() const { return textures_; }

private:
	TouchObject<TEGraphicsContext> context_ { nullptr };
	VkPhysicalDevice physicalDevice_ { nullptr };
	VkDevice device_ { nullptr };
	std::vector<std::unique_ptr<Texture>> textures_;
	std::unordered_map<HANDLE, Texture*> handleMap_;

};

class TextureLinks : public Links<TextureLinks, TextureLink>
{
public:
	TextureLinks() = default;
	TextureLinks(
		TouchObject<TEInstance> instance, 
		TouchObject<TEGraphicsContext> context,
		VkPhysicalDevice physicalDevice,
		VkDevice device) 
		:	Links<TextureLinks, TextureLink>(instance), 
			context_(context),
			physicalDevice_(physicalDevice),
			device_(device) { }

	~TextureLinks() {};

	void addLink(TouchObject<TELinkInfo> linkInfo) override
	{
		links_.push_back(std::make_unique<TextureLink>(instance_, context_, physicalDevice_, device_, linkInfo));
		nameMap_[linkInfo->name] = links_.back().get();
		identifierMap_[linkInfo->identifier] = links_.back().get();
	}

private:
	TouchObject<TEGraphicsContext> context_ { nullptr };
	VkPhysicalDevice physicalDevice_{ nullptr };
	VkDevice device_{ nullptr };



};