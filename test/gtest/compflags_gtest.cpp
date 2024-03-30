#include <gtest/gtest.h>

#include "compflags.h"


class Comp
{
public:
	Comp(CompFlags flags = CompFlagBits::InternalTimeAuto) : flags_(flags) {}

	CompFlags flags() const { return flags_(); }
	void setFlags(CompFlags flags) { flags_ = flags; }

private:
	CompFlags flags_;
};

TEST(CompFlagsTest, test1)
{
	CompFlags flags { CompFlagBits::InternalTime, CompFlagBits::ExternalTime };
	//flags |= CompFlagBits::ExternalTime ;
	EXPECT_EQ(flags(), 0b00000011);
}

TEST(CompFlagsTest, test2)
{
	CompFlags flags = CompFlagBits::InternalTime | CompFlagBits::ExternalTime | CompFlagBits::AutoUpdate;
	EXPECT_EQ(flags(), 0b00000111);
}

TEST(CompFlagsTest, test3)
{
	CompFlags a = CompFlagBits::InternalTime | CompFlagBits::AutoUpdate;
	CompFlags b { CompFlagBits::InternalTimeAuto };
	EXPECT_EQ(a(), b());
}

TEST(CompFlagsTest, test4)
{
	Comp comp1;
	EXPECT_EQ(comp1.flags(), CompFlagBits::InternalTimeAuto);

	Comp comp2 { CompFlagBits::InternalTime | CompFlagBits::Realtime };
	EXPECT_EQ(comp2.flags(), CompFlagBits::InternalTime | CompFlagBits::Realtime);
	
}

TEST(CompFlagsTest, testBool)
{
	CompFlags a = CompFlagBits::InternalTime | CompFlagBits::AutoUpdate;

	EXPECT_TRUE(a & CompFlagBits::AutoUpdate);
	EXPECT_FALSE(a & CompFlagBits::ExternalTime);

}