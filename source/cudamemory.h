#pragma once

#include <vulkan/vulkan.h>
#include <cuda_runtime.h>
#include <memory>
#include <array>
#include <string>
#include "componentmask.h"
#include "cudaflags.h"

enum class CUDADataType : uint8_t
{
	UInt8,
	Float16,
	Float32,

	Undefined = 0xFF
};

struct CUDAMemoryDesc
{
	std::array<uint32_t, 3> shape         { 0, 0, 0 }; // numComponents, height, width
	size_t                  componentSize { 1 };
	ComponentMask			componentMask { ComponentMask::RGBA };
	CUDADataType            dataType      { CUDADataType::Undefined };
	std::array<uint32_t, 3> strides       { 0, 0, 0 }; // in elements (not bytes)
};

struct CUDAMemory
{
	void*                            ptr   { nullptr };
	size_t                           size  { 0 };
	CUDAMemoryDesc					 desc { };
};


VkFormat vkFormatFromCUDAMemoryDesc(CUDAMemoryDesc desc);
CUDADataType cudaDataTypeFromVkFormat(VkFormat format);
uint8_t numCompsFromVkFormat(VkFormat format);
uint8_t numCompsFromCudaFlags(CudaFlags flags);
size_t componentSizeFromVkFormat(VkFormat format);
std::string cudaDataTypeToString(CUDADataType type);
cudaChannelFormatDesc cudaChannelFormatDescFromVkFormat(VkFormat vkFormat);
