#pragma once

#include <TouchEngine/TouchEngine.h>

#include "renderer.h"
#include "texture.h"
#include "common/cuda_helpers.h"

#include "toplink.h"
#include "choplink.h"
#include "datlink.h"
#include "parlink.h"

#include "compflags.h"

#include <string>
#include <mutex>
#include <memory>
#include <functional>
#include <chrono>
#include <thread>
#include <atomic>


class Comp
{
public:
	Comp();
	Comp(
		const std::string& filePath, 
		CompFlags compFlags = CompFlagBits::InternalTimeAuto | CompFlagBits::CudaStreamDefault,
		int64_t fps = 60
	);

	~Comp();

	bool loadTox(const std::string& filePath, int64_t fps = 60);
	void unload();
	bool loaded() const; 

	void start();
	void stop();

	bool frameDidFinish();
	void applyValueChanges();
	bool startNextFrame(int64_t timeValue = 0, int32_t timeScale = 0);

	InTopLinks&        inputTopLinks()  { return *inTopLinks_; }
	OutTopLinks&       outputTopLinks() { return *outTopLinks_; }
	InChopLinks&       inChopLinks()    { return *inChopLinks_; }
	OutChopLinks&      outChopLinks()   { return *outChopLinks_; }
	InDatLinks&        inDatLinks()     { return *inDatLinks_; }
	OutDatLinks&       outDatLinks()    { return *outDatLinks_; }
	ParLinkCollection& parLinks()       { return *parLinks_; }

	void setOnFrameCallback(std::function<void(Comp&, std::shared_ptr<void>)> callback, std::shared_ptr<void> userData);
	void clearOnFrameCallback();
	bool callOnFrameCallback();

	void setOnLayoutChangeCallback(std::function<void(Comp&, std::shared_ptr<void>)> callback, std::shared_ptr<void> userData);
	void clearOnLayoutChangeCallback();
	bool callOnLayoutChangeCallback();

	struct Time
	{
		float rate { 0.0f };
		int64_t frame { 0 };
		double seconds { 0.0 };
		int64_t value { 0 };
		int32_t scale { 0 };
	};

	cudaStream_t cudaStream() const { return cudaStream_; }

	Time time() const;
	float frameRate() const;

	// for internal use only, not for python bindings
	//-----------------------------------------------------------------------------------------------------------------
	bool asyncRunning() const { return asyncRunning_.load(); }

	//static void setPrintInfoFunc(std::function<void(std::string)> func) { printInfo = func; }

private:
	// shared state between the main or free running thread and the TouchEngine thread
	//-----------------------------------------------------------------------------------------------------------------

	mutable                  std::mutex mutex_;
	std::condition_variable  cv_;
	bool                     ssPendingLayoutChange_      { false };
	bool                     ssLoaded_                   { false };
	bool                     ssUnloading_                { false };
	bool                     ssReady_                    { false };
	bool                     ssInFrame_                  { false };
	Time					 ssTime						 { };
	std::vector<std::string> ssPendingOutputTextures_;
	std::vector<std::string> ssPendingOutputFloatBuffers;
	std::vector<std::string> ssPendingOutputStringData;

	void getState(bool& configured, bool& loaded, bool& linksChanged, bool& inFrame);
	void setInFrame(bool inFrame, bool setTime = false, int64_t timeValue = 0, int32_t timeScale = 0);

	// free running 
	//-----------------------------------------------------------------------------------------------------------------
	bool								  asyncActive_{ false }; 
	std::atomic<bool>					  asyncRunning_ { false };
	std::thread							  asyncThread_;
	std::mutex							  asyncMutex_;
	std::condition_variable 			  asyncCV_;
	bool							      asyncSettingCallback_ { false };
	std::condition_variable 			  asyncStopCV_;
	bool								  asyncContinueStop_ { false };
	std::condition_variable 			  asyncLayoutReadyCV_;
	bool								  asyncLayoutReady_ { false };
	void								  asyncUpdate();
	void								  startAsync();
	void								  stopAsync();

	// main thread only
	//-----------------------------------------------------------------------------------------------------------------

	std::string               filePath_;
	CompFlags                 compFlags_         { CompFlagBits::InternalTimeAuto | CompFlagBits::CudaStreamDefault };
	TouchObject<TEInstance>   instance_          { nullptr };

