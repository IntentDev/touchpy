#include <gtest/gtest.h>
#include "color.h"

TEST(ColorTest, DefaultConstructor)
{
    Color color;
    EXPECT_EQ(color.r, 0.0f);
    EXPECT_EQ(color.g, 0.0f);
    EXPECT_EQ(color.b, 0.0f);
    EXPECT_EQ(color.a, 1.0f);
}

TEST(ColorTest, FloatConstructor)
{
    Color color(0.5f, 0.3f, 0.8f, 0.7f);
    EXPECT_EQ(color.r, 0.5f);
    EXPECT_EQ(color.g, 0.3f);
    EXPECT_EQ(color.b, 0.8f);
    EXPECT_EQ(color.a, 0.7f);
}

TEST(ColorTest, ArrayConstructor)
{
    std::array<float, 4> values = {0.2f, 0.4f, 0.6f, 0.9f};
    Color color(values);
    EXPECT_EQ(color.r, 0.2f);
    EXPECT_EQ(color.g, 0.4f);
    EXPECT_EQ(color.b, 0.6f);
    EXPECT_EQ(color.a, 0.9f);
}

TEST(ColorTest, IntConstructor)
{
    Color color(100, 150, 200, 50);
    EXPECT_EQ(color.r, 0.392156869f);
    EXPECT_EQ(color.g, 0.588235319f);
    EXPECT_EQ(color.b, 0.784313738f);
    EXPECT_EQ(color.a, 0.196078435f);
}

TEST(ColorTest, HexValueConstructor)
{
    Color color(0xFF336699);
    EXPECT_EQ(color.r, 1.0f);
    EXPECT_EQ(color.g, 0.2f);
    EXPECT_EQ(color.b, 0.4f);
    EXPECT_EQ(color.a, 0.6f);
}

TEST(ColorTest, HexStringConstructor)
{
    Color color("#336699FF");
    EXPECT_EQ(color.r, 0.2f);
    EXPECT_EQ(color.g, 0.4f);
    EXPECT_EQ(color.b, 0.6f);
    EXPECT_EQ(color.a, 1.0f);
}


TEST(ColorTest, HSVConstructor)
{
	Color color(Color::HSVTag{}, 0.0f, 0.0f, 1.0f, 1.0f);
    //Color hsvColor(Color::HSVTag{}, color.asHSV());
	EXPECT_EQ(color.r, 1.0f) << "HSV: r " << color.r;
	EXPECT_EQ(color.g, 1.0f) << "HSV: g " << color.g;
	EXPECT_EQ(color.b, 1.0f) << "HSV: b " << color.b;
	EXPECT_EQ(color.a, 1.0f) << "HSV: a " << color.a;

    color.set(Color::HSVTag{}, 120.0f, 1.0f, 1.0f, 0.5f);
    EXPECT_EQ(color.r, 0.0f) << "HSV: r " << color.r;
    EXPECT_EQ(color.g, 1.0f) << "HSV: g " << color.g;
    EXPECT_EQ(color.b, 0.0f) << "HSV: b " << color.b;
    EXPECT_EQ(color.a, 0.5f) << "HSV: a " << color.a;
}

TEST(ColorTest, HSVset)
{
	Color color;
	color.set(Color::HSVTag{}, 240.0f, 1.0f, 1.0f, 0.5f);
	EXPECT_EQ(color.r, 0.0f) << "HSV: r " << color.r;
	EXPECT_EQ(color.g, 0.0f) << "HSV: g " << color.g;
	EXPECT_EQ(color.b, 1.0f) << "HSV: b " << color.b;
	EXPECT_EQ(color.a, 0.5f) << "HSV: a " << color.a;
}

TEST(ColorTest, HSLConstructor)
{
	Color color(Color::HSLTag{}, 0.0f, 1.f, .5f, 0.7f);
	//Color hslColor(Color::HSLTag{}, color.asHSL());
    EXPECT_EQ(color.r, 1.0f) << "HSL: r " << color.r;
	EXPECT_EQ(color.g, 0.0f) << "HSL: g " << color.g;
	EXPECT_EQ(color.b, 0.0f) << "HSL: b " << color.b;
	EXPECT_EQ(color.a, 0.7f) << "HSL: a " << color.a;
}

TEST(ColorTest, HSLset)
{
	Color color;
	color.set(Color::HSLTag{}, 240.0f, 1.0f, 0.5f, 0.7f);
	EXPECT_EQ(color.r, 0.0f) << "HSL: r " << color.r;
	EXPECT_EQ(color.g, 0.0f) << "HSL: g " << color.g;
	EXPECT_EQ(color.b, 1.0f) << "HSL: b " << color.b;
	EXPECT_EQ(color.a, 0.7f) << "HSL: a " << color.a;
}

TEST(ColorTest, SetColor)
{
    Color color;
    color.set(0.5f, 0.3f, 0.8f, 0.7f);
    EXPECT_EQ(color.r, 0.5f);
    EXPECT_EQ(color.g, 0.3f);
    EXPECT_EQ(color.b, 0.8f);
    EXPECT_EQ(color.a, 0.7f);
}


TEST(ColorTest, GetFloatArray)
{
    Color color(0.2f, 0.4f, 0.6f, 0.9f);
    std::array<float, 4> values = color.asFloatArray();
    EXPECT_EQ(values[0], 0.2f);
    EXPECT_EQ(values[1], 0.4f);
    EXPECT_EQ(values[2], 0.6f);
    EXPECT_EQ(values[3], 0.9f);
}

TEST(ColorTest, GetIntArray)
{
    Color color(0.2f, 0.4f, 0.6f, 0.9f);
    std::array<int, 4> values = color.asIntArray();
    EXPECT_EQ(values[0], 51);
    EXPECT_EQ(values[1], 102);
    EXPECT_EQ(values[2], 153);
    EXPECT_EQ(values[3], 229);
}

TEST(ColorTest, GetHexValue)
{
    Color color(0.2f, 0.4f, 0.6f, 0.9f);
    int hexValue = color.asHex();
    EXPECT_EQ(hexValue, 0x336699E5);
}

TEST(ColorTest, GetHexString)
{
    Color color(0.2f, 0.4f, 0.6f, 0.9f);
    std::string hexStr = color.asHexStr();
    EXPECT_EQ(hexStr, "#336699e5");
}

TEST(ColorTest, GetHSV)
{
    Color color(Color::HSVTag{}, 120.f, 0.4f, 0.6f);
	auto hsv = color.asHSV();
    EXPECT_EQ(hsv[0], 120.f);
	EXPECT_EQ(hsv[1], 0.4f);
    EXPECT_EQ(hsv[2], 0.6f);
}


TEST(ColorTest, GetHSL)
{
    Color color(Color::HSLTag{}, 180.f, 0.8f, 0.3f);
    auto hsv = color.asHSL();
    EXPECT_EQ(hsv[0], 180.f);
    EXPECT_EQ(hsv[1], 0.8f);
    EXPECT_EQ(hsv[2], 0.3f);
}
