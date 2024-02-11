#pragma once

#include <initializer_list>
#include <set>
#include <type_traits>
#include <cstdio>
#include <cstdint>

// set method

template<typename EnumType>
class FlagsSet
{
private:
    std::set<EnumType> flags;

public:
    // Allow initialization from an initializer list of EnumType
    FlagsSet(std::initializer_list<EnumType> initList) : flags(initList) {}

    void add(EnumType flag) { flags.insert(flag); }

    void remove(EnumType flag) { flags.erase(flag); }

    bool contains(EnumType flag) const { return flags.find(flag) != flags.end(); }

    size_t count() const { return flags.size(); }

    void clear() { flags.clear(); }

    // Intersection: keep only flags that are also set in 'other'
    void intersection(const EnumType& other)
    {
        std::set<EnumType> intersectionSet;
        for (auto flag : flags)
        {
            if (other.contains(flag))
            {
                intersectionSet.insert(flag);
            }
        }
        flags = std::move(intersectionSet);
    }
};


// Example enum for flags
enum class MyFlags : uint32_t
{
    None = 0,
    Flag1 = 1 << 0,
    Flag2 = 1 << 1,
    Flag3 = 1 << 2,

    Flag_End = 1u << 31
};


// need buid a full test
int quickTest()
{
    // flagsSet usage
    FlagsSet<MyFlags> flagSet{ MyFlags::Flag1, MyFlags::Flag2 };
    flagSet.add(MyFlags::Flag3);      // Add Flag3
    bool hasFlag1 = flagSet.contains(MyFlags::Flag1);  // Check if Flag1 is set
    flagSet.remove(MyFlags::Flag2);   // Remove Flag2
    size_t numFlags = flagSet.count(); // Get the number of flags set


    return 0;
}










