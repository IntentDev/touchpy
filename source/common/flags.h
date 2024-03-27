#pragma once

#include <type_traits>

template<typename FlagsBit>
class Flags
{
public:
	using IntType = std::underlying_type_t<FlagsBit>;

	constexpr Flags() : value(0u) {}
	constexpr Flags(FlagsBit val) : value(static_cast<IntType>(val)) {}
	constexpr Flags(IntType val) : value(val) {}

	template<typename... Args, typename = std::enable_if_t<(std::is_same_v<Args, FlagsBit> && ...)>>
	constexpr Flags(Args... args) : value(ORFlags({ args... })) {}

	constexpr IntType getValue() const { return value; }

	//constexpr operator IntType() const { return value; }
	IntType operator()() const { return value; }
	constexpr operator FlagsBit() const { return static_cast<FlagsBit>(value); }


	constexpr Flags operator|(FlagsBit rhs) const { return Flags(value | static_cast<IntType>(rhs)); }
	constexpr Flags& operator|=(FlagsBit rhs) { value = value | static_cast<IntType>(rhs); return *this; }

	constexpr Flags operator&(FlagsBit rhs) const { return Flags(value & static_cast<IntType>(rhs)); }
	constexpr Flags& operator&=(FlagsBit rhs) { value = value & static_cast<IntType>(rhs); return *this; }

	constexpr Flags operator^(FlagsBit rhs) const { return Flags(value ^ static_cast<IntType>(rhs)); }
	constexpr Flags& operator^=(FlagsBit rhs) { value = value ^ static_cast<IntType>(rhs); return *this; }

	constexpr Flags operator~() const { return Flags(~value); }

	constexpr Flags operator<<(int shift) const { return Flags(value << shift); }
	constexpr Flags& operator<<=(int shift) { value = value << shift;  return *this; }

	constexpr Flags operator>>(int shift) const { return Flags(value >> shift); }
	constexpr Flags& operator>>=(int shift) { value = value >> shift;  return *this; }

	constexpr bool operator==(FlagsBit rhs) const { return value == static_cast<IntType>(rhs); }
	constexpr bool operator!=(FlagsBit rhs) const { return value != static_cast<IntType>(rhs); }
	constexpr bool operator<(FlagsBit rhs) const { return value < static_cast<IntType>(rhs); }
	constexpr bool operator<=(FlagsBit rhs) const { return value <= static_cast<IntType>(rhs); }
	constexpr bool operator>(FlagsBit rhs) const { return value > static_cast<IntType>(rhs); }
	constexpr bool operator>=(FlagsBit rhs) const { return value >= static_cast<IntType>(rhs); }
	constexpr bool operator&&(FlagsBit rhs) const { return value && static_cast<IntType>(rhs); }
	constexpr bool operator||(FlagsBit rhs) const { return value || static_cast<IntType>(rhs); }

protected:
	IntType value;

private:
	static IntType ORFlags(std::initializer_list<FlagsBit> list)
	{
		IntType result = 0u;
		for (auto flag : list) {
			result = static_cast<IntType>(result) | static_cast<IntType>(flag);
		}
		return result;
	}
};


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
}\
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