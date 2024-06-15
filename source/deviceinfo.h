#pragma once

#include <cuda_runtime.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <string>

#include "utils/utils.h"
#include "logging.h"

struct DeviceInfo
{
	uint8_t index;
	uint8_t luid[8];
	uint8_t uuid[16];
	std::string name;
	bool hasCompute { false };
	bool hasVulkan { false };
	int computeCapMajor;
	int computeCapMinor;
	int multiProcessorCount;
	uint64_t totalMemory;

};

std::vector<DeviceInfo> enumerateDevices(VkInstance instance)
{
	std::vector<DeviceInfo> devices;
	int deviceCount;
	cudaGetDeviceCount(&deviceCount);
	for (int i = 0; i < deviceCount; ++i)
	{
		cudaDeviceProp prop;
		cudaGetDeviceProperties(&prop, i);

		//spdlog::debug("CUDA   GPU {}: name: {}, luid: {}, uuid: {}", 
		//	i, prop.name, utils::arrayToHexString(prop.luid, 8), utils::arrayToHexString(prop.uuid.bytes, 16));

		DeviceInfo info;
		info.index = i;
		std::copy(prop.luid, prop.luid + 8, info.luid);
		std::copy(prop.uuid.bytes, prop.uuid.bytes + 16, info.uuid);
		info.name = prop.name;
		info.hasCompute = prop.computeMode != cudaComputeModeProhibited;
		info.computeCapMajor = prop.major;
		info.computeCapMinor = prop.minor;
		info.multiProcessorCount = prop.multiProcessorCount;
		info.totalMemory = prop.totalGlobalMem;
		devices.push_back(info);
	}

	uint32_t physicalDeviceCount;
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
	std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

	for (int i = 0; i < physicalDeviceCount; ++i)
	{
		VkPhysicalDeviceProperties2 physicalDeviceProperties;
		physicalDeviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

		VkPhysicalDeviceIDProperties  physicalDeviceIDProperties{ };
		physicalDeviceIDProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES;
		physicalDeviceProperties.pNext = &physicalDeviceIDProperties;

		vkGetPhysicalDeviceProperties2(physicalDevices[i], &physicalDeviceProperties);

		spdlog::debug("Vulkan GPU {}: name: {}, luid: {}, uuid: {}", 
			i, physicalDeviceProperties.properties.deviceName, utils::arrayToHexString(
				physicalDeviceIDProperties.deviceLUID, 8), utils::arrayToHexString(physicalDeviceIDProperties.deviceUUID, 16));

		for (auto& device : devices)
		{
			auto result = std::memcmp(device.uuid, physicalDeviceIDProperties.deviceUUID, 16);
			if (result == 0)
			{
				device.hasVulkan = true;
				break;
			}
		}
	}

	return devices;
}