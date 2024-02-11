#pragma once

#include "vri/vri.h"
#include <functional>
#include <vector>

class VkContext
{
public:
    VkContext();
    ~VkContext();
    void cleanup();
    void init();

    //void setPresenter(Presenter* presenter) { presenter_ = presenter; }

    // TODO: make this a generic list with a uniform interface for all renderers
    //void setUIGraphics(UIGraphics* uiGraphics) { uiGraphics_ = uiGraphics; }

    void setRequiredExtensions(std::vector<const char*> extensions);
    void createInstance();
    void createPrimaryDevice();
    void allocateInstanceResources();

    void onFrameBegin();
    void onFrameEnd();


    vri::VContext& vContext() { return vContext_; }

private:

    //Vision& vision_;
    //Presenter* presenter_{ nullptr };
    //UIGraphics* uiGraphics_{ nullptr };

    vri::VContext                       vContext_{ };
    std::vector<const char*>            requiredExtensions_{ };
    VkDebugUtilsMessengerEXT            debugMessenger_{ nullptr };
    std::vector<std::function<void()>>  vDestroyCallbacks_;

    VkDescriptorPool                    descriptorPool_{ nullptr };



//#ifdef NDEBUG
//    const bool                         enableValidationLayers_{ false };
//#else
    const bool                         enableValidationLayers_{ true };
//#endif
    const std::vector<const char*>     validationLayers_
    {
    "VK_LAYER_KHRONOS_validation",

    };

    const std::vector<const char*>     deviceExtensions_
    {
        VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
        VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME,
        VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME,
    };
};