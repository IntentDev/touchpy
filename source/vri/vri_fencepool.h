#pragma once
#include "common/helpers.h"
#include "vri_macros.h"
#include "vri_initializers.h"
#include <vector>

NAMESPACE_BEGIN(vri)

class FencePool 
{
public:
    FencePool(VkDevice device, size_t initialSize = 0) 
        :   device_(device) 
    {
        for (size_t i = 0; i < initialSize; ++i) 
            unusedFences_.push_back(createFence());
        
    }

    ~FencePool() 
    {
        vkDeviceWaitIdle(device_);
        for (auto fence : fences_)
            vkDestroyFence(device_, fence, nullptr);
    }

    VkFence acquire() 
    {
        if (!unusedFences_.empty()) 
        {
            VkFence fence = unusedFences_.back();
            unusedFences_.pop_back();
            // Optionally reset the fence before returning it
            vkResetFences(device_, 1, &fence);
            return fence;
        }
        return createFence();
    }

    void release(VkFence fence) { unusedFences_.push_back(fence); }

    void cleanupUnused(size_t maxUnused) 
    {
        if (unusedFences_.size() <= maxUnused) 
            return;

        vkWaitForFences(device_, static_cast<uint32_t>(unusedFences_.size()), unusedFences_.data(), VK_TRUE, UINT64_MAX);
        while (unusedFences_.size() > maxUnused) 
        {
            VkFence fence = unusedFences_.back();
            vkDestroyFence(device_, fence, nullptr);
            unusedFences_.pop_back();
            fences_.erase(std::remove(fences_.begin(), fences_.end(), fence), fences_.end());
        }
    }

private:
    VkFence createFence() 
    {
        VkFence fence;
        VkFenceCreateInfo fenceInfo = vri::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
        VK_CHECK(vkCreateFence(device_, &fenceInfo, nullptr, &fence));
        fences_.push_back(fence);
        return fence;
    }

    std::vector<VkFence> fences_;
    std::vector<VkFence> unusedFences_;
    VkDevice device_;
};

NAMESPACE_END(vri)
