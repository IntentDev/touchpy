#pragma once

#include "common/helpers.h"
#include <vulkan/vulkan.h>
#include <vector>

NAMESPACE_BEGIN(vri)

inline VkMemoryAllocateInfo memoryAllocateInfo()
{
	VkMemoryAllocateInfo memoryAllocation = {};
	memoryAllocation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	return memoryAllocation;
}

inline VkMappedMemoryRange mappedMemoryRange()
{
	VkMappedMemoryRange mappedMemoryRange = {};
	mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	return mappedMemoryRange;
}

inline VkCommandBufferAllocateInfo commandBufferAllocateInfo(
    VkCommandPool        commandPool,
    VkCommandBufferLevel level,
    uint32_t             bufferCount)
{
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool        = commandPool;
	commandBufferAllocateInfo.level              = level;
	commandBufferAllocateInfo.commandBufferCount = bufferCount;
	return commandBufferAllocateInfo;
}

inline VkCommandPoolCreateInfo commandPoolCreateInfo()
{
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	return commandPoolCreateInfo;
}

inline VkCommandBufferBeginInfo commandBufferBeginInfo()
{
	VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	return cmdBufferBeginInfo;
}

inline VkCommandBufferInheritanceInfo commandBufferInheritanceInfo()
{
	VkCommandBufferInheritanceInfo commandBufferInheritanceInfo = {};
	commandBufferInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	return commandBufferInheritanceInfo;
}

inline VkRenderPassBeginInfo renderPassBeginInfo()
{
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	return renderPassBeginInfo;
}

inline VkRenderPassCreateInfo renderPassCreateInfo()
{
	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	return renderPassCreateInfo;
}

inline VkRenderingAttachmentInfoKHR renderingAttachmentInfo()
{
	VkRenderingAttachmentInfoKHR attachmentInfo = {};
	attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
	attachmentInfo.pNext = VK_NULL_HANDLE;
	return attachmentInfo;
}

inline VkRenderingInfoKHR renderingInfo(
	VkRect2D                            renderArea            = {},
    uint32_t                            colorAttachmentCount = 0,
    const VkRenderingAttachmentInfoKHR *pColorAttachments      = VK_NULL_HANDLE,
    VkRenderingFlagsKHR                 flags                  = 0)
{
	VkRenderingInfoKHR renderingInfo   = {};
	renderingInfo.sType                = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
	renderingInfo.pNext                = VK_NULL_HANDLE;
	renderingInfo.flags                = flags;
	renderingInfo.renderArea           = renderArea;
	renderingInfo.layerCount           = 0;
	renderingInfo.viewMask             = 0;
	renderingInfo.colorAttachmentCount = colorAttachmentCount;
	renderingInfo.pColorAttachments    = pColorAttachments;
	renderingInfo.pDepthAttachment     = VK_NULL_HANDLE;
	renderingInfo.pStencilAttachment   = VK_NULL_HANDLE;
	return renderingInfo;
}

inline VkImageMemoryBarrier imageMemoryBarrier()
{
	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	return imageMemoryBarrier;
}


inline VkBufferMemoryBarrier bufferMemoryBarrier()
{
	VkBufferMemoryBarrier bufferMemoryBarrier = {};
	bufferMemoryBarrier.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	bufferMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	bufferMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	return bufferMemoryBarrier;
}

inline VkMemoryBarrier memoryBarrier()
{
	VkMemoryBarrier memoryBarrier = {};
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	return memoryBarrier;
}

inline VkImageCreateInfo imageCreateInfo()
{
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	return imageCreateInfo;
}

inline VkSamplerCreateInfo samplerCreateInfo()
{
	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType         = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.maxAnisotropy = 1.0f;
	return samplerCreateInfo;
}

inline VkImageViewCreateInfo imageViewCreateInfo()
{
	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	return imageViewCreateInfo;
}

inline VkFramebufferCreateInfo framebufferCreateInfo()
{
	VkFramebufferCreateInfo framebufferCreateInfo = {};
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	return framebufferCreateInfo;
}

