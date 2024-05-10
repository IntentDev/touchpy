#pragma once
#include "vulkan/vk_enum_string_helper.h"
#include <iostream>
#include <logging.h>


//#ifdef NDEBUG
//#define VK_CHECK(x) x
//#else
//#define VK_CHECK(x)                                                                   \
//do                                                                                    \
//{                                                                                     \
//	VkResult err = x;                                                                 \
//	if (err)                                                                          \
//	{                                                                                 \
//		std::cout << "Vulkan error: " <<  __FILE__ << ": " << __LINE__ << ": "		  \
//			<< string_VkResult(err) << std::endl;									  \
//		abort();                                                                      \
//	}                                                                                 \
//} while (0)
//#endif

#define VK_CHECK(x)																				\
do																								\
{																								\
	VkResult err = x;																			\
	if (err)																					\
	{																							\
		spdlog::error("Vulkan error: {} : {} : {}", __FILE__, __LINE__, string_VkResult(err));  \
		throw std::runtime_error("Vulkan error");												\
	}																							\
} while (0)

