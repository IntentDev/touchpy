#pragma once

#include <TouchEngine/TouchEngine.h>
#include "renderer.h"
#include "texture.h"

#include <string>
#include <mutex>
#include <memory>

class TdTox
{
public:
	TdTox(std::string filePath);
	~TdTox();

	void load();
	void didConfigure(TEResult result);
	bool loaded() { std::lock_guard<std::mutex> lock(mutex_); return loaded_ = true; }
	void update();
	void render(bool loaded);


private:
	std::mutex   mutex_;
	bool 		 pendingLayoutChange_ { false };
	bool         loaded_ { false };
	std::string  filePath_;
	bool	     configureRenderer_ { false };	
	TEResult     configureResult_ { TEResultSuccess };
	bool		 configureError_{ false };
	bool 		 inFrame_ { false };

	TouchObject<TEInstance>  instance_ { nullptr };
	double       inputSampleRate_ { 60.0 };
	int32_t      inputChannelCount_ { 0 };

	int64_t      framesPerSecond_ { 60 };

	std::unordered_map<std::string, size_t> outputLinkTextureMap_;
	std::vector<std::string>				pendingOutputTextures_;
	std::unordered_map<HANDLE, Texture>   outputTextures_;
	std::unordered_map<HANDLE, Texture>   inputTextures_;

	std::unique_ptr<Renderer>	renderer_;
	VkDevice device_ { VK_NULL_HANDLE };
	VkPhysicalDevice physicalDevice_ { VK_NULL_HANDLE };
	std::vector<uint32_t> queueFamilyIndices_;
	VkQueue queue_ { VK_NULL_HANDLE };
	VkCommandBuffer commandBuffer_ { VK_NULL_HANDLE };



	std::unique_ptr<Texture>	texFromTE_;
	std::unique_ptr<Texture>	texToTE_;

	bool srcInitialized_ { false };
	bool dstInitialized_ { false };


	static void	eventCallback(
		TEInstance* instance,
		TEEvent event,
		TEResult result,
		int64_t start_time_value,
		int32_t start_time_scale,
		int64_t end_time_value,
		int32_t end_time_scale,
		void* info);

	static void	linkEventCallback(
		TEInstance* instance, 
		TELinkEvent event, 
		const char* identifier,
		void* info);

	void applyLayoutChange();
	bool applyOutputTextureChange();

	void linkLayoutDidChange(TELinkEvent event, const char* identifier);
	void linkValueChange(const char* identifier);
	void endFrame(int64_t start_time_value, int32_t start_time_scale, TEResult result);
	void getState(bool& configured, bool& loaded, bool& linksChanged, bool& inFrame);
	void setInFrame(bool inFrame);




	void createRenderer();



};