inline VkSemaphoreCreateInfo semaphoreCreateInfo()
{
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	return semaphoreCreateInfo;
}

inline VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags = 0)
{
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = flags;
	return fenceCreateInfo;
}

inline VkEventCreateInfo eventCreateInfo()
{
	VkEventCreateInfo eventCreateInfo = {};
	eventCreateInfo.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
	return eventCreateInfo;
}

inline VkSubmitInfo submitInfo()
{
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	return submitInfo;
}

inline VkViewport viewport(
    float width,
    float height,
    float minDepth,
    float maxDepth)
{
	VkViewport viewport = {};
	viewport.width    = width;
	viewport.height   = height;
	viewport.minDepth = minDepth;
	viewport.maxDepth = maxDepth;
	return viewport;
}

inline VkRect2D rect2D(
    int32_t width,
    int32_t height,
    int32_t offsetX,
    int32_t offsetY)
{
	VkRect2D rect2D = {};
	rect2D.extent.width  = width;
	rect2D.extent.height = height;
	rect2D.offset.x      = offsetX;
	rect2D.offset.y      = offsetY;
	return rect2D;
}

inline VkBufferCreateInfo bufferCreateInfo()
{
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	return bufferCreateInfo;
}

inline VkBufferCreateInfo bufferCreateInfo(
    VkBufferUsageFlags usage,
    VkDeviceSize       size)
{
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.usage = usage;
	bufferCreateInfo.size  = size;
	return bufferCreateInfo;
}

inline VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(
    uint32_t              count,
    VkDescriptorPoolSize *poolSizes,
    uint32_t              maxSets)
{
	VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
	descriptorPoolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.poolSizeCount = count;
	descriptorPoolInfo.pPoolSizes    = poolSizes;
	descriptorPoolInfo.maxSets       = maxSets;
	return descriptorPoolInfo;
}

inline VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(
    const std::vector<VkDescriptorPoolSize>& poolSizes,
    uint32_t                                 maxSets)
{
	VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
	descriptorPoolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolInfo.pPoolSizes    = poolSizes.data();
	descriptorPoolInfo.maxSets       = maxSets;
	return descriptorPoolInfo;
}

inline VkDescriptorPoolSize descriptorPoolSize(
    VkDescriptorType type,
    uint32_t         count)
{
	VkDescriptorPoolSize descriptorPoolSize = {};
	descriptorPoolSize.type            = type;
	descriptorPoolSize.descriptorCount = count;
	return descriptorPoolSize;
}

inline VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(
    VkDescriptorType   type,
    VkShaderStageFlags flags,
    uint32_t           binding,
    uint32_t           count = 1)
{
	VkDescriptorSetLayoutBinding setLayoutBinding = {};
	setLayoutBinding.descriptorType  = type;
	setLayoutBinding.stageFlags      = flags;
	setLayoutBinding.binding         = binding;
	setLayoutBinding.descriptorCount = count;
	return setLayoutBinding;
}

inline VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
    const VkDescriptorSetLayoutBinding *bindings,
    uint32_t                            bindingCount)
{
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
	descriptorSetLayoutCreateInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.pBindings    = bindings;
	descriptorSetLayoutCreateInfo.bindingCount = bindingCount;
	return descriptorSetLayoutCreateInfo;
}

inline VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
    const std::vector<VkDescriptorSetLayoutBinding> &bindings)
{
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
	descriptorSetLayoutCreateInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.pBindings    = bindings.data();
	descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	return descriptorSetLayoutCreateInfo;
}

inline VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(
    const VkDescriptorSetLayout *setLayouts,
    uint32_t                     setLayoutCount = 1)
{
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = setLayoutCount;
	pipelineLayoutCreateInfo.pSetLayouts    = setLayouts;
	return pipelineLayoutCreateInfo;
}

inline VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(
    uint32_t setLayoutCount = 1)
{
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = setLayoutCount;
	return pipelineLayoutCreateInfo;
}

