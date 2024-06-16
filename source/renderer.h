#pragma once

#include <TouchEngine/TouchEngine.h>
#include <TouchEngine/TEVulkan.h>

#include "vri/vri.h"

#include <functional>
#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include <unordered_map>

struct DeviceInfo;

class Renderer
{
public:

    ~Renderer();

    static std::shared_ptr<Renderer> instance(uint8_t gpuIndex);

    uint8_t* physicalDeviceUUID() { return vContext_.physicalDeviceIDProperties.deviceUUID; }
    vri::VContext& vContext() { return vContext_; }

    TouchObject<TEGraphicsContext> teContext() { return teContext_; }

private:
    vri::VContext                       vContext_{ };
    std::vector<const char*>            requiredExtensions_{ };
    VkDebugUtilsMessengerEXT            debugMessenger_{ nullptr };
    std::vector<std::function<void()>>  vDestroyCallbacks_;

    TouchObject<TEVulkanContext>        teContext_;

    static std::unordered_map<uint8_t, std::shared_ptr<Renderer>> instances_;
    static std::mutex instancesMutex_;
    
    Renderer(uint8_t gpuIndex);
    static void initSingleton(uint8_t gpuIndex);

    void init(DeviceInfo deviceInfo);
    void createVkInstance();
    void cleanup();
    bool configureTEInstance(TEInstance* instance, std::string& error);
    void createPrimaryDevice(DeviceInfo deviceInfo);
    void allocateInstanceResources();
    void setRequiredExtensions(std::vector<const char*> extensions);

    static const std::string ConfigureError;
    std::string getConfigureError() const;
    

#ifdef NDEBUG
    const bool                         enableValidationLayers_{ false };
#else
    const bool                         enableValidationLayers_{ true };
#endif
    const std::vector<const char*>     validationLayers_
    {
    "VK_LAYER_KHRONOS_validation",

    };

    const std::vector<const char*>     deviceExtensions_
    {
        VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
        VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME,
        VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME,
        VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,
        VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
        VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME,
        VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME,

    };





    
    

};