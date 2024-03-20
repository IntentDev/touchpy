#pragma once

#include <array>
#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>


struct Color
{
    double r, g, b, a;

    struct HSVTag {};
    struct HSLTag {};

    Color() : r(0.0), g(0.0), b(0.0), a(1.0) {}
    Color(double r, double g, double b, double a = 1.f) : r(r), g(g), b(b), a(a) {}
    Color(const std::array<double, 4>& values) : r(values[0]), g(values[1]), b(values[2]), a(values[3]) {}

    // integer values (0-255)
    Color(int r, int g, int b, int a = 255) : r(r / 255.0), g(g / 255.0), b(b / 255.0), a(a / 255.0) {}

    Color(const int& hexValue)
    {
        r = ((hexValue >> 24) & 0xFF) / 255.0;
        g = ((hexValue >> 16) & 0xFF) / 255.0;
        b = ((hexValue >> 8) & 0xFF) / 255.0;
        a = ((hexValue) & 0xFF) / 255.0;
    }

    // hexadecimal string value in the format #RRGGBBAA or #RRGGBB
    Color(const std::string& hexStr) 
    {
        
        if (isHexStr(hexStr))
        {   
            // strip the # if it exists
            std::string strippedHexStr = hexStr;
            if (strippedHexStr[0] == '#')
                strippedHexStr = strippedHexStr.substr(1, strippedHexStr.length() - 1);

            unsigned int hexValue;
            std::stringstream ss;
            ss << std::hex << strippedHexStr;
            ss >> hexValue;
            r = ((hexValue >> 24) & 0xFF) / 255.0;
            g = ((hexValue >> 16) & 0xFF) / 255.0;
            b = ((hexValue >> 8) & 0xFF) / 255.0;
            a = ((hexValue) & 0xFF) / 255.0;
        }
        else
        {
            r = 0.0;
            g = 0.0;
            b = 0.0;
            a = 1.0;
        }
    }

    // HSV values
    Color(HSVTag, double hue, double sat, double value, double alpha = 1.)
	{
		double chroma = value * sat;
		double huePrime = std::fmod(hue / 60.0, 6.0);
		double x = chroma * (1.0 - std::abs(std::fmod(huePrime, 2.0) - 1.0));
		double m = value - chroma;

		if (huePrime < 1.0)      { r = chroma; g = x; b = 0.0; }
		else if (huePrime < 2.0) { r = x; g = chroma; b = 0.0; }
		else if (huePrime < 3.0) { r = 0.0; g = chroma; b = x; }
		else if (huePrime < 4.0) { r = 0.0; g = x; b = chroma; }
		else if (huePrime < 5.0) { r = x; g = 0.0; b = chroma; }
		else if (huePrime < 6.0) { r = chroma; g = 0.0; b = x; }
		else                      { r = 0.0; g = 0.0;b = 0.0; }

		r += m;
		g += m;
		b += m;
		a = alpha;
	}

    Color(HSVTag, const std::array<double, 3>& values, double alpha = 1.) : Color(HSVTag{}, values[0], values[1], values[2], alpha) {}

	// HSL values
	Color(HSLTag, double hue, double sat, double lightness, double alpha = 1.)
    {
        double chroma = (1.0 - std::abs(2.0 * lightness - 1.0)) * sat;
        double huePrime = std::fmod(hue / 60.0, 6.0);
        double x = chroma * (1.0 - std::abs(std::fmod(huePrime, 2.0) - 1.0));
        double m = lightness - chroma / 2.0;

        if (huePrime < 1.0)      { r = chroma;  g = x; b = 0.0; }
        else if (huePrime < 2.0) { r = x;  g = chroma; b = 0.0; }
        else if (huePrime < 3.0) { r = 0.0; g = chroma; b = x; }
        else if (huePrime < 4.0) { r = 0.0; g = x; b = chroma; }
        else if (huePrime < 5.0) { r = x; g = 0.0; b = chroma; }
        else if (huePrime < 6.0) { r = chroma; g = 0.0; b = x; }
        else                      { r = 0.0; g = 0.0; b = 0.0; }

        r += m;
        g += m;
        b += m;
        a = alpha;
    }

    Color(HSLTag, const std::array<double, 3>& values, double alpha = 1.) : Color(HSLTag{}, values[0], values[1], values[2], alpha) {}

    // Setters
    void set(const Color& color) { *this = color; }
    void set(double r, double g, double b, double a = 1.) { *this = Color(r, g, b, a); }
    void set(const std::array<double, 4>& values) { *this = Color(values); }
    void set(int r, int g, int b, int a = 255) { *this = Color(r, g, b, a); }
    void set(const unsigned int& hex) { *this = Color(hex); }
    void set(const std::string& hexStr) { *this = Color(hexStr); }
    void set(HSVTag, double h, double s, double v, double alpha = 1.) { *this = Color(HSVTag{}, h, s, v, alpha); }
    void set(HSLTag, double h, double s, double l, double alpha = 1.) { *this = Color(HSLTag{}, h, s, l, alpha); }