inline VkDescriptorSetAllocateInfo descriptorSetAllocateInfo(
    VkDescriptorPool             descriptorPool,
    const VkDescriptorSetLayout *setLayouts,
    uint32_t                     descriptorSetCount)
{
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
	descriptorSetAllocateInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool     = descriptorPool;
	descriptorSetAllocateInfo.pSetLayouts        = setLayouts;
	descriptorSetAllocateInfo.descriptorSetCount = descriptorSetCount;
	return descriptorSetAllocateInfo;
}

inline VkDescriptorImageInfo descriptorImageInfo(VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout)
{
	VkDescriptorImageInfo descriptorImageInfo = {};
	descriptorImageInfo.sampler     = sampler;
	descriptorImageInfo.imageView   = imageView;
	descriptorImageInfo.imageLayout = imageLayout;
	return descriptorImageInfo;
}

inline VkWriteDescriptorSet writeDescriptorSet(
    VkDescriptorSet         dstSet,
    VkDescriptorType        type,
    uint32_t                binding,
    VkDescriptorBufferInfo *bufferInfo,
    uint32_t                descriptorCount = 1)
{
	VkWriteDescriptorSet writeDescriptorSet = {};
	writeDescriptorSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.dstSet          = dstSet;
	writeDescriptorSet.descriptorType  = type;
	writeDescriptorSet.dstBinding      = binding;
	writeDescriptorSet.pBufferInfo     = bufferInfo;
	writeDescriptorSet.descriptorCount = descriptorCount;
	return writeDescriptorSet;
}

inline VkWriteDescriptorSet writeDescriptorSet(
    VkDescriptorSet        dstSet,
    VkDescriptorType       type,
    uint32_t               binding,
    VkDescriptorImageInfo *imageInfo,
    uint32_t               descriptorCount = 1)
{
	VkWriteDescriptorSet writeDescriptorSet = {};
	writeDescriptorSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.dstSet          = dstSet;
	writeDescriptorSet.descriptorType  = type;
	writeDescriptorSet.dstBinding      = binding;
	writeDescriptorSet.pImageInfo      = imageInfo;
	writeDescriptorSet.descriptorCount = descriptorCount;
	return writeDescriptorSet;
}

inline VkVertexInputBindingDescription vertexInputBindingDescription(
    uint32_t          binding,
    uint32_t          stride,
    VkVertexInputRate inputRate)
{
	VkVertexInputBindingDescription vertexInputBindingDescription = {};
	vertexInputBindingDescription.binding   = binding;
	vertexInputBindingDescription.stride    = stride;
	vertexInputBindingDescription.inputRate = inputRate;
	return vertexInputBindingDescription;
}

inline VkVertexInputBindingDescription2EXT vertex_input_binding_description2ext(
    uint32_t          binding,
    uint32_t          stride,
    VkVertexInputRate inputRate,
    uint32_t          divisor)
{
	VkVertexInputBindingDescription2EXT vertex_input_binding_description2ext = {};
	vertex_input_binding_description2ext.sType     = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT;
	vertex_input_binding_description2ext.binding   = binding;
	vertex_input_binding_description2ext.stride    = stride;
	vertex_input_binding_description2ext.inputRate = inputRate;
	vertex_input_binding_description2ext.divisor   = divisor;
	return vertex_input_binding_description2ext;
}

inline VkVertexInputAttributeDescription vertexInputAttributeDescription(
    uint32_t binding,
    uint32_t location,
    VkFormat format,
    uint32_t offset)
{
	VkVertexInputAttributeDescription vertexInputAttributeDescription = {};
	vertexInputAttributeDescription.location = location;
	vertexInputAttributeDescription.binding  = binding;
	vertexInputAttributeDescription.format   = format;
	vertexInputAttributeDescription.offset   = offset;
	return vertexInputAttributeDescription;
}

