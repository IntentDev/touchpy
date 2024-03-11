#pragma once

#include <vulkan/vulkan.h>
#include <cuda_runtime.h>
#include <memory>
#include <array>
#include <string>
#include "componentmask.h"

enum class CUDADataType : uint8_t
{
	UInt8,
	Float16,
	Float32,

	Undefined = 0xFF
};

struct CUDAMemoryShape
{
	uint32_t                width         { 0 };
	uint32_t                height        { 0 };
	uint8_t                 numComponents { 4 };
	size_t                  componentSize { 1 };
	ComponentMask			componentMask { ComponentMask::RGBA };
	CUDADataType            dataType      { CUDADataType::Undefined };
	std::array<uint32_t, 3> strides       { 0, 0, 0 };
};

struct CUDAMemory
{
	void*                            ptr   { nullptr };
	size_t                           size  { 0 };
	CUDAMemoryShape					 shape { };
};


VkFormat vkFormatFromCUDAMemoryShape(CUDAMemoryShape shape);
CUDADataType cudaDataTypeFromVkFormat(VkFormat format);
uint8_t numCompsFromVkFormat(VkFormat format);
size_t componentSizeFromVkFormat(VkFormat format);
std::string cudaDataTypeToString(CUDADataType type);
cudaChannelFormatDesc cudaChannelFormatDescFromVkFormat(VkFormat vkFormat);
