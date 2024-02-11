#pragma once

#include <optional>
#include <vector>
#include <tuple>
#include <vulkan/vulkan.h>

struct QueueFamilyInfo 
{
    uint32_t familyIndex { 0 };
    uint32_t queueCount { 0 };
    uint32_t queuesAllocated { 0 };
    VkQueueFlags queueFlags { 0 };
    VkBool32 presentSupport { VK_FALSE };
    std::vector<float> queuePriorities { };
    std::vector<float> normalizedQueuePriorities { };

    void setQueuePrioriy(uint32_t queueIndex, float priority)
    {
        if (queueIndex <= queuesAllocated)
        {
            queuePriorities.resize(queueIndex + 1);
        }
        queuePriorities[queueIndex] = priority;
        setNormalizedPriorities();
    }

private:
    void setNormalizedPriorities()
    {
        normalizedQueuePriorities.clear();
		float totalPriority = 0.0f;
		for (auto& p : queuePriorities) { totalPriority += p; }
		for (auto& p : queuePriorities) { normalizedQueuePriorities.push_back(p / totalPriority); }
	}

};

using deviceQueueInfo = std::tuple<std::optional<uint32_t>, uint32_t, VkQueue*>;

class QueueFamilyIndices 
{
public:
    QueueFamilyIndices() = default;

    QueueFamilyIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) 
    {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(
            physicalDevice, &queueFamilyCount, queueFamilies.data());

        familyInfos_;

        for (uint32_t i = 0; i < queueFamilyCount; i++)
        {
            QueueFamilyInfo familyInfo;
            familyInfo.familyIndex = i;
            familyInfo.queueCount = queueFamilies[i].queueCount;
            familyInfo.queueFlags = queueFamilies[i].queueFlags;

            if (surface != VK_NULL_HANDLE)
            {
                vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &familyInfo.presentSupport);
            }

            familyInfos_.push_back(familyInfo);
        }
    }

    deviceQueueInfo setDeviceQueueInfo(VkQueueFlags queueFlags, 
        VkQueue* queue, std::optional<uint32_t> preference = std::nullopt, float priority = 1.0f)
	{
		auto queueFamilyIndex = getQueueFamilyIndex(queueFlags, preference);
		if (queueFamilyIndex.has_value()) 
		{
            return deviceQueueInfoUtil(queueFamilyIndex.value(), queue, priority);
		}

		return std::make_tuple(std::nullopt, 0, queue);
	}

    deviceQueueInfo setPresentQueueInfo(VkQueue* queue,
        std::optional<uint32_t> preference = std::nullopt, float priority = 1.0f)
	{
		auto queueFamilyIndex = getPresentQueueFamilyIndex(preference);
		if (queueFamilyIndex.has_value())
		{
            return deviceQueueInfoUtil(queueFamilyIndex.value(), queue, priority);
		}
        return std::make_tuple(std::nullopt, 0, queue);
	}

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos()
    {
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos_;
        for (auto& queueFamilyInfo : queueFamilyInfos())
        {
            if (queueFamilyInfo.queuesAllocated == 0)
                continue;

            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamilyInfo.familyIndex;
            queueCreateInfo.queueCount = queueFamilyInfo.queuesAllocated;
            queueCreateInfo.pQueuePriorities = queueFamilyInfo.normalizedQueuePriorities.data();

            queueCreateInfos_.push_back(queueCreateInfo);
        }
        return queueCreateInfos_;
    }

    const std::vector<QueueFamilyInfo>& queueFamilyInfos() const { return familyInfos_; }
    const std::vector<uint32_t>& usedFamilyIndices() const { return usedFamilyIndices_; }


private:
    std::vector<QueueFamilyInfo> familyInfos_;
    std::vector<uint32_t> usedFamilyIndices_;

    deviceQueueInfo deviceQueueInfoUtil(uint32_t familyIndex, VkQueue* queue, float priority = 1.0f)
	{
        // familyIndex not in familyIndices add it
        if (std::find(usedFamilyIndices_.begin(), usedFamilyIndices_.end(),
            familyIndex) == usedFamilyIndices_.end())
		{
            usedFamilyIndices_.push_back(familyIndex);
		}

        auto familyInfo = familyInfos_[familyIndex];
        auto queueIndex = familyInfo.queuesAllocated - 1;
        familyInfos_[familyIndex].setQueuePrioriy(queueIndex, priority);
		return std::make_tuple(familyIndex, queueIndex, queue);
	}

    // Returns the index of a queue family that supports the requested queue flags
    // If a preference is provided, it will try to allocate a queue from that family first
    std::optional<uint32_t> getQueueFamilyIndex(VkQueueFlags queueFlags, 
        std::optional<uint32_t> preference = std::nullopt, float priority = 1.0f)
    {
        // use preference if it is valid
        if (preference.has_value() && preference.value() < familyInfos_.size())
        {
            auto& family = familyInfos_[preference.value()];
            if ((family.queueFlags & queueFlags) && family.queuesAllocated < family.queueCount)
            {
                family.queuesAllocated++;
                return family.familyIndex;
            }
        }

        // try to find a family with no queues allocated
        for (auto& family : familyInfos_)
        {
            if ((family.queueFlags & queueFlags) && family.queuesAllocated == 0)
            {
                family.queuesAllocated++;
                return family.familyIndex;
            }
        }

        // Next try to find a family with available queues
        for (auto& family : familyInfos_)
        {
            if ((family.queueFlags & queueFlags) && family.queuesAllocated < family.queueCount)
            {
                family.queuesAllocated++;
                return family.familyIndex;
            }
        }

        // No available queues
        return std::nullopt;
    }

    // Returns the index of first available queue family that supports presentation
    std::optional<uint32_t> getPresentQueueFamilyIndex(
        std::optional<uint32_t> preference = std::nullopt, float priority = 1.0f)
    {
        // use preference if it is valid
        if (preference.has_value() && preference.value() < familyInfos_.size())
        {
            auto& family = familyInfos_[preference.value()];
			if (family.presentSupport && family.queuesAllocated < family.queueCount)
			{
                family.queuesAllocated++;
				return family.familyIndex;
			}
		}

        // get first available queue family that supports presentation
        for (auto& family : familyInfos_)
        {
            if (family.presentSupport && family.queuesAllocated < family.queueCount)
            {
                family.queuesAllocated++;
                return family.familyIndex;
            }
        }

        return std::nullopt;
    }

};



