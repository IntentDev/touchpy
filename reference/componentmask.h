#pragma once
#include <type_traits>
#include <string>

#include <type_traits>


enum class ComponentMask : uint8_t
{
	None = 0,
	R = 1 << 0,
	G = 1 << 1,
	B = 1 << 2,
	A = 1 << 3,
	RG = R | G,
	RGB = R | G | B,
	RGBA = R | G | B | A
};

constexpr ComponentMask operator|(ComponentMask a, ComponentMask b)
{
	return static_cast<ComponentMask>(
		static_cast<std::underlying_type_t<ComponentMask>>(a) |
		static_cast<std::underlying_type_t<ComponentMask>>(b));
}

constexpr ComponentMask operator&(ComponentMask a, ComponentMask b)
{
	return static_cast<ComponentMask>(
		static_cast<std::underlying_type_t<ComponentMask>>(a) &
		static_cast<std::underlying_type_t<ComponentMask>>(b));
}

constexpr ComponentMask operator^(ComponentMask a, ComponentMask b)
{
	return static_cast<ComponentMask>(
		static_cast<std::underlying_type_t<ComponentMask>>(a) ^
		static_cast<std::underlying_type_t<ComponentMask>>(b));
}

constexpr ComponentMask operator~(ComponentMask a)
{
	return static_cast<ComponentMask>(~static_cast<std::underlying_type_t<ComponentMask>>(a));
}

constexpr ComponentMask& operator|=(ComponentMask& a, ComponentMask b)
{
	a = a | b;
	return a;
}

constexpr ComponentMask& operator&=(ComponentMask& a, ComponentMask b)
{
	a = a & b;
	return a;
}

constexpr ComponentMask& operator^=(ComponentMask& a, ComponentMask b)
{
	a = a ^ b;
	return a;
}

constexpr bool operator!(ComponentMask a)
{
	return !static_cast<std::underlying_type_t<ComponentMask>>(a);
}

constexpr bool operator==(ComponentMask a, ComponentMask b)
{
	return static_cast<std::underlying_type_t<ComponentMask>>(a) == static_cast<std::underlying_type_t<ComponentMask>>(b);
}

constexpr bool operator!=(ComponentMask a, ComponentMask b)
{
	return static_cast<std::underlying_type_t<ComponentMask>>(a) != static_cast<std::underlying_type_t<ComponentMask>>(b);
}

constexpr bool operator&&(ComponentMask a, ComponentMask b)
{
	return static_cast<bool>(a & b);
}

constexpr bool operator||(ComponentMask a, ComponentMask b)
{
	return static_cast<bool>(a | b);
}

namespace componentMask
{
	inline uint8_t numActiveComponents(ComponentMask flags)
	{
		uint8_t result = 0;
		if (flags && ComponentMask::R)
		{
			++result;
		}
		if (flags && ComponentMask::G)
		{
			++result;
		}
		if (flags && ComponentMask::B)
		{
			++result;
		}
		if (flags && ComponentMask::A)
		{
			++result;
		}
		return result;
	}

	inline std::string toString(ComponentMask flags)
	{
		std::string result;
		if (flags == ComponentMask::None)
		{
			result = "None";
		}
		else
		{
			if (flags && ComponentMask::R)
			{
				result += "Red";
			}
			if (flags && ComponentMask::G)
			{
				if (!result.empty())
				{
					result += " | ";
				}
				result += "Green";
			}
			if (flags && ComponentMask::B)
			{
				if (!result.empty())
				{
					result += " | ";
				}
				result += "Blue";
			}
			if (flags && ComponentMask::A)
			{
				if (!result.empty())
				{
					result += " | ";
				}
				result += "Alpha";
			}
		}
		return result;
	}
}

