#include <gtest/gtest.h>

#include "componentmask.h"

// Notes to use GTest:

// create a file with the same name as the header file you want to test, but with _gtest appended to the end
// include <gtest/gtest.h> and the header file you want to test
// 
// create a test suite with TEST(TestSuiteName, testName) { /* test code here */ }
// all tests in the same file should have the same TestSuiteName
// 
// best practice is to have one test per function if possible (or at least one test per logical unit)
// operators belows is not really best practice and should be split into separate tests for each operator
// but in this case they are so simple that it's not really necessary
// 
// run tests by selecting touchpygtest.exe as the startup project and pressing F5
// existing tests will be run and the results will be displayed in the output window
//
// note when making a new file, cmake has to rebuild the project before the new file will be included in the build
// the fastest way to do this in VS I've found is to have cmake file open and press ctrl-s to save it, 
// then press F5 to build and run the tests


TEST(ComponentMask, operators)
{
	ComponentMask mask1 = ComponentMask::R;
	ComponentMask mask2 = ComponentMask::G;
	ComponentMask mask3 = ComponentMask::B;

	EXPECT_EQ(mask1 & mask2, ComponentMask::None);
	EXPECT_EQ(mask1 | mask2 | mask3, ComponentMask::RGB);
	EXPECT_EQ(mask1 ^ mask1, ComponentMask::None);
	EXPECT_EQ(mask1 ^ mask2, ComponentMask::RG);

	EXPECT_NE(mask1, ~mask1);

	auto tmp = mask1;
	tmp |= mask2;
	EXPECT_EQ(mask1 | mask2, tmp);

	tmp = mask1;
	tmp &= mask2;
	EXPECT_EQ(mask1 & mask2, tmp);

	EXPECT_EQ(mask1 & mask1, mask1);
	EXPECT_EQ(mask1 | mask1, mask1);

	EXPECT_EQ(mask1 & mask2, ComponentMask::None);
	EXPECT_EQ(mask1 | mask2, ComponentMask::RG);

	tmp = ComponentMask::R;
	EXPECT_TRUE(mask1 == tmp);
	EXPECT_TRUE(mask1 != mask2);
	EXPECT_FALSE(mask1 == mask2);


	auto flags = ComponentMask::R | ComponentMask::G | ComponentMask::B;
	EXPECT_TRUE(flags && ComponentMask::R);
	EXPECT_TRUE(flags && ComponentMask::G);
	EXPECT_TRUE(flags && ComponentMask::B);
	EXPECT_FALSE(flags && ComponentMask::A);
}

TEST(ComponentMask, numActiveComponents)
{
	auto flags = ComponentMask::R | ComponentMask::G;

	EXPECT_EQ(componentMask::numActiveComponents(flags), 2);
	EXPECT_EQ(componentMask::numActiveComponents(ComponentMask::R), 1);
	EXPECT_EQ(componentMask::numActiveComponents(ComponentMask::None), 0);
}

TEST(ComponentMask, toString)
{
	EXPECT_EQ(componentMask::toString(ComponentMask::R), "Red");
	EXPECT_EQ(componentMask::toString(ComponentMask::G), "Green");
	EXPECT_EQ(componentMask::toString(ComponentMask::B), "Blue");
	EXPECT_EQ(componentMask::toString(ComponentMask::A), "Alpha");
	EXPECT_EQ(componentMask::toString(ComponentMask::RG), "Red | Green");
	EXPECT_EQ(componentMask::toString(ComponentMask::RGB), "Red | Green | Blue");
	EXPECT_EQ(componentMask::toString(ComponentMask::RGBA), "Red | Green | Blue | Alpha");
	EXPECT_EQ(componentMask::toString(ComponentMask::None), "None");
}