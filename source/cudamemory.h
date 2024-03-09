#pragma once

#include <vulkan/vulkan.h>
#include <cuda_runtime.h>
#include <memory>

enum class CUDADataType : uint8_t
{
	UInt8,
	Float16,
	Float32,

	Undefined = 0xFF
};

struct CUDAMemoryShape
{
	uint32_t     width         { 0 };
	uint32_t     height        { 0 };
	uint8_t      numComponents { 0 };
	size_t       componentSize { 0 };
	CUDADataType dataType      { CUDADataType::Undefined };
	uint32_t     strides[3]    { 0, 0, 0 };


};

struct CUDAMemory
{

	void*                            ptr   { nullptr };
	size_t                           size  { 0 };
	CUDAMemoryShape					 shape { };
	//std::unique_ptr<CUDAMemoryShape> shape { };


	//CUDAMemory() = default;
	//void setShape(std::unique_ptr<CUDAMemoryShape> newShape) {
	//	shape = std::move(newShape);
	//}
};

VkFormat vkFormatFromCUDAMemoryShape(CUDAMemoryShape shape);
CUDADataType cudaDataTypeFromVkFormat(VkFormat format);
uint8_t numCompsFromVkFormat(VkFormat format);
size_t componentSizeFromVkFormat(VkFormat format);