inline VkVertexInputAttributeDescription2EXT vertex_input_attribute_description2ext(
    uint32_t binding,
    uint32_t location,
    VkFormat format,
    uint32_t offset)
{
	VkVertexInputAttributeDescription2EXT vertex_input_attribute_description2ext = {};
	vertex_input_attribute_description2ext.sType    = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
	vertex_input_attribute_description2ext.location = location;
	vertex_input_attribute_description2ext.binding  = binding;
	vertex_input_attribute_description2ext.format   = format;
	vertex_input_attribute_description2ext.offset   = offset;
	return vertex_input_attribute_description2ext;
}

inline VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo()
{
	VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = {};
	pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	return pipelineVertexInputStateCreateInfo;
}

inline VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(
    VkPrimitiveTopology                     topology,
    VkPipelineInputAssemblyStateCreateFlags flags,
    VkBool32                                primitiveRestartEnable)
{
	VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = {};
	pipelineInputAssemblyStateCreateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineInputAssemblyStateCreateInfo.topology               = topology;
	pipelineInputAssemblyStateCreateInfo.flags                  = flags;
	pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = primitiveRestartEnable;
	return pipelineInputAssemblyStateCreateInfo;
}

inline VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
    VkPolygonMode                           polygonMode,
    VkCullModeFlags                         cullMode,
    VkFrontFace                             frontFace,
    VkPipelineRasterizationStateCreateFlags flags = 0)
{
	VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo = {};
	pipelineRasterizationStateCreateInfo.sType            = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	pipelineRasterizationStateCreateInfo.polygonMode      = polygonMode;
	pipelineRasterizationStateCreateInfo.cullMode         = cullMode;
	pipelineRasterizationStateCreateInfo.frontFace        = frontFace;
	pipelineRasterizationStateCreateInfo.flags            = flags;
	pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfo.lineWidth        = 1.0f;
	return pipelineRasterizationStateCreateInfo;
}

inline VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(
    VkColorComponentFlags colorWriteMask,
    VkBool32              blendEnable)
{
	VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState = {};
	pipelineColorBlendAttachmentState.colorWriteMask = colorWriteMask;
	pipelineColorBlendAttachmentState.blendEnable    = blendEnable;
	return pipelineColorBlendAttachmentState;
}

inline VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
    uint32_t                                   attachmentCount,
    const VkPipelineColorBlendAttachmentState *attachments)
{
	VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo = {};
	pipelineColorBlendStateCreateInfo.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	pipelineColorBlendStateCreateInfo.attachmentCount = attachmentCount;
	pipelineColorBlendStateCreateInfo.pAttachments    = attachments;
	return pipelineColorBlendStateCreateInfo;
}

inline VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(
    VkBool32    depthTestEnable,
    VkBool32    depthWriteEnable,
    VkCompareOp depthCompareOp)
{
	VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo = {};
	pipelineDepthStencilStateCreateInfo.sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	pipelineDepthStencilStateCreateInfo.depthTestEnable  = depthTestEnable;
	pipelineDepthStencilStateCreateInfo.depthWriteEnable = depthWriteEnable;
	pipelineDepthStencilStateCreateInfo.depthCompareOp   = depthCompareOp;
	pipelineDepthStencilStateCreateInfo.front            = pipelineDepthStencilStateCreateInfo.back;
	pipelineDepthStencilStateCreateInfo.back.compareOp   = VK_COMPARE_OP_ALWAYS;
	return pipelineDepthStencilStateCreateInfo;
}

inline VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(
    uint32_t                           viewportCount,
    uint32_t                           scissorCount,
    VkPipelineViewportStateCreateFlags flags = 0)
{
	VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo = {};
	pipelineViewportStateCreateInfo.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pipelineViewportStateCreateInfo.viewportCount = viewportCount;
	pipelineViewportStateCreateInfo.scissorCount  = scissorCount;
	pipelineViewportStateCreateInfo.flags         = flags;
	return pipelineViewportStateCreateInfo;
}

inline VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(
    VkSampleCountFlagBits                 rasterizationSamples,
    VkPipelineMultisampleStateCreateFlags flags = 0)
{
	VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo = {};
	pipelineMultisampleStateCreateInfo.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pipelineMultisampleStateCreateInfo.rasterizationSamples = rasterizationSamples;
	pipelineMultisampleStateCreateInfo.flags                = flags;
	return pipelineMultisampleStateCreateInfo;
}