	std::unique_ptr<Renderer> renderer_;
	VkDevice                  device_            { VK_NULL_HANDLE };
	VkPhysicalDevice          physicalDevice_    { VK_NULL_HANDLE };
	std::vector<uint32_t>     queueFamilyIndices_;
	VkQueue                   queue_             { VK_NULL_HANDLE };
	VkCommandBuffer           commandBuffer_     { VK_NULL_HANDLE };
	VkFence                   submitFence_       { VK_NULL_HANDLE };

	cudaStream_t              cudaStream_        { nullptr };
	int                       cudaDevice_        { -1 };


	std::vector<std::string>           changedOutputTextures_;
	std::vector<std::string>           changedOutputFloatBuffers_;
	std::vector<std::string>           changedOutputStringData_;

	std::unique_ptr<InTopLinks>        inTopLinks_;
	std::unique_ptr<OutTopLinks>       outTopLinks_;
	std::unique_ptr<InChopLinks>       inChopLinks_;
	std::unique_ptr<OutChopLinks>      outChopLinks_;
	std::unique_ptr<InDatLinks>        inDatLinks_;
	std::unique_ptr<OutDatLinks>       outDatLinks_;
	std::unique_ptr<ParLinkCollection> parLinks_;

	
	bool											  updateLoopRunning_	{ false };
	std::shared_ptr<void>							  onFrameCallbackUserData_ { nullptr };
	std::function<void(Comp&, std::shared_ptr<void>)> onFrameCallback_ { nullptr };
	std::shared_ptr<void>							  onLayoutChangeCallbackUserData_{ nullptr };
	std::function<void(Comp&, std::shared_ptr<void>)> onLayoutChangeCallback_{ nullptr };

	//static std::function<void(std::string)> printInfo;

	void initComp();
	bool initInstance();
	bool loadTox(const std::string& filePath, CompFlags compFlags = CompFlagBits::InternalTimeAuto, int64_t fps = 60);
	void autoUpdate();
	void stopUpdate();
	void applyLayoutChange();
	void applyOutputTextureChange();
	void applyOutputFloatBufferChange();
	void applyOutputStringDataChange();

	void createRenderer();
	void cudaInit();
	void setCudaDevice();

	// TouchEngine thread only
	//-----------------------------------------------------------------------------------------------------------------

	static void	eventCallback(
		TEInstance* instance,
		TEEvent		event,
		TEResult    result,
		int64_t     start_time_value,
		int32_t     start_time_scale,
		int64_t     end_time_value,
		int32_t     end_time_scale,
		void*       info);

	void onEventInstanceReady(TEResult result, Comp* comp);
	void onEventInstanceDidLoad(TEResult result, Comp* comp);
	void onEventInstanceDidUnload(TEResult result, Comp* comp);
	void onEventFrameDidFinish(TEResult result, int64_t start_time_value, int32_t start_time_scale, 
		int64_t end_time_value, int32_t end_time_scale, Comp* comp);
	void onEventGeneral(TEResult result, uint64_t start_time, uint64_t end_time, Comp* comp);

	static void	linkEventCallback(
		TEInstance* instance, 
		TELinkEvent event, 
		const char* identifier,
		void* info);

	void onLinkLayoutChange(TELinkEvent event, const char* identifier);
	void onLinkEventValueChange(const char* identifier);

	void onLinkEventAdded(const char* identifier)       { onLinkLayoutChange(TELinkEventAdded, identifier); }
	void onLinkEventRemoved(const char* identifier)     { onLinkLayoutChange(TELinkEventRemoved, identifier); }
	void onLinkEventModified(const char* identifier)    { onLinkLayoutChange(TELinkEventModified, identifier); }
	void onLinkEventMoved(const char* identifier)       { onLinkLayoutChange(TELinkEventMoved, identifier); }
	void onLinkEventStateChange(const char* identifier) { onLinkLayoutChange(TELinkEventStateChange, identifier); }
	void onLinkEventChildChange(const char* identifier) { onLinkLayoutChange(TELinkEventChildChange, identifier); }    

	std::string getLinkInfoAsString(TouchObject<TELinkInfo> info);


};
