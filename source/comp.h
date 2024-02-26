#pragma once

#include <TouchEngine/TouchEngine.h>
#include "renderer.h"
#include "texture.h"
#include "common/cuda_helpers.h"
#include "par.h"
#include "chop.h"


#include <string>
#include <mutex>
#include <memory>
#include <chrono>


class Comp
{
public:
	Comp();
	Comp(const std::string& filePath);
	~Comp();

	void loadTox(const std::string& filePath);
	void unload();
	bool loaded() const; 
	bool ready() const { return ready_; }
	void update();

	ParCollection& pars() { return parCollection_; }

private:

	// shared state between the main thread and the TouchEngine thread
	//-----------------------------------------------------------------------------------------------------------------

	mutable std::mutex                      mutex_;
	bool                                    ssPendingLayoutChange_ { false };
	bool                                    ssLoaded_              { false };
	bool                                    ssReady_			   { false };
	bool                                    ssInFrame_             { false };

	void getState(bool& configured, bool& loaded, bool& linksChanged, bool& inFrame);
	void setInFrame(bool inFrame);

	

	// main thread only
	//-----------------------------------------------------------------------------------------------------------------

	std::string                             filePath_;
	size_t								    buffersPerInputLink  { 2 };
	TouchObject<TEInstance>                 instance_            { nullptr };
	bool 									ready_				 { false };
	double                                  inputSampleRate_     { 60.0 };
	int32_t                                 inputChannelCount_   { 0 };

	int64_t                                 framesPerSecond_     { 1 };

	std::vector<std::string>                pendingOutputTextures_;
	std::unordered_map<HANDLE, Texture>     outputTextures_;
	std::unordered_map<HANDLE, Texture>     inputTextures_;

	std::unique_ptr<Renderer>               renderer_;
	VkDevice                                device_              { VK_NULL_HANDLE };
	VkPhysicalDevice                        physicalDevice_      { VK_NULL_HANDLE };
	std::vector<uint32_t>                   queueFamilyIndices_;
	VkQueue                                 queue_               { VK_NULL_HANDLE };
	VkCommandBuffer                         commandBuffer_       { VK_NULL_HANDLE };

	std::unordered_map<HANDLE, std::unique_ptr<Texture>>                   texturesExternal_;
	std::unordered_map<std::string, std::vector<std::unique_ptr<Texture>>> texturesInternal_;

	std::unique_ptr<Texture>                texToTE_;

	VkFence                                 submitFence_         { VK_NULL_HANDLE };

	cudaStream_t                            cudaStream_          { nullptr };
	int										cudaDevice_ 		 { -1 };

	std::chrono::high_resolution_clock::time_point lastFrameTime_{};

	ParCollection 							parCollection_;
	ChopCollection							inputChops_;
	ChopCollection							outputChops_;


	void initComp();
	void load();

	void applyLayoutChange();
	bool applyOutputTextureChange();


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

	void onEventInstanceReady(TEResult result);
	void onEventInstanceDidLoad(TEResult result);
	void onEventInstanceDidUnload(TEResult result);
	void onEventFrameDidFinish(TEResult result, int64_t start_time_value, int32_t start_time_scale);
	void onEventGeneral(TEResult result, uint64_t start_time, uint64_t end_time);

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





};