    // Getters
    std::array<double, 4> asDoubleArray() const { return { r, g, b, a }; }
    std::array<float, 4> asFloatArray() const { return { static_cast<float>(r), static_cast<float>(g), static_cast<float>(b), static_cast<float>(a) }; }

    std::array<int, 4> asIntArray() const
    {
        return { clampInt(r * 255),
                 clampInt(g * 255),
                 clampInt(b * 255),
                 clampInt(a * 255) };
    }

    int asHex() const
	{
        return (clampInt(r * 255) << 24) |
			   (clampInt(g * 255) << 16) |
			   (clampInt(b * 255) << 8) |
			   (clampInt(a * 255));
	}

    // return lowercase hex string in the format #RRGGBBAA
    std::string asHexStr() const {
        std::stringstream ss;
        ss << "#"
            << std::setfill('0') << std::setw(2) << std::hex << clampInt(r * 255)
            << std::setfill('0') << std::setw(2) << std::hex << clampInt(g * 255)
            << std::setfill('0') << std::setw(2) << std::hex << clampInt(b * 255)
            << std::setfill('0') << std::setw(2) << std::hex << clampInt(a * 255);
        return ss.str();
    }

    // return array of hue, saturation, and value
    std::array<double, 3> asHSV() const
	{
		double max = std::max(r, std::max(g, b));
		double min = std::min(r, std::min(g, b));
		double chroma = max - min;

		double huePrime = 0.0;
		if (chroma != 0.0)
		{
			if (max == r)
				huePrime = std::fmod((g - b) / chroma, 6.0);
			else if (max == g)
				huePrime = ((b - r) / chroma) + 2.0;
			else // max == b
				huePrime = ((r - g) / chroma) + 4.0;
		}

		double hue = huePrime * 60.0;
		if (hue < 0.0)
			hue += 360.0;

		double value = max;
		double saturation = 0.0;
		if (max != 0.0)
			saturation = chroma / max;

		return { hue, saturation, value };
	}

    // return array of hue, saturation, and lightness
    std::array<double, 3> asHSL() const
	{
		double max = std::max(r, std::max(g, b));
		double min = std::min(r, std::min(g, b));
		double chroma = max - min;

		double huePrime = 0.0;
		if (chroma != 0.0)
		{
			if (max == r)
				huePrime = std::fmod((g - b) / chroma, 6.0);
			else if (max == g)
				huePrime = ((b - r) / chroma) + 2.0;
			else // max == b
				huePrime = ((r - g) / chroma) + 4.0;
		}

		double hue = huePrime * 60.0;
		if (hue < 0.0)
			hue += 360.0;

		double lightness = (max + min) / 2.0;
		double saturation = 0.0;
		if (lightness != 0.0 && lightness != 1.0)
			saturation = chroma / (1.0 - std::abs(2.0 * lightness - 1.0));

		return { hue, saturation, lightness };
	}
    

    // Copy constructor and assignment operator (if needed)
    Color(const Color& other) = default;
    Color& operator=(const Color& other) = default;

private:
    // Helper function to clamp values between 0 and 1
	double clamp(double value) const { return std::max(0.0, std::min(1.0, value)); }

    // Helper function to clamp values between 0 and 255
    int clampInt(double value) const { return static_cast<int>(std::max(0, std::min(255, static_cast<int>(value)))); }

    // Helper function to verify that a string is a valid hexadecimal string
    bool isHexStr(const std::string& hexStr) const
	{
        char pound[1]{ '#' };
        if (hexStr.length() != 9 || hexStr.length() != 7 && hexStr[0] != '#')
			return false;

		for (int i = 1; i < hexStr.length(); ++i)
		{
			if (!isxdigit(hexStr[i]))
				return false;
		}

        //std::cout << "is hex str" << std::endl;
		return true;
	}


};

static_assert(sizeof(Color) == sizeof(double) * 4, "ColorRGBA size does not match expected size.");
static_assert(std::is_standard_layout_v<Color>, "ColorRGBA must be standard layout.");

template<typename T>
void colorSetTo(const Color& color, T& output);

// specialization example for VkClearColorValue
//template<>
//void colorSetTo(const Color& color, VkClearColorValue& output) {
//	output.float32[0] = color.r;
//	output.float32[1] = color.g;
//	output.float32[2] = color.b;
//	output.float32[3] = color.a;
//}