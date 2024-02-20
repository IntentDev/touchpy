#pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>


//#include "OpenImageIO/imageio.h" // causing CUDA to not compile

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <string>

namespace utils
{


	// need to create a custom format and type description to pass to this function
	// so it can use any library
	//void ImageLoadUINT8(std::string path, int &width, int &height, int &numChannels, std::unique_ptr<unsigned char[]> &pixels);


	template<typename T>
	T getMinimumBitMask(T a, T b) {
		T combined = static_cast<T>(a | b);
		return static_cast<T>(combined & (-combined)); // two's complement: -combined = ~combined + 1
	}

	std::string generateUUID(uint32_t length = 0);

	std::string stringTrimStart(const std::string& str, size_t length);

	std::string stringTrimEnd(const std::string& str, size_t length);

	std::string stringTrim(const std::string& str, size_t front, size_t back);


	template <typename T>
	void printTypeInfo()
	{
		constexpr int width = 30;
		std::cout << std::endl;
		std::cout << std::left << std::setfill(' ');
		std::cout << std::setw(width) << "\t\ttype: " << typeid(T).name() << std::endl;
		std::cout << std::setw(width) << "\t\tis trivial:" << std::boolalpha << std::is_trivial<T>::value << std::endl;
		std::cout << std::setw(width) << "\t\tis trivially copyable:" << std::boolalpha << std::is_trivially_copyable<T>::value << std::endl;
		std::cout << std::setw(width) << "\t\tis standard layout:" << std::boolalpha << std::is_standard_layout<T>::value << std::endl;

		std::cout << std::setw(width) << "\t\tsize:" << sizeof(T) << std::endl;
		std::cout << std::setw(width) << "\t\talignment:" << alignof(T) << std::endl;

		std::cout << std::endl;
	}

	template<typename T>
	inline void safeMemCpy(T* dest, const T* src, size_t count = 1) 
	{
		static_assert(std::is_trivial<T>::value, "Type must be trivial");
		static_assert(std::is_trivially_copyable<T>::value, "Type must be trivially copyable");
		static_assert(std::is_standard_layout<T>::value, "Type must have standard layout");

		std::memcpy(dest, src, count * sizeof(T));
	}

	// iterate over tuple and call function on each element (lambda or function pointer)
	// works with values and pointers as specified by the tuple definition
	// example:
	//			using MyTypes = std::tuple<int, float, Object*>
	//			MyTypes types = MyTypes { 1, 2.0f, new Object() };
	//			forEach(types, [](auto& element) { element->doSomething(); });
	// 
	template <class Tuple, class F>
	constexpr decltype(auto) forEach(Tuple&& tuple, F&& f)
	{
		return[] <std::size_t... I>
			(Tuple && tuple, F && f, std::index_sequence<I...>)
		{
			(f(std::get<I>(tuple)), ...);
			return f;
		}(std::forward<Tuple>(tuple), std::forward<F>(f),
			std::make_index_sequence<std::tuple_size<std::remove_reference_t<Tuple>>::value>{});
	}


} // namespace utils