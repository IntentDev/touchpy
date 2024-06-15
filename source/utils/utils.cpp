#include "utils.h"
#include <array>
#include <filesystem>
#include <sstream>
#include <random>
#include <iomanip>
#include <iostream>
#include <type_traits>


NAMESPACE_BEGIN(utils)


//void utils::ImageLoadUINT8(std::string path, int& width, int& height, int& numChannels, std::unique_ptr<unsigned char[]>& pixels)
//{
//	auto inp = OIIO::ImageInput::open(path);
//	if (!inp)
//		return;
//	const OIIO::ImageSpec& spec = inp->spec();
//	width = spec.width;
//	height = spec.height;
//	numChannels = spec.nchannels;
//	OIIO::TypeDesc format = spec.format;
//
//	std::cout << "Loading texture: " << path << "\n";
//	std::cout << "\txres = " << width << ", yres = " << height << ", nchannels = " << numChannels << " format: " << format.c_str() << std::endl;
//
//	pixels = std::unique_ptr<unsigned char[]>(new unsigned char[width * height * numChannels]);
//
//	inp->read_image(0, 0, 0, numChannels, OIIO::TypeDesc::UINT8, &pixels[0]);
//	inp->close();
//}


std::string generateUUID(uint32_t length) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, 15);

	const char* hexChars = "0123456789abcdef";
	std::stringstream ss;

	for (int i = 0; i < 36; ++i) {
		if (i == 8 || i == 13 || i == 18 || i == 23) {
			ss << '-';
		}
		else if (i == 14) {
			ss << '4';
		}
		else if (i == 19) {
			ss << hexChars[(dis(gen) & 0x3) | 0x8];
		}
		else {
			ss << hexChars[dis(gen)];
		}
	}
	if (length == 0)
		return ss.str();

	std::string shortUUID = ss.str();
	shortUUID.resize(std::min(36u, length));
	return shortUUID;
}

std::string stringTrimStart(const std::string& str, size_t length) {
	if (str.length() <= length) {
		return "";
	}
	return str.substr(length);
}

std::string stringTrimEnd(const std::string& str, size_t length) {
	if (str.length() <= length) {
		return "";
	}
	return str.substr(0, str.length() - length);
}

std::string stringTrim(const std::string& str, size_t front, size_t back) {
	if (front + back >= str.length()) {
		return "";
	}
	return str.substr(front, str.length() - front - back);
}


std::string arrayToHexString(const uint8_t* data, size_t size)
{
	std::string hexString;
	for (size_t i = 0; i < size; ++i)
	{
		char buffer[3];
		sprintf(buffer, "%02X", data[i]);
		hexString += buffer;
	}
	return hexString;
}

std::string arrayToHexString(const char* data, size_t size)
{
	return arrayToHexString(reinterpret_cast<const uint8_t*>(data), size);
}


NAMESPACE_END(utils)