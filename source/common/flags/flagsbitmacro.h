#pragma once
#include <cstdint>
#include <type_traits>

// Macro to define the underlying operator overload outside the class
// Probably the fastest way to do this, but it's a bit ugly and might be dangerous

#define ENABLE_BITMASK_OPERATORS(x)  \
inline x operator|(x lhs, x rhs) { \
    using T = std::underlying_type_t<x>; \
    return static_cast<x>(static_cast<T>(lhs) | static_cast<T>(rhs)); \
} \
inline x operator&(x lhs, x rhs) { \
	using T = std::underlying_type_t<x>; \
	return static_cast<x>(static_cast<T>(lhs) & static_cast<T>(rhs)); \
} \
inline x operator^(x lhs, x rhs) { \
	using T = std::underlying_type_t<x>; \
	return static_cast<x>(static_cast<T>(lhs) ^ static_cast<T>(rhs)); \
} \
inline x operator~(x lhs) { \
	using T = std::underlying_type_t<x>; \
	return static_cast<x>(~static_cast<T>(lhs)); \
} \
inline x& operator|=(x& lhs, x rhs) { \
	using T = std::underlying_type_t<x>; \
	lhs = static_cast<x>(static_cast<T>(lhs) | static_cast<T>(rhs)); \
	return lhs; \
} \
inline x& operator&=(x& lhs, x rhs) { \
	using T = std::underlying_type_t<x>; \
	lhs = static_cast<x>(static_cast<T>(lhs) & static_cast<T>(rhs)); \
	return lhs; \
} \
inline x& operator^=(x& lhs, x rhs) { \
	using T = std::underlying_type_t<x>; \
	lhs = static_cast<x>(static_cast<T>(lhs) ^ static_cast<T>(rhs)); \
	return lhs; \
} \
inline x operator<<(x lhs, std::underlying_type_t<x> shift) { \
	using T = std::underlying_type_t<x>; \
	return static_cast<x>(static_cast<T>(lhs) << shift); \
} \
inline x& operator<<=(x& lhs, std::underlying_type_t<x> shift) { \
	using T = std::underlying_type_t<x>; \
	lhs = static_cast<x>(static_cast<T>(lhs) << shift); \
	return lhs; \
} \
inline x operator>>(x lhs, std::underlying_type_t<x> shift) { \
	using T = std::underlying_type_t<x>; \
	return static_cast<x>(static_cast<T>(lhs) >> shift); \
} \
inline x& operator>>=(x& lhs, std::underlying_type_t<x> shift) { \
	using T = std::underlying_type_t<x>; \
	lhs = static_cast<x>(static_cast<T>(lhs) >> shift); \
	return lhs; \
} 



// Example enum for flags
enum class MyFlags : uint32_t
{
	None = 0,
	Flag1 = 1 << 0,
	Flag2 = 1 << 1,
	Flag3 = 1 << 2,

	Flag_End = 1u << 31
};

ENABLE_BITMASK_OPERATORS(MyFlags)


// test operator overloads
void test_operators()
{
	MyFlags flags = MyFlags::Flag1 | MyFlags::Flag2;
	flags |= MyFlags::Flag3;
	flags &= ~MyFlags::Flag2;
	flags ^= MyFlags::Flag1;
	flags <<= 1;
	flags >>= 1;
}
