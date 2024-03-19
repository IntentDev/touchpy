#include <gtest/gtest.h>

#include "componentmask.h"


TEST(ComponentMask, TestOperators)
{
	ComponentMask mask1 = ComponentMask::R;
	ComponentMask mask2 = ComponentMask::G;
	ComponentMask mask3 = ComponentMask::B;

	EXPECT_EQ(mask1 & mask2, ComponentMask::None);
	EXPECT_EQ(mask1 | mask2 | mask3, ComponentMask::RGB);

	EXPECT_EQ(mask1 & mask1, mask1);
	EXPECT_EQ(mask1 | mask1, mask1);
	EXPECT_EQ(mask1 ^ mask1, ComponentMask::None);

	EXPECT_EQ(mask1 & mask2, ComponentMask::None);
	EXPECT_EQ(mask1 | mask2, ComponentMask::RG);
}