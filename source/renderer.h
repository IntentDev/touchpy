#pragma once

#include "vri/vri.h"
#include <functional>
#include <vector>
#include <string>
#include <TouchEngine/TouchEngine.h>
#include <TouchEngine/TEVulkan.h>

class Renderer
{
public:
    Renderer();
    ~Renderer();
    void cleanup();
    void init();

    //void setPresenter(Presenter* presenter) { presenter_ = presenter; }

    // TODO: make this a generic list with a uniform interface for all renderers
    //void setUIGraphics(UIGraphics* uiGraphics) { uiGraphics_ = uiGraphics; }

    void setRequiredExtensions(std::vector<const char*> extensions);
    void createInstance();
    bool configureTEInstance(TEInstance* instance, std::string& error);
    void createPrimaryDevice();
    void allocateInstanceResources();



    void renderFrame();
    void onFrameBegin();
    void onFrameEnd();

    uint8_t* physicalDeviceUUID() { return physicalDeviceUUID_; }
    vri::VContext& vContext() { return vContext_; }

    TEGraphicsContext* teContext() { return teContext_.get(); }

    //bool doesInputTextureTransfer() const { return true; }

    //size_t getInputImageCount() const;
    //void beginImageLayout();

    //void addInputImage(
    //    const unsigned char* rgba, 
    //    size_t bytesPerRow, 
    //    int width, 
    //    int height);

    //bool getInputImage(
    //    size_t index, 
    //    TouchObject<TETexture> & texture, 
    //    TouchObject<TESemaphore> & semaphore, 
    //    uint64_t& waitValue);

    //void clearInputImages();
    //size_t getRightSideImageCount();
    //void addOutputImage();
    //void endImageLayout();

    //void clearOutputImages(); // TODO: ?

    //bool updateOutputImage(
    //    const TouchObject<TEInstance> & instance, 
    //    size_t index, 
    //    const std::string& identifier);

    //const TouchObject<TETexture>& getOutputImage(size_t index) const;



private:

    uint8_t                             physicalDeviceUUID_[VK_UUID_SIZE];
    vri::VContext                       vContext_{ };
    std::vector<const char*>            requiredExtensions_{ };
    VkDebugUtilsMessengerEXT            debugMessenger_{ nullptr };
    std::vector<std::function<void()>>  vDestroyCallbacks_;

    VkDescriptorPool                    descriptorPool_{ nullptr };

    TouchObject<TEVulkanContext>        teContext_;

    static const std::string ConfigureError;
    std::string getConfigureError() const;
    

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
        VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,
        VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
        VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME,
        VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME,

    };





    
    

};