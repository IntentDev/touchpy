#include <gtest/gtest.h>
#include "color.h"

#include <iostream>

TEST(ColorTest, DefaultConstructor)
{
    //std::cout << "color sise: " << sizeof(Color) << std::endl;

    Color color;
    EXPECT_EQ(color.r, 0.0);
    EXPECT_EQ(color.g, 0.0);
    EXPECT_EQ(color.b, 0.0);
    EXPECT_EQ(color.a, 1.0);
}

TEST(ColorTest, DoubleConstructor)
{
    Color color(0.5, 0.3, 0.8, 0.7);
    EXPECT_EQ(color.r, 0.5);
    EXPECT_EQ(color.g, 0.3);
    EXPECT_EQ(color.b, 0.8);
    EXPECT_EQ(color.a, 0.7);
}

TEST(ColorTest, ArrayConstructor)
{
    std::array<double, 4> values = {0.2, 0.4, 0.6, 0.9};
    Color color(values);
    EXPECT_EQ(color.r, 0.2);
    EXPECT_EQ(color.g, 0.4);
    EXPECT_EQ(color.b, 0.6);
    EXPECT_EQ(color.a, 0.9);
}

TEST(ColorTest, IntConstructor)
{
    Color color(100, 150, 200, 50);
    EXPECT_DOUBLE_EQ(color.r, 0.3921568627450980392156862745098);
    EXPECT_DOUBLE_EQ(color.g, 0.58823529411764705882352941176471);
    EXPECT_DOUBLE_EQ(color.b, 0.78431372549019607843137254901961);
    EXPECT_DOUBLE_EQ(color.a, 0.1960784313725490196078431372549);
}

TEST(ColorTest, HexValueConstructor)
{
    Color color(0xFF336699);
    EXPECT_DOUBLE_EQ(color.r, 1.0);
    EXPECT_DOUBLE_EQ(color.g, 0.2);
    EXPECT_DOUBLE_EQ(color.b, 0.4);
    EXPECT_DOUBLE_EQ(color.a, 0.6);
}

TEST(ColorTest, HexStringConstructor)
{
    Color color("#336699FF");
    EXPECT_EQ(color.r, 0.2);
    EXPECT_EQ(color.g, 0.4);
    EXPECT_EQ(color.b, 0.6);
    EXPECT_EQ(color.a, 1.0);
}


TEST(ColorTest, HSVConstructor)
{
	Color color(Color::HSVTag{}, 0.0, 0.0, 1.0, 1.0);
    //Color hsvColor(Color::HSVTag{}, color.asHSV());
	EXPECT_DOUBLE_EQ(color.r, 1.0) << "HSV: r " << color.r;
	EXPECT_DOUBLE_EQ(color.g, 1.0) << "HSV: g " << color.g;
	EXPECT_DOUBLE_EQ(color.b, 1.0) << "HSV: b " << color.b;
	EXPECT_DOUBLE_EQ(color.a, 1.0) << "HSV: a " << color.a;

    color.set(Color::HSVTag{}, 120.0, 1.0, 1.0, 0.5);
    EXPECT_DOUBLE_EQ(color.r, 0.0) << "HSV: r " << color.r;
    EXPECT_DOUBLE_EQ(color.g, 1.0) << "HSV: g " << color.g;
    EXPECT_DOUBLE_EQ(color.b, 0.0) << "HSV: b " << color.b;
    EXPECT_DOUBLE_EQ(color.a, 0.5) << "HSV: a " << color.a;
}

TEST(ColorTest, HSVset)
{
	Color color;
	color.set(Color::HSVTag{}, 240.0, 1.0, 1.0, 0.5);
	EXPECT_DOUBLE_EQ(color.r, 0.0) << "HSV: r " << color.r;
	EXPECT_DOUBLE_EQ(color.g, 0.0) << "HSV: g " << color.g;
	EXPECT_DOUBLE_EQ(color.b, 1.0) << "HSV: b " << color.b;
	EXPECT_DOUBLE_EQ(color.a, 0.5) << "HSV: a " << color.a;
}

