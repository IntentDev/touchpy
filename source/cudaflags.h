#pragma once

#include <common/flags.h>
#include <iostream>

using Type = uint32_t;

enum class CudaFlagBits : Type
{
	None = 0,
	RGBA = 1 << 0,
	RGB = 1 << 1,
	RG = 1 << 2,
	R = 1 << 3,
	BGRA = 1 << 4,
	BGR = 1 << 5,
	CHW = 1 << 6,
	HWC = 1 << 7,
	VEC4 = 1 << 8,
	VEC3 = 1 << 9,
	VEC2 = 1 << 10,
	//Interleaved = 1 << 8, 
	//Planar = 1 << 9, // currently always planar
	//Flipped = 1 << 10, // currently always vertically flipped
};

using CudaFlags = Flags<CudaFlagBits>;

ENABLE_BITMASK_OPERATORS(CudaFlagBits)
