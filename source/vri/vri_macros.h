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
		std::cout << "Vulkan error: " <<  __FILE__ << ": " << __LINE__ << ": "		  \
			<< string_VkResult(err) << std::endl;									  \
		abort();                                                                      \
	}                                                                                 \
} while (0)
#endif


//#define VK_CHECK(x)                                                                   \
//do                                                                                    \
//{                                                                                     \
//	VkResult err = x;                                                                 \
//	if (err)                                                                          \
//	{                                                                                 \
//		std::cout << "Vulkan error: " << string_VkResult(err) << std::endl;           \
//		abort();                                                                      \
//	}                                                                                 \
//} while (0)




//if (TESemaphoreGetType(semaphore) == TESemaphoreTypeVulkanFence)
//{
//    VkFence fence = static_cast<TEVulkanFence*>(semaphore.get())->GetHandle();
//    auto it = myOutputFences.find(fence);
//    if (it == myOutputFences.end())
//    {
//        // We cache output fences -
//        // TouchEngine's callbacks allow us to delete our cached fence when the original is deleted
//
//        VkFenceCreateInfo fenceCreateInfo{};
//        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
//        fenceCreateInfo.flags = 0;
//
//        VkFence newFence;
//        VkResult result = vkCreateFence(myDevice, &fenceCreateInfo, nullptr, &newFence);
//        if (result != VK_SUCCESS)
//        {
//            // Handle fence creation failure
//        }
//
//        it = myOutputFences.insert(std::make_pair(fence, newFence)).first;
//
//        static_cast<TEVulkanFence*>(semaphore.get())->SetCallback(fenceCallback, this);
//    }
//
//    vkWaitForFences(myDevice, 1, &it->second, VK_TRUE, waitValue);