inline VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(
    const VkDynamicState *            dynamicStates,
    uint32_t                          dynamicStateCount,
    VkPipelineDynamicStateCreateFlags flags = 0)
{
	VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo = {};
	pipelineDynamicStateCreateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	pipelineDynamicStateCreateInfo.pDynamicStates    = dynamicStates;
	pipelineDynamicStateCreateInfo.dynamicStateCount = dynamicStateCount;
	pipelineDynamicStateCreateInfo.flags             = flags;
	return pipelineDynamicStateCreateInfo;
}

inline VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(
    const std::vector<VkDynamicState> &dynamicStates,
    VkPipelineDynamicStateCreateFlags  flags = 0)
{
	VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo = {};
	pipelineDynamicStateCreateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	pipelineDynamicStateCreateInfo.pDynamicStates    = dynamicStates.data();
	pipelineDynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	pipelineDynamicStateCreateInfo.flags             = flags;
	return pipelineDynamicStateCreateInfo;
}

inline VkPipelineTessellationStateCreateInfo pipelineTessellationStateCreateInfo(uint32_t patchControlPoints)
{
	VkPipelineTessellationStateCreateInfo pipelineTessellationStateCreateInfo = {};
	pipelineTessellationStateCreateInfo.sType              = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
	pipelineTessellationStateCreateInfo.patchControlPoints = patchControlPoints;
	return pipelineTessellationStateCreateInfo;
}

inline VkGraphicsPipelineCreateInfo pipelineCreateInfo(
    VkPipelineLayout      layout,
    VkRenderPass          renderPass,
    VkPipelineCreateFlags flags = 0)
{
	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType              = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.layout             = layout;
	pipelineCreateInfo.renderPass         = renderPass;
	pipelineCreateInfo.flags              = flags;
	pipelineCreateInfo.basePipelineIndex  = -1;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	return pipelineCreateInfo;
}

inline VkGraphicsPipelineCreateInfo pipelineCreateInfo()
{
	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType              = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.basePipelineIndex  = -1;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	return pipelineCreateInfo;
}

inline VkComputePipelineCreateInfo computePipelineCreateInfo(
    VkPipelineLayout      layout,
    VkPipelineCreateFlags flags = 0)
{
	VkComputePipelineCreateInfo computePipelineCreateInfo = {};
	computePipelineCreateInfo.sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.layout = layout;
	computePipelineCreateInfo.flags  = flags;
	return computePipelineCreateInfo;
}

inline VkPushConstantRange pushConstantRange(
    VkShaderStageFlags stageFlags,
    uint32_t           size,
    uint32_t           offset)
{
	VkPushConstantRange pushConstantRange = {};
	pushConstantRange.stageFlags = stageFlags;
	pushConstantRange.offset     = offset;
	pushConstantRange.size       = size;
	return pushConstantRange;
}

inline VkBindSparseInfo bindSparseInfo()
{
	VkBindSparseInfo bindSparseInfo = {};
	bindSparseInfo.sType = VK_STRUCTURE_TYPE_BIND_SPARSE_INFO;
	return bindSparseInfo;
}


inline VkSpecializationMapEntry specializationMapEntry(uint32_t constantId, uint32_t offset, size_t size)
{
	VkSpecializationMapEntry specializationMapEntry = {};
	specializationMapEntry.constantID = constantId;
	specializationMapEntry.offset     = offset;
	specializationMapEntry.size       = size;
	return specializationMapEntry;
}


inline VkSpecializationInfo specializationInfo(uint32_t mapEntryCount, const VkSpecializationMapEntry *mapEntries, size_t dataSize, const void *data)
{
	VkSpecializationInfo specializationInfo = {};
	specializationInfo.mapEntryCount = mapEntryCount;
	specializationInfo.pMapEntries   = mapEntries;
	specializationInfo.dataSize      = dataSize;
	specializationInfo.pData         = data;
	return specializationInfo;
}

NAMESPACE_END(vri)


