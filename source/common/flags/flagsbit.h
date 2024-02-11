#pragma once

#include <cstdint>
#include <iomanip>


// Do not use this enum class and class directly, copy and paste the code below to a new file!!!
// 
// to use: Replace all instances of EFlagsBit, FlagsBit and then set specific enumerator names and values

enum class EFlagsBit : uint32_t
{
	None = 0,
	Flag1 = 1 << 0,
	Flag2 = 1 << 1,
	Flag3 = 1 << 2,
	Flag4 = 1 << 3,

	Flag_End = 1u << 31
};

class FlagsBit
{
public:
	using EnumType = EFlagsBit;
	using IntType = std::underlying_type_t<EnumType>;

	constexpr FlagsBit() : value(0u) {}
	constexpr FlagsBit(EnumType val) : value(static_cast<IntType>(val)) {}
	constexpr FlagsBit(IntType val) : value(val) {}

	template<typename... Args, typename = std::enable_if_t<(std::is_same_v<Args, EnumType> && ...)>>
	constexpr FlagsBit(Args... args) : value(ORFlags({ args... })) {}

	constexpr IntType getValue() const { return value; }

	constexpr operator IntType() const { return value; }
	constexpr operator EnumType() const { return static_cast<EnumType>(value); }

	constexpr FlagsBit operator|(EnumType rhs) const { return FlagsBit(value | static_cast<IntType>(rhs)); }
	constexpr FlagsBit& operator|=(EnumType rhs) { value = value | static_cast<IntType>(rhs); return *this; }

	constexpr FlagsBit operator&(EnumType rhs) const { return FlagsBit(value & static_cast<IntType>(rhs)); }
	constexpr FlagsBit& operator&=(EnumType rhs) { value = value & static_cast<IntType>(rhs); return *this; }

	constexpr FlagsBit operator^(EnumType rhs) const { return FlagsBit(value ^ static_cast<IntType>(rhs)); }
	constexpr FlagsBit& operator^=(EnumType rhs) { value = value ^ static_cast<IntType>(rhs); return *this; }

	constexpr FlagsBit operator~() const { return FlagsBit(~value); }

	constexpr FlagsBit operator<<(int shift) const { return FlagsBit(value << shift); }
	constexpr FlagsBit& operator<<=(int shift) { value = value << shift;  return *this; }

	constexpr FlagsBit operator>>(int shift) const { return FlagsBit(value >> shift); }
	constexpr FlagsBit& operator>>=(int shift) { value = value >> shift;  return *this; }

	constexpr bool operator==(EnumType rhs) const { return value == static_cast<IntType>(rhs); }
	constexpr bool operator!=(EnumType rhs) const { return value != static_cast<IntType>(rhs); }
	constexpr bool operator<(EnumType rhs) const { return value < static_cast<IntType>(rhs); }
	constexpr bool operator<=(EnumType rhs) const { return value <= static_cast<IntType>(rhs); }
	constexpr bool operator>(EnumType rhs) const { return value > static_cast<IntType>(rhs); }
	constexpr bool operator>=(EnumType rhs) const { return value >= static_cast<IntType>(rhs); }
	constexpr bool operator&&(EnumType rhs) const { return value && static_cast<IntType>(rhs); }
	constexpr bool operator||(EnumType rhs) const { return value || static_cast<IntType>(rhs); }





protected:
	IntType value;

private:
	static IntType ORFlags(std::initializer_list<EnumType> list)
	{
		IntType result = 0u;
		for (auto flag : list) {
			result = static_cast<IntType>(result) | static_cast<IntType>(flag);
		}
		return result;
	}
};
