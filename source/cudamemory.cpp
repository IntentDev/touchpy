#include "cudamemory.h"

VkFormat vkFormatFromCUDAMemoryDesc(CUDAMemoryDesc desc)
{
	uint32_t numComps = desc.shape[0];
	if (desc.flags & CudaFlagBits::HWC) numComps = desc.shape[2];

	switch (numComps)
	{
	case 4:
	case 3:
		switch (desc.dataType)
		{
		case CUDADataType::UInt8:   return VK_FORMAT_B8G8R8A8_UNORM;
		case CUDADataType::UInt16:  return VK_FORMAT_R16G16B16A16_UNORM;
		case CUDADataType::Float16: return VK_FORMAT_R16G16B16A16_SFLOAT;
		case CUDADataType::Float32: return VK_FORMAT_R32G32B32A32_SFLOAT;
		}
		break;
	case 2:
		switch (desc.dataType)
		{
		case CUDADataType::UInt8:   return VK_FORMAT_R8G8_UNORM;
		case CUDADataType::UInt16:  return VK_FORMAT_R16G16_UNORM;
		case CUDADataType::Float16: return VK_FORMAT_R16G16_SFLOAT;
		case CUDADataType::Float32: return VK_FORMAT_R32G32_SFLOAT;
		}
		break;
	case 1:
		switch (desc.dataType)
		{
		case CUDADataType::UInt8:   return VK_FORMAT_R8_UNORM;
		case CUDADataType::UInt16:  return VK_FORMAT_R16_UNORM;
		case CUDADataType::Float16: return VK_FORMAT_R16_SFLOAT;
		case CUDADataType::Float32: return VK_FORMAT_R32_SFLOAT;
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
	case VK_FORMAT_R16G16B16A16_UNORM:  return CUDADataType::UInt16;
	case VK_FORMAT_R16G16B16A16_SFLOAT: return CUDADataType::Float16;
	case VK_FORMAT_R32G32B32A32_SFLOAT: return CUDADataType::Float32;
	case VK_FORMAT_R8G8_UNORM:          return CUDADataType::UInt8;
	case VK_FORMAT_R16G16_UNORM:		return CUDADataType::UInt16;
	case VK_FORMAT_R16G16_SFLOAT:		return CUDADataType::Float16;
	case VK_FORMAT_R32G32_SFLOAT:       return CUDADataType::Float32;
	case VK_FORMAT_R8_UNORM:            return CUDADataType::UInt8;
	case VK_FORMAT_R16_UNORM:			return CUDADataType::UInt16;
	case VK_FORMAT_R16_SFLOAT:			return CUDADataType::Float16;
	case VK_FORMAT_R32_SFLOAT:          return CUDADataType::Float32;
	}
	return CUDADataType::Undefined;
}

uint8_t numCompsFromVkFormat(VkFormat format)
{
	switch (format)
	{
	case VK_FORMAT_B8G8R8A8_UNORM:		return 4;
	case VK_FORMAT_R16G16B16A16_UNORM:  return 4;
	case VK_FORMAT_R16G16B16A16_SFLOAT: return 4;
	case VK_FORMAT_R32G32B32A32_SFLOAT: return 4;
	case VK_FORMAT_R8G8_UNORM:          return 2;
	case VK_FORMAT_R16G16_UNORM:		return 2;
	case VK_FORMAT_R16G16_SFLOAT:		return 2;
	case VK_FORMAT_R32G32_SFLOAT:       return 2;
	case VK_FORMAT_R8_UNORM:            return 1;
	case VK_FORMAT_R16_UNORM:			return 1;
	case VK_FORMAT_R16_SFLOAT:			return 1;
	case VK_FORMAT_R32_SFLOAT:          return 1;
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

CudaFlags cudaFlagsFromVkFormat(VkFormat format)
{
	switch (format)
	{
		case VK_FORMAT_B8G8R8A8_UNORM:		return CudaFlagBits::RGBA;
		case VK_FORMAT_R16G16B16A16_UNORM:	return CudaFlagBits::RGBA;
		case VK_FORMAT_R16G16B16A16_SFLOAT: return CudaFlagBits::RGBA;
		case VK_FORMAT_R32G32B32A32_SFLOAT: return CudaFlagBits::RGBA;
		case VK_FORMAT_R8G8_UNORM:			return CudaFlagBits::RG;
		case VK_FORMAT_R16G16_UNORM:		return CudaFlagBits::RG;
		case VK_FORMAT_R16G16_SFLOAT:		return CudaFlagBits::RG;
		case VK_FORMAT_R32G32_SFLOAT:		return CudaFlagBits::RG;
		case VK_FORMAT_R8_UNORM:			return CudaFlagBits::R;
		case VK_FORMAT_R16_UNORM:			return CudaFlagBits::R;
		case VK_FORMAT_R16_SFLOAT:			return CudaFlagBits::R;
		case VK_FORMAT_R32_SFLOAT:			return CudaFlagBits::R;
		default: break;
	}
	return CudaFlags();
}


size_t componentSizeFromVkFormat(VkFormat format)
{
	switch (format)
	{
	case VK_FORMAT_B8G8R8A8_UNORM:      return sizeof(uint8_t);
	case VK_FORMAT_R16G16B16A16_UNORM:  return sizeof(uint16_t);
	case VK_FORMAT_R16G16B16A16_SFLOAT: return sizeof(half);
	case VK_FORMAT_R32G32B32A32_SFLOAT: return sizeof(float);
	case VK_FORMAT_R8G8_UNORM:          return sizeof(uint8_t);
	case VK_FORMAT_R16G16_UNORM:        return sizeof(uint16_t);
	case VK_FORMAT_R16G16_SFLOAT:       return sizeof(half);
	case VK_FORMAT_R32G32_SFLOAT:       return sizeof(float);
	case VK_FORMAT_R8_UNORM:            return sizeof(uint8_t);
	case VK_FORMAT_R16_UNORM:           return sizeof(uint16_t);
	case VK_FORMAT_R16_SFLOAT:          return sizeof(half);
	case VK_FORMAT_R32_SFLOAT:          return sizeof(float);
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
		case VK_FORMAT_R16G16B16A16_UNORM: return cudaCreateChannelDesc<ushort4>();
		case VK_FORMAT_R16G16B16A16_SFLOAT: return cudaCreateChannelDescHalf4();
		case VK_FORMAT_R32G32B32A32_SFLOAT: return cudaCreateChannelDesc<float4>();

		case VK_FORMAT_R8G8_UNORM: return cudaCreateChannelDesc<uchar2>();
		case VK_FORMAT_R16G16_UNORM: return cudaCreateChannelDesc<ushort2>();
		case VK_FORMAT_R16G16_SFLOAT: return cudaCreateChannelDescHalf2();
		case VK_FORMAT_R32G32_SFLOAT: return cudaCreateChannelDesc<float2>();

		case VK_FORMAT_R8_UNORM: return cudaCreateChannelDesc<uint8_t>();
		case VK_FORMAT_R16_UNORM: return cudaCreateChannelDesc<uint16_t>();
		case VK_FORMAT_R16_SFLOAT: return cudaCreateChannelDescHalf();
		case VK_FORMAT_R32_SFLOAT: return cudaCreateChannelDesc<float>();

		default : break;
	}

	return cudaCreateChannelDesc<uchar4>();
}

void printCudaFlags(CudaFlags flags)
{
	std::cout << "CudaFlags: ";

	if (flags & CudaFlagBits::None)
	{
		std::cout << "None";
		return;
	}

	if (flags & CudaFlagBits::RGBA)
		std::cout << "RGBA ";
	if (flags & CudaFlagBits::RGB)
		std::cout << "RGB ";
	if (flags & CudaFlagBits::RG)
		std::cout << "RG ";
	if (flags & CudaFlagBits::R)
		std::cout << "R ";
	if (flags & CudaFlagBits::BGRA)
		std::cout << "BGRA ";
	if (flags & CudaFlagBits::BGR)
		std::cout << "BGR ";
	if (flags & CudaFlagBits::CHW)
		std::cout << "CHW ";
	if (flags & CudaFlagBits::HWC)
		std::cout << "HWC ";
	//if (flags & CudaFlagBits::Interleaved)
	//	std::cout << "Interleaved ";
	//if (flags & CudaFlagBits::Planar)
	//	std::cout << "Planar ";
	//if (flags & CudaFlagBits::Flipped)
	//	std::cout << "Flipped ";

	std::cout << std::endl;
}
