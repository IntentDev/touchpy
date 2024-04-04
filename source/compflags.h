#pragma once

#include <common/flags.h>

using Type = uint32_t;

enum class CompFlagBits : Type
{
	InternalTime = 1 << 0,
	ExternalTime = 1 << 1,
	AutoUpdate = 1 << 2,
	AsyncUpdate = 1 << 3,
	Realtime = 1 << 4,

	InternalTimeAuto = static_cast<Type>(InternalTime) | static_cast<Type>(AutoUpdate),
	InternalTimeAsync = static_cast<Type>(InternalTime) | static_cast<Type>(AsyncUpdate),

	CudaStreamDefault = 1 << 5,
	CudaStreamInternal = 1 << 6,
};

using CompFlags = Flags<CompFlagBits>;

ENABLE_BITMASK_OPERATORS(CompFlagBits)


