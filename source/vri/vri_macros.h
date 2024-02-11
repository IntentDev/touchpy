#pragma once
#include "vulkan/vk_enum_string_helper.h"
#include <iostream>


#ifdef NDEBUG
#define VK_CHECK(x) x
#else
#define VK_CHECK(x)                                                                   \
do                                                                                    \
{                                                                                     \
	VkResult err = x;                                                                 \
	if (err)                                                                          \
	{                                                                                 \
		std::cout << "Vulkan error: " << string_VkResult(err) << std::endl;           \
		abort();                                                                      \
	}                                                                                 \
} while (0)
#endif