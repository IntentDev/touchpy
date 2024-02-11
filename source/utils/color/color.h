#pragma once

#include <array>
#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>


struct Color
{
    float r, g, b, a;

    struct HSVTag {};
    struct HSLTag {};

    Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
    Color(float r, float g, float b, float a = 1.f) : r(r), g(g), b(b), a(a) {}
    Color(const std::array<float, 4>& values) : r(values[0]), g(values[1]), b(values[2]), a(values[3]) {}

    // integer values (0-255)
    Color(int r, int g, int b, int a = 255) : r(r / 255.0f), g(g / 255.0f), b(b / 255.0f), a(a / 255.0f) {}

    Color(const int& hexValue)
    {
        r = ((hexValue >> 24) & 0xFF) / 255.0f;
        g = ((hexValue >> 16) & 0xFF) / 255.0f;
        b = ((hexValue >> 8) & 0xFF) / 255.0f;
        a = ((hexValue) & 0xFF) / 255.0f;
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
            r = ((hexValue >> 24) & 0xFF) / 255.0f;
            g = ((hexValue >> 16) & 0xFF) / 255.0f;
            b = ((hexValue >> 8) & 0xFF) / 255.0f;
            a = ((hexValue) & 0xFF) / 255.0f;
        }
        else
        {
            r = 0.0f;
            g = 0.0f;
            b = 0.0f;
            a = 1.0f;
        }
    }

    // HSV values
    Color(HSVTag, float hue, float sat, float value, float alpha = 1.f)
	{
		float chroma = value * sat;
		float huePrime = std::fmod(hue / 60.0f, 6.0f);
		float x = chroma * (1.0f - std::abs(std::fmod(huePrime, 2.0f) - 1.0f));
		float m = value - chroma;

		if (huePrime < 1.0f)      { r = chroma; g = x; b = 0.0f; }
		else if (huePrime < 2.0f) { r = x; g = chroma; b = 0.0f; }
		else if (huePrime < 3.0f) { r = 0.0f; g = chroma; b = x; }
		else if (huePrime < 4.0f) { r = 0.0f; g = x; b = chroma; }
		else if (huePrime < 5.0f) { r = x; g = 0.0f; b = chroma; }
		else if (huePrime < 6.0f) { r = chroma; g = 0.0f; b = x; }
		else                      { r = 0.0f; g = 0.0f;b = 0.0f; }

		r += m;
		g += m;
		b += m;
		a = alpha;
	}

    Color(HSVTag, const std::array<float, 3>& values, float alpha = 1.f) : Color(HSVTag{}, values[0], values[1], values[2], alpha) {}

	// HSL values
	Color(HSLTag, float hue, float sat, float lightness, float alpha = 1.f)
    {
        float chroma = (1.0f - std::abs(2.0f * lightness - 1.0f)) * sat;
        float huePrime = std::fmod(hue / 60.0f, 6.0f);
        float x = chroma * (1.0f - std::abs(std::fmod(huePrime, 2.0f) - 1.0f));
        float m = lightness - chroma / 2.0f;

        if (huePrime < 1.0f)      { r = chroma;  g = x; b = 0.0f; }
        else if (huePrime < 2.0f) { r = x;  g = chroma; b = 0.0f; }
        else if (huePrime < 3.0f) { r = 0.0f; g = chroma; b = x; }
        else if (huePrime < 4.0f) { r = 0.0f; g = x; b = chroma; }
        else if (huePrime < 5.0f) { r = x; g = 0.0f; b = chroma; }
        else if (huePrime < 6.0f) { r = chroma; g = 0.0f; b = x; }
        else                      { r = 0.0f; g = 0.0f; b = 0.0f; }

        r += m;
        g += m;
        b += m;
        a = alpha;
    }

    Color(HSLTag, const std::array<float, 3>& values, float alpha = 1.f) : Color(HSLTag{}, values[0], values[1], values[2], alpha) {}

    // Setters
    void set(const Color& color) { *this = color; }
    void set(float r, float g, float b, float a = 1.f) { *this = Color(r, g, b, a); }
    void set(const std::array<float, 4>& values) { *this = Color(values); }
    void set(int r, int g, int b, int a = 255) { *this = Color(r, g, b, a); }
    void set(const unsigned int& hex) { *this = Color(hex); }
    void set(const std::string& hexStr) { *this = Color(hexStr); }
    void set(HSVTag, float h, float s, float v, float alpha = 1.f) { *this = Color(HSVTag{}, h, s, v, alpha); }
    void set(HSLTag, float h, float s, float l, float alpha = 1.f) { *this = Color(HSLTag{}, h, s, l, alpha); }

    // Getters
    std::array<float, 4> asFloatArray() const { return { r, g, b, a }; }

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
    std::array<float, 3> asHSV() const
	{
		float max = std::max(r, std::max(g, b));
		float min = std::min(r, std::min(g, b));
		float chroma = max - min;

		float huePrime = 0.0f;
		if (chroma != 0.0f)
		{
			if (max == r)
				huePrime = std::fmod((g - b) / chroma, 6.0f);
			else if (max == g)
				huePrime = ((b - r) / chroma) + 2.0f;
			else // max == b
				huePrime = ((r - g) / chroma) + 4.0f;
		}

		float hue = huePrime * 60.0f;
		if (hue < 0.0f)
			hue += 360.0f;

		float value = max;
		float saturation = 0.0f;
		if (max != 0.0f)
			saturation = chroma / max;

		return { hue, saturation, value };
	}

    // return array of hue, saturation, and lightness
    std::array<float, 3> asHSL() const
	{
		float max = std::max(r, std::max(g, b));
		float min = std::min(r, std::min(g, b));
		float chroma = max - min;

		float huePrime = 0.0f;
		if (chroma != 0.0f)
		{
			if (max == r)
				huePrime = std::fmod((g - b) / chroma, 6.0f);
			else if (max == g)
				huePrime = ((b - r) / chroma) + 2.0f;
			else // max == b
				huePrime = ((r - g) / chroma) + 4.0f;
		}

		float hue = huePrime * 60.0f;
		if (hue < 0.0f)
			hue += 360.0f;

		float lightness = (max + min) / 2.0f;
		float saturation = 0.0f;
		if (lightness != 0.0f && lightness != 1.0f)
			saturation = chroma / (1.0f - std::abs(2.0f * lightness - 1.0f));

		return { hue, saturation, lightness };
	}
    

    // Copy constructor and assignment operator (if needed)
    Color(const Color& other) = default;
    Color& operator=(const Color& other) = default;

private:
    // Helper function to clamp values between 0 and 1
	float clamp(float value) const { return std::max(0.0f, std::min(1.0f, value)); }

    // Helper function to clamp values between 0 and 255
    int clampInt(float value) const { return static_cast<int>(std::max(0, std::min(255, static_cast<int>(value)))); }

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