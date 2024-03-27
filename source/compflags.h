#pragma once

#include <common/flags.h>

using Type = uint32_t;

enum class CompFlagBits : Type
{
	InternalTime = 1 << 0,
	ExternalTime = 1 << 1,
	AutoUpdate = 1 << 2,
	SemiAutoUpdate = 1 << 3,
	AsyncUpdate = 1 << 5,
	Realtime = 1 << 6,
	InternalTimeAuto = static_cast<Type>(InternalTime) | static_cast<Type>(AutoUpdate),
	InternalTimeSemiAuto = static_cast<Type>(InternalTime) | static_cast<Type>(SemiAutoUpdate),
	InternalTimeAsync = static_cast<Type>(InternalTime) | static_cast<Type>(AsyncUpdate),
};

using CompFlags = Flags<CompFlagBits>;

ENABLE_BITMASK_OPERATORS(CompFlagBits)


