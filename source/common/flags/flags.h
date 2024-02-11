#pragma once

#include <type_traits>
#include <initializer_list>


template <typename EnumType, typename IntType = uint64_t>
class Flags
{
public:
    // using EnumType = EFlags;
    using EIntType = std::underlying_type_t<EnumType>;

    constexpr Flags() :flags(0ull) { }
    constexpr Flags(EnumType flag) : flags(1 << (static_cast<EIntType>(flag) - 1)) { }
    constexpr Flags(IntType val) : flags(val) {}

    template<typename... Args, typename = std::enable_if_t<(std::is_same_v<Args, EnumType> && ...)>>
    constexpr Flags(Args... args) : flags(ORFlags({ args... })) {}

    void setFlag(EnumType flag) { flags |= 1 << (static_cast<EIntType>(flag) - 1); }

    constexpr        operator IntType ()      const { return flags; } // allow casting to IntType
    // constexpr        operator   EnumType ()     const { return static_cast<EnumType>(flags); } // probably shoudn't have this?
    constexpr Flags& operator=  (const Flags& other) { this->flags = other.flags; return *this; }

    constexpr Flags  operator|  (EnumType rhs)  const { return Flags(flags | (1 << (static_cast<EIntType>(rhs) - 1))); }
    constexpr Flags  operator|  (Flags rhs)     const { return Flags(flags | rhs); }
    constexpr Flags  operator&  (EnumType rhs)  const { return Flags(flags & (1 << (static_cast<EIntType>(rhs) - 1))); }
    constexpr Flags  operator&  (Flags rhs)     const { return Flags(flags & rhs); }
    constexpr Flags  operator^  (EnumType rhs)  const { return Flags(flags ^ (1 << (static_cast<EIntType>(rhs) - 1))); }
    constexpr Flags  operator^  (Flags rhs)     const { return Flags(flags ^ rhs); }
    constexpr Flags  operator~  ()              const { return Flags(~flags); }
    constexpr Flags  operator<< (int shift)     const { return Flags(flags << shift); }
    constexpr Flags  operator>> (int shift)     const { return Flags(flags >> shift); }

    constexpr Flags& operator|= (EnumType rhs) { flags |= (1 << (static_cast<EIntType>(rhs) - 1)); return *this; }
    constexpr Flags& operator|= (Flags rhs) { flags |= rhs; return *this; }
    constexpr Flags& operator&= (EnumType rhs) { flags = flags & (1 << (static_cast<EIntType>(rhs) - 1)); return *this; }
    constexpr Flags& operator&= (Flags rhs) { flags &= rhs; return *this; }
    constexpr Flags& operator^= (EnumType rhs) { flags = flags ^ (1 << (static_cast<EIntType>(rhs) - 1)); return *this; }
    constexpr Flags& operator^= (Flags rhs) { flags ^= rhs; return *this; }

    constexpr Flags& operator<<=(int shift) { flags = flags << shift;  return *this; }
    constexpr Flags& operator>>=(int shift) { flags = flags >> shift;  return *this; }

    constexpr bool   operator== (EnumType rhs)  const { return flags == (1 << (static_cast<EIntType>(rhs) - 1)); }
    constexpr bool   operator!= (EnumType rhs)  const { return flags != (1 << (static_cast<EIntType>(rhs) - 1)); }
    constexpr bool   operator<  (EnumType rhs)  const { return flags <  (1 << (static_cast<EIntType>(rhs) - 1)); }
    constexpr bool   operator<= (EnumType rhs)  const { return flags <= (1 << (static_cast<EIntType>(rhs) - 1)); }
    constexpr bool   operator>  (EnumType rhs)  const { return flags >  (1 << (static_cast<EIntType>(rhs) - 1)); }
    constexpr bool   operator>= (EnumType rhs)  const { return flags >= (1 << (static_cast<EIntType>(rhs) - 1)); }
    constexpr bool   operator&& (EnumType rhs)  const { return flags && (1 << (static_cast<EIntType>(rhs) - 1)); }
    constexpr bool   operator|| (EnumType rhs)  const { return flags || (1 << (static_cast<EIntType>(rhs) - 1)); }


private:
    IntType ORFlags(std::initializer_list<EnumType> list)
    {
        IntType result = 0u;
        for (auto flag : list) {
            result = result | 1 << (static_cast<EIntType>(flag) - 1);
        }
        return result;
    }

    IntType flags;
};


// usage example
/*
#include <iostream>
#include <iomanip>
#include <bitset>
#include <cassert>

enum class EMyFlags : uint8_t
{
    None,
    Flag1,
    Flag2,
    Flag3,
    Flag4,
    Flag5,
    Flag6,
    Flag7,
    Flag8,
    Flag9,
    Flag10,
    Flag11,
    Flag12,
    Flag13,
    Flag14,
    Flag15,
    Flag16,
    Flag17,
    Flag18,
    Flag19,

    Flag_End = 255
};
using MyFlags = Flags<EMyFlags>;

int main()
{
    int w = 10;
    const int b = 16;

    MyFlags flags{};
    std::cout << std::setw(w) << "flags:" << std::setw(2 + b) << std::bitset<b>(flags) << std::endl;

    flags.setFlag(EMyFlags::Flag1);
    std::cout << std::setw(w) << "flags:" << std::setw(2 + b) << std::bitset<b>(flags) << std::endl;

    flags = flags | EMyFlags::Flag3;
    std::cout << std::setw(w) << "flags:" << std::setw(2 + b) << std::bitset<b>(flags) << std::endl;

    flags |= EMyFlags::Flag5;
    std::cout << std::setw(w) << "flags:" << std::setw(2 + b) << std::bitset<b>(flags) << std::endl;

    flags = flags ^ EMyFlags::Flag1;
    std::cout << std::setw(w) << "flags:" << std::setw(2 + b) << std::bitset<b>(flags) << std::endl;

    flags ^= EMyFlags::Flag3;
    std::cout << std::setw(w) << "flags:" << std::setw(2 + b) << std::bitset<b>(flags) << std::endl;

    MyFlags flags1 = MyFlags(EMyFlags::Flag9);
    std::cout << std::setw(w) << "flags1:" << std::setw(2 + b) << std::bitset<b>(flags1) << std::endl;
    assert(flags1 == EMyFlags::Flag9);


    MyFlags flags2{ EMyFlags::Flag2, EMyFlags::Flag4, EMyFlags::Flag6 };
    std::cout << std::setw(w) << "flags2:" << std::setw(2 + b) << std::bitset<b>(flags2) << std::endl;
    assert(flags2 >= EMyFlags::Flag6);

    flags2 |= flags | flags1;
    std::cout << std::setw(w) << "flags2:" << std::setw(2 + b) << std::bitset<b>(flags2) << std::endl;

    return 0;
}

*/