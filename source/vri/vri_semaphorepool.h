#pragma once
#include "common/helpers.h"
#include "vri_macros.h"
#include "vri_initializers.h"
#include <vector>

NAMESPACE_BEGIN(vri)

class SemaphorePool 
{
public:
    SemaphorePool(VkDevice device, size_t initialSize = 0) 
        :   device_(device) 
    {
        for (size_t i = 0; i < initialSize; ++i)
            unusedSemaphores_.push_back(createSemaphore());
    }

    ~SemaphorePool() 
    {
        vkDeviceWaitIdle(device_);
        for (auto semaphore : semaphores_)
            vkDestroySemaphore(device_, semaphore, nullptr);
    }

    VkSemaphore acquire() 
    {
        if (!unusedSemaphores_.empty()) 
        {
            VkSemaphore semaphore = unusedSemaphores_.back();
            unusedSemaphores_.pop_back();
            return semaphore;
        }
        return createSemaphore();
    }

    void release(VkSemaphore semaphore) { unusedSemaphores_.push_back(semaphore); }

    void cleanupUnused(size_t maxUnused) 
    {
        if (unusedSemaphores_.size() <= maxUnused) 
            return;

        vkDeviceWaitIdle(device_);
        while (unusedSemaphores_.size() > maxUnused) 
        {
            VkSemaphore semaphore = unusedSemaphores_.back();
            vkDestroySemaphore(device_, semaphore, nullptr);
            unusedSemaphores_.pop_back();
            semaphores_.erase(std::remove(
                semaphores_.begin(), semaphores_.end(), semaphore), semaphores_.end());
        }
    }

private:
    VkSemaphore createSemaphore() 
    {
        VkSemaphore semaphore;
        VkSemaphoreCreateInfo semaphoreInfo = vri::semaphoreCreateInfo();
        VK_CHECK(vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &semaphore));
        semaphores_.push_back(semaphore);
        return semaphore;
    }

    std::vector<VkSemaphore> semaphores_;
    std::vector<VkSemaphore> unusedSemaphores_;
    VkDevice device_;
};

NAMESPACE_END(vri)
