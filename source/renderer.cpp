#include "renderer.h"
#include "deviceinfo.h"
#include "logging.h"


const std::string Renderer::ConfigureError = "Vulkan is not supported or the selected GPU does not have the needed features.";

std::unordered_map<uint8_t, std::shared_ptr<Renderer>> Renderer::instances_;
std::mutex Renderer::instancesMutex_;

std::shared_ptr<Renderer> 
Renderer::instance(uint8_t gpuIndex)
{
	std::lock_guard<std::mutex> lock(instancesMutex_);
	auto it = instances_.find(gpuIndex);
	if (it == instances_.end())
	{
		instances_[gpuIndex].reset(new Renderer(gpuIndex));
	}
	return instances_[gpuIndex];
}

void
Renderer::initSingleton(uint8_t gpuIndex)
{
	// this will attempt to call the private/protected constructor and won't compile without derived class
	//instances_[gpuIndex] = std::make_shared<Renderer>(gpuIndex); 

	instances_[gpuIndex].reset(new Renderer(gpuIndex));
}

Renderer::Renderer(uint8_t gpuIndex)
{
	createVkInstance();
	auto deviceInfos = enumerateDevices(vContext_.instance);

	if (deviceInfos.size() == 0)
	{
		spdlog::error("No Vulkan/CUDA compatible devices found");
		return;
	}

	if (gpuIndex >= deviceInfos.size())
	{
		spdlog::error("Invalid GPU index");
		return;
	}

	auto gpuInfo = deviceInfos[gpuIndex];
	init(gpuInfo);
}

Renderer::~Renderer()
{
	cleanup();
}

void 
Renderer::cleanup()
{
	// wait for device to finish
	vkDeviceWaitIdle(vContext_.device);

	for (auto& callback = --vDestroyCallbacks_.end();
		callback != vDestroyCallbacks_.begin(); --callback)
	{
		callback->operator()(); // same as (*callback)();
	}
}

void
Renderer::createVkInstance()
{
	//vri::printAvailableValidationLayers();
	//setRequiredExtensions(presenter_->getRequiredExtensions());

	vri::createInstance(vContext_, requiredExtensions_, validationLayers_, enableValidationLayers_, debugMessenger_);

	vDestroyCallbacks_.push_back([&]() {
		if (enableValidationLayers_)
			vri::destroyDebugUtilsMessengerEXT(
				vContext_.instance, debugMessenger_, nullptr);

		vkDestroyInstance(vContext_.instance, nullptr);
		spdlog::debug("Vulkan instance destroyed");
		SPDLOG_FLUSH
		});
}


void 
Renderer::init(DeviceInfo deviceInfo)
{
	createPrimaryDevice(deviceInfo.uuid);
	allocateInstanceResources();

	VkPhysicalDeviceProperties2  physicalDeviceProperties{ };
	physicalDeviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

	VkPhysicalDeviceIDProperties  physicalDeviceIDProperties{ };
	physicalDeviceIDProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES;
	physicalDeviceProperties.pNext = &physicalDeviceIDProperties;

	vkGetPhysicalDeviceProperties2(vContext_.physicalDevice, &physicalDeviceProperties);

	TEResult result = TEVulkanContextCreate(
		physicalDeviceIDProperties.deviceUUID,
		physicalDeviceIDProperties.driverUUID,
		physicalDeviceIDProperties.deviceLUID,
		physicalDeviceIDProperties.deviceLUIDValid,
		TETextureOriginBottomLeft, 
		teContext_.take()
	);

	if (result != TEResultSuccess)
	{
		throw std::runtime_error("Failed to create TEVulkanContext");
	}
	else
	{
		spdlog::debug("TEVulkanContext created");
		SPDLOG_FLUSH
	}

	
    std::memcpy(physicalDeviceUUID_, physicalDeviceIDProperties.deviceUUID, sizeof(uint8_t) * 16);
}

void 
Renderer::setRequiredExtensions(std::vector<const char*> extensions)
{
	requiredExtensions_ = extensions;
}

void 
Renderer::createPrimaryDevice(uint8_t uuid[16])
{
	if(vri::setPrimaryPhysicalDevice(vContext_, deviceExtensions_, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT, uuid))
	{
		vkGetPhysicalDeviceProperties(vContext_.physicalDevice,
			&vContext_.physicalDeviceProperties);

		spdlog::debug("Selected GPU: {}, uuid: {}", vContext_.physicalDeviceProperties.deviceName, utils::arrayToHexString(uuid, 16));
		SPDLOG_FLUSH

			vri::createDevice(vContext_, deviceExtensions_,
				validationLayers_, enableValidationLayers_);

		vDestroyCallbacks_.push_back([&]()
			{
				vkDestroyDevice(vContext_.device, nullptr); }
		);
	}
	else
	{
		spdlog::error("Failed to set primary physical Vulkan device");
	}
}