TEST(ColorTest, HSLConstructor)
{
	Color color(Color::HSLTag{}, 0.0, 1., .5, 0.7);
	//Color hslColor(Color::HSLTag{}, color.asHSL());
    EXPECT_DOUBLE_EQ(color.r, 1.0) << "HSL: r " << color.r;
	EXPECT_DOUBLE_EQ(color.g, 0.0) << "HSL: g " << color.g;
	EXPECT_DOUBLE_EQ(color.b, 0.0) << "HSL: b " << color.b;
	EXPECT_DOUBLE_EQ(color.a, 0.7) << "HSL: a " << color.a;
}

TEST(ColorTest, HSLset)
{
	Color color;
	color.set(Color::HSLTag{}, 240.0, 1.0, 0.5, 0.7);
	EXPECT_DOUBLE_EQ(color.r, 0.0) << "HSL: r " << color.r;
	EXPECT_DOUBLE_EQ(color.g, 0.0) << "HSL: g " << color.g;
	EXPECT_DOUBLE_EQ(color.b, 1.0) << "HSL: b " << color.b;
	EXPECT_DOUBLE_EQ(color.a, 0.7) << "HSL: a " << color.a;
}

TEST(ColorTest, SetColor)
{
    Color color;
    color.set(0.5, 0.3, 0.8, 0.7);
    EXPECT_DOUBLE_EQ(color.r, 0.5);
    EXPECT_DOUBLE_EQ(color.g, 0.3);
    EXPECT_DOUBLE_EQ(color.b, 0.8);
    EXPECT_DOUBLE_EQ(color.a, 0.7);
}


TEST(ColorTest, GetDoubleArray)
{
    Color color(0.2, 0.4, 0.6, 0.9);
    std::array<double, 4> values = color.asDoubleArray();
    EXPECT_DOUBLE_EQ(values[0], 0.2);
    EXPECT_DOUBLE_EQ(values[1], 0.4);
    EXPECT_DOUBLE_EQ(values[2], 0.6);
    EXPECT_DOUBLE_EQ(values[3], 0.9);
}

TEST(ColorTest, asFloatArray)
{
    Color color(0.2, 0.4, 0.6, 0.9);
	std::array<float, 4> values = color.asFloatArray();
	EXPECT_FLOAT_EQ(values[0], 0.2);
	EXPECT_FLOAT_EQ(values[1], 0.4);
	EXPECT_FLOAT_EQ(values[2], 0.6);
	EXPECT_FLOAT_EQ(values[3], 0.9);
}

TEST(ColorTest, GetIntArray)
{
    Color color(0.2, 0.4, 0.6, 0.9);
    std::array<int, 4> values = color.asIntArray();
    EXPECT_EQ(values[0], 51);
    EXPECT_EQ(values[1], 102);
    EXPECT_EQ(values[2], 153);
    EXPECT_EQ(values[3], 229);
}

TEST(ColorTest, GetHexValue)
{
    Color color(0.2, 0.4, 0.6, 0.9);
    int hexValue = color.asHex();
    EXPECT_EQ(hexValue, 0x336699E5);
}

TEST(ColorTest, GetHexString)
{
    Color color(0.2, 0.4, 0.6, 0.9);
    std::string hexStr = color.asHexStr();
    EXPECT_EQ(hexStr, "#336699e5");
}

TEST(ColorTest, GetHSV)
{
    Color color(Color::HSVTag{}, 120., 0.4, 0.6);
	auto hsv = color.asHSV();
    EXPECT_DOUBLE_EQ(hsv[0], 120.);
    EXPECT_DOUBLE_EQ(hsv[1], 0.4);
    EXPECT_DOUBLE_EQ(hsv[2], 0.6);
}


TEST(ColorTest, GetHSL)
{
    Color color(Color::HSLTag{}, 180., 0.8, 0.3);
    auto hsv = color.asHSL();
    EXPECT_DOUBLE_EQ(hsv[0], 180.);
    EXPECT_DOUBLE_EQ(hsv[1], 0.8);
    EXPECT_DOUBLE_EQ(hsv[2], 0.3);
}
