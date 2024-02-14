#include "vkcontext.h"

VkContext::VkContext()
{

}

VkContext::~VkContext()
{
}

void 
VkContext::cleanup()
{
	for (auto callback = --vDestroyCallbacks_.end();
		callback != vDestroyCallbacks_.begin(); --callback)
	{
		callback->operator()(); // same as (*callback)();
	}
}

void 
VkContext::init()
{
	createPrimaryDevice();
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
		std::cout << "TEVulkanContext created" << std::endl;
	}
}

void 
VkContext::setRequiredExtensions(std::vector<const char*> extensions)
{
	requiredExtensions_ = extensions;
}

void
VkContext::createInstance()
{
	//vri::printAvailableValidationLayers();
	//setRequiredExtensions(presenter_->getRequiredExtensions());

	vri::createInstance(vContext_, requiredExtensions_, validationLayers_, enableValidationLayers_, debugMessenger_);



	vDestroyCallbacks_.push_back([&]() {
		if (enableValidationLayers_)
			vri::destroyDebugUtilsMessengerEXT(
				vContext_.instance, debugMessenger_, nullptr);

		vkDestroyInstance(vContext_.instance, nullptr);
		std::cout << "Vulkan instance destroyed" << std::endl;
		});
}

void 
VkContext::createPrimaryDevice()
{
	vri::setPrimaryPhysicalDevice(vContext_, deviceExtensions_,
		VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT);

	vkGetPhysicalDeviceProperties(vContext_.physicalDevice,
		&vContext_.physicalDeviceProperties);

	vri::createDevice(vContext_, deviceExtensions_,
		validationLayers_, enableValidationLayers_);

	vDestroyCallbacks_.push_back([&]()
		{
			vkDestroyDevice(vContext_.device, nullptr); }
	);

}

void 
VkContext::allocateInstanceResources()
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
		VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
		vContext_.transferCommandPool);

	vDestroyCallbacks_.push_back([&]() {
		if (vContext_.graphicsCommandPool)
			vkDestroyCommandPool(vContext_.device, vContext_.graphicsCommandPool, nullptr);
		if (vContext_.transferCommandPool)
			vkDestroyCommandPool(vContext_.device, vContext_.transferCommandPool, nullptr);
		});

}


void 
VkContext::onFrameBegin()
{
	// wait and reset fences are now in presenter_->recordCommands()
	// if wait fences are enabled here they must be reset after recordCommands success
	// 
	// VkFrameFence frameFence = presenter_->nextFrameFence();
	// vkWaitForFences(vContext_.device, 1, &frameFence, VK_TRUE, UINT64_MAX);


	//presenter_->recordCommands();
	// vkResetFences(vContext_.device, 1, &frameFence);

	//presenter_->submitPresent(vContext_.graphicsQueue);

	vContext_.currentFrame = (vContext_.currentFrame + 1) % vContext_.maxFramesInFlight;
}

void 
VkContext::onFrameEnd()
{

}