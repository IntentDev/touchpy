#include "cudamemory.h"

VkFormat vkFormatFromCUDAMemoryDesc(CUDAMemoryDesc desc)
{
	switch (desc.shape[0])
	{
	case 4:
	case 3:
		switch (desc.dataType)
		{
		case CUDADataType::UInt8:   return VK_FORMAT_B8G8R8A8_UNORM;
		case CUDADataType::Float32: return VK_FORMAT_R32G32B32A32_SFLOAT;
		case CUDADataType::Float16: return VK_FORMAT_R16G16B16A16_SFLOAT;
		}
		break;
	case 2:
		switch (desc.dataType)
		{
		case CUDADataType::UInt8:   return VK_FORMAT_R8G8_UNORM;
		case CUDADataType::Float32: return VK_FORMAT_R32G32_SFLOAT;
		case CUDADataType::Float16: return VK_FORMAT_R16G16_SFLOAT;
		}
		break;
	case 1:
		switch (desc.dataType)
		{
		case CUDADataType::UInt8:   return VK_FORMAT_R8_UNORM;
		case CUDADataType::Float32: return VK_FORMAT_R32_SFLOAT;
		case CUDADataType::Float16: return VK_FORMAT_R16_SFLOAT;
		}
		break;
	}
	return VK_FORMAT_UNDEFINED;
}

CUDADataType cudaDataTypeFromVkFormat(VkFormat format)
{
	switch (format)
	{
	case VK_FORMAT_B8G8R8A8_UNORM:      return CUDADataType::UInt8;
	case VK_FORMAT_R32G32B32A32_SFLOAT: return CUDADataType::Float32;
	case VK_FORMAT_R16G16B16A16_SFLOAT: return CUDADataType::Float16;
	case VK_FORMAT_B8G8R8_UNORM:        return CUDADataType::UInt8;
	case VK_FORMAT_R32G32B32_SFLOAT:    return CUDADataType::Float32;
	case VK_FORMAT_R16G16B16_SFLOAT:    return CUDADataType::Float16;
	case VK_FORMAT_R8G8_UNORM:          return CUDADataType::UInt8;
	case VK_FORMAT_R32G32_SFLOAT:       return CUDADataType::Float32;
	case VK_FORMAT_R16G16_SFLOAT:       return CUDADataType::Float16;
	case VK_FORMAT_R8_UNORM:            return CUDADataType::UInt8;
	case VK_FORMAT_R32_SFLOAT:          return CUDADataType::Float32;
	case VK_FORMAT_R16_SFLOAT:          return CUDADataType::Float16;
	}
	return CUDADataType::Undefined;
}

uint8_t numCompsFromVkFormat(VkFormat format)
{
	switch (format)
	{
	case VK_FORMAT_B8G8R8A8_UNORM:		return 4;
	case VK_FORMAT_R32G32B32A32_SFLOAT: return 4;
	case VK_FORMAT_R16G16B16A16_SFLOAT: return 4;
	case VK_FORMAT_B8G8R8_UNORM:        return 3;
	case VK_FORMAT_R32G32B32_SFLOAT:    return 3;
	case VK_FORMAT_R16G16B16_SFLOAT:    return 3;
	case VK_FORMAT_R8G8_UNORM:          return 2;
	case VK_FORMAT_R32G32_SFLOAT:       return 2;
	case VK_FORMAT_R16G16_SFLOAT:       return 2;
	case VK_FORMAT_R8_UNORM:            return 1;
	case VK_FORMAT_R32_SFLOAT:          return 1;
	case VK_FORMAT_R16_SFLOAT:          return 1;
	}
	return 0;
}

uint8_t numCompsFromCudaFlags(CudaFlags flags)
{
	if		(flags & CudaFlagBits::BGRA || flags & CudaFlagBits::RGBA) return 4;
	else if (flags & CudaFlagBits::BGR || flags & CudaFlagBits::RGB) return 3;
	else if (flags & CudaFlagBits::RG) return 2;
	else if (flags & CudaFlagBits::R) return 1;
	return 0;
}

size_t componentSizeFromVkFormat(VkFormat format)
{
	switch (format)
	{
	case VK_FORMAT_B8G8R8A8_UNORM:      return sizeof(uint8_t);
	case VK_FORMAT_R32G32B32A32_SFLOAT: return sizeof(float);
	case VK_FORMAT_R16G16B16A16_SFLOAT: return sizeof(uint16_t);
	case VK_FORMAT_B8G8R8_UNORM:        return sizeof(uint8_t);
	case VK_FORMAT_R32G32B32_SFLOAT:    return sizeof(float);
	case VK_FORMAT_R16G16B16_SFLOAT:    return sizeof(uint16_t);
	case VK_FORMAT_R8G8_UNORM:          return sizeof(uint8_t);
	case VK_FORMAT_R32G32_SFLOAT:       return sizeof(float);
	case VK_FORMAT_R16G16_SFLOAT:       return sizeof(uint16_t);
	case VK_FORMAT_R8_UNORM:            return sizeof(uint8_t);
	case VK_FORMAT_R32_SFLOAT:          return sizeof(float);
	case VK_FORMAT_R16_SFLOAT:          return sizeof(uint16_t);
	}
	return 0;
}

std::string cudaDataTypeToString(CUDADataType type)
{
	switch (type)
	{
	case CUDADataType::UInt8:   return "uint8";
	case CUDADataType::Float32: return "float32";
	case CUDADataType::Float16: return "float16";
	}
	return "undefined";
}

cudaChannelFormatDesc cudaChannelFormatDescFromVkFormat(VkFormat vkFormat)
{
	switch (vkFormat)
	{
		case VK_FORMAT_B8G8R8A8_UNORM: return cudaCreateChannelDesc<uchar4>();
		case VK_FORMAT_R32G32B32A32_SFLOAT: return cudaCreateChannelDesc<float4>();
		case VK_FORMAT_R32G32_SFLOAT: return cudaCreateChannelDesc<float2>();
		case VK_FORMAT_R32_SFLOAT: return cudaCreateChannelDesc<float>();
		case VK_FORMAT_R16G16B16A16_SFLOAT: return cudaCreateChannelDesc<short4>();
		case VK_FORMAT_R16G16_SFLOAT: return cudaCreateChannelDesc<short2>();
		case VK_FORMAT_R16_SFLOAT: return cudaCreateChannelDesc<int16_t>();
		default : break;
	}

	return cudaCreateChannelDesc<uchar4>();
}
