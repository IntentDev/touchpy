#pragma once

#include <TouchEngine/TouchEngine.h>
#include <TouchEngine/TEVulkan.h>

#include "vri/vri.h"
#include <functional>
#include <vector>
#include <string>
#include <memory>
#include <mutex>



class Renderer
{
public:

    ~Renderer();

    static std::shared_ptr<Renderer> instance();

    uint8_t* physicalDeviceUUID() { return physicalDeviceUUID_; }
    vri::VContext& vContext() { return vContext_; }

    TouchObject<TEGraphicsContext> teContext() { return teContext_; }

private:
    uint8_t                             physicalDeviceUUID_[VK_UUID_SIZE] { };
    vri::VContext                       vContext_{ };
    std::vector<const char*>            requiredExtensions_{ };
    VkDebugUtilsMessengerEXT            debugMessenger_{ nullptr };
    std::vector<std::function<void()>>  vDestroyCallbacks_;

    VkDescriptorPool                    descriptorPool_{ nullptr };

    TouchObject<TEVulkanContext>        teContext_;

    static std::shared_ptr<Renderer> instance_;
    static std::once_flag initInstanceFlag_;
    
    Renderer();
    static void initSingleton();

    void init();
    void createVkInstance();
    void cleanup();
    bool configureTEInstance(TEInstance* instance, std::string& error);
    void createPrimaryDevice();
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