void 
Renderer::allocateInstanceResources()
{
	vri::createVmaAllocator(vContext_);
	vDestroyCallbacks_.push_back([&]() { vmaDestroyAllocator(vContext_.allocator); });

	vri::createDescriptorPool(vContext_, descriptorPool_, vContext_.maxFramesInFlight);
	vDestroyCallbacks_.push_back([&]() {
		if (descriptorPool_ != nullptr)
			vkDestroyDescriptorPool(vContext_.device, descriptorPool_, nullptr);
		});

	vri::createCommandPool(
		vContext_.device,
		vContext_.graphicsFamily.value(),
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		vContext_.graphicsCommandPool);

	vri::createCommandPool(
		vContext_.device,
		vContext_.transferFamily.value(),
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, // | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
		vContext_.transferCommandPool);

	vDestroyCallbacks_.push_back([&]() {
		if (vContext_.graphicsCommandPool)
			vkDestroyCommandPool(vContext_.device, vContext_.graphicsCommandPool, nullptr);
		if (vContext_.transferCommandPool)
			vkDestroyCommandPool(vContext_.device, vContext_.transferCommandPool, nullptr);
		});

	vri::allocateCommandBuffers(
		vContext_.device, 
		vContext_.transferCommandPool, 
		vContext_.transferCommandBuffers, 
		vContext_.maxFramesInFlight);

	vDestroyCallbacks_.push_back([&]() {
		if (vContext_.transferCommandBuffers.size() > 0)
			vkFreeCommandBuffers(vContext_.device, vContext_.transferCommandPool, 
				static_cast<uint32_t>(vContext_.transferCommandBuffers.size()), 
				vContext_.transferCommandBuffers.data());
		});

}


bool Renderer::configureTEInstance(TEInstance* instance, std::string& error)
{
	int32_t count = 0;
	TEResult result = TEInstanceGetSupportedTextureTypes(instance, nullptr, &count);
	if (result == TEResultInsufficientMemory)
	{
		std::vector<TETextureType> textureTypes(count);
		result = TEInstanceGetSupportedTextureTypes(instance, textureTypes.data(), &count);
		if (result == TEResultSuccess)
		{
			textureTypes.resize(count);
			if (std::find(textureTypes.begin(), textureTypes.end(), TETextureTypeVulkan) != textureTypes.end())
			{
				result = TEInstanceGetSupportedVkFormats(instance, nullptr, &count);
				if (result == TEResultInsufficientMemory)
				{
					std::vector<VkFormat> formats(count);
					result = TEInstanceGetSupportedVkFormats(instance, formats.data(), &count);
					if (result == TEResultSuccess)
					{
						formats.resize(count);

						// TODO: need to check all formats supported by tdpy
						if (std::find(formats.begin(), formats.end(), VK_FORMAT_R8G8B8A8_UNORM) == formats.end())
						{
							error = getConfigureError();
							return false;
						}
					}
				}
			}
		}
	}
	result = TEInstanceGetSupportedSemaphoreTypes(instance, nullptr, &count);
	if (result == TEResultInsufficientMemory)
	{
		std::vector<TESemaphoreType> semaphoreTypes(count);
		result = TEInstanceGetSupportedSemaphoreTypes(instance, semaphoreTypes.data(), &count);
		if (result == TEResultSuccess)
		{
			semaphoreTypes.resize(count);
			if (std::find(semaphoreTypes.begin(), semaphoreTypes.end(), TESemaphoreTypeVulkan) == semaphoreTypes.end())
			{
				error = getConfigureError();
				return false;
			}
		}
	}
	return true;
}


std::string Renderer::getConfigureError() const
{
	std::string composed = ConfigureError;

	if (vContext_.physicalDeviceProperties.deviceName != nullptr)
	{
		composed += "\nThe selected GPU is: ";
		composed += vContext_.physicalDeviceProperties.deviceName;
	}

	return composed;
}

//void Renderer::renderFrame()
//{
//	//onFrameBegin();
//	//onFrameEnd();
//}
//
//void
//Renderer::onFrameBegin()
//{
//	// wait and reset fences are now in presenter_->recordCommands()
//	// if wait fences are enabled here they must be reset after recordCommands success
//	// 
//	// VkFrameFence frameFence = presenter_->nextFrameFence();
//	// vkWaitForFences(vContext_.device, 1, &frameFence, VK_TRUE, UINT64_MAX);
//
//
//	//presenter_->recordCommands();
//	// vkResetFences(vContext_.device, 1, &frameFence);
//
//	//presenter_->submitPresent(vContext_.graphicsQueue);
//
//	vContext_.currentFrame = (vContext_.currentFrame + 1) % vContext_.maxFramesInFlight;
//}
//
//void 
//Renderer::onFrameEnd()
//{
//
//}