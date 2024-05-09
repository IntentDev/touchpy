#pragma once

#include <vulkan/vulkan.h>
#include <cuda_runtime.h>
#include <cuda_fp16.h>
#include <memory>
#include <array>
#include <string>

#include "cudaflags.h"
#include "cudadatatypes.h"

enum class CUDADataType : uint8_t
{
	UInt8,
	UInt16,
	Float16,
	Float32,

	Undefined = 0xFF
};

struct CUDAMemoryDesc
{
	std::array<uint32_t, 3> shape         { 0, 0, 0 }; // numComponents, height, width
	size_t                  componentSize { 1 };
	CudaFlags				flags         { CudaFlagBits::None };
	CUDADataType            dataType      { CUDADataType::Undefined };
	std::array<uint32_t, 3> strides       { 0, 0, 0 }; // in elements (not bytes)

	bool operator==(const CUDAMemoryDesc& other) const
	{
		return shape == other.shape && 
			componentSize == other.componentSize && 
			flags == other.flags && 
			dataType == other.dataType && 
			strides == other.strides;
	}

	bool operator!=(const CUDAMemoryDesc& other) const
	{
		return !(*this == other);
	}
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
CudaFlags cudaFlagsFromVkFormat(VkFormat format);
size_t componentSizeFromVkFormat(VkFormat format);
std::string cudaDataTypeToString(CUDADataType type);
cudaChannelFormatDesc cudaChannelFormatDescFromVkFormat(VkFormat vkFormat);

void printCudaFlags(CudaFlags flags);
