#pragma once

#include <common/flags.h>

using Type = uint32_t;

enum class CudaFlagBits : Type
{
	RGBA = 1 << 0,
	RGB = 1 << 1,
	RG = 1 << 2,
	R = 1 << 3,
	BGRA = 1 << 4,
	BGR = 1 << 5,
	CHW = 1 << 6,
	BCHW = 1 << 7,
	HWC = 1 << 8,
	Flipped = 1 << 9,
	Interleaved = 1 << 10,
	Planar = 1 << 11,
	Linear = 1 << 12,
	Normalized = 1 << 13,
	UInt8 = 1 << 14,
	Float = 1 << 15,
};

using CudaFlags = Flags<CudaFlagBits>;

ENABLE_BITMASK_OPERATORS(CudaFlagBits)
