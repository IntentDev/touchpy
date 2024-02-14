#pragma once

#include <TouchEngine/TouchEngine.h>
#include "vkcontext.h"
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
	void configured(TEResult result);
	bool loaded() { std::lock_guard<std::mutex> lock(mutex_); return loaded_ = true; }
	void update();


private:
	std::mutex   mutex_;
	bool 		 pendingLayoutChange_ { false };
	bool         loaded_ { false };
	std::string  filePath_;
	bool	     configured_ { false };	
	TEResult     configuredResult_ { TEResultSuccess };
	bool 		 inFrame_ { false };

	TouchObject<TEInstance>  instance_ { nullptr };
	double       inputSampleRate_ { 60.0 };
	int32_t      inputChannelCount_ { 0 };

	int64_t      framesPerSecond_ { 60 };

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

	void linkLayoutDidChange(TELinkEvent event, const char* identifier);
	void linkValueChange(const char* identifier);
	void endFrame(int64_t start_time_value, int32_t start_time_scale, TEResult result);
	void getState(bool& configured, bool& loaded, bool& linksChanged, bool& inFrame);
	void setInFrame(bool inFrame);


	std::unique_ptr<VkContext>	vkContext_;
	std::unique_ptr<Texture>	texFromTE_;
	std::unique_ptr<Texture>	texToTE_;

	void createVkContext();



};
