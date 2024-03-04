#pragma once

#include <TouchEngine/TouchEngine.h>
#include "links.h"

#include <memory> 
#include <vector>
#include <string>
#include <map>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>



class ChopLink : public Link<ChopLink>
{
public:
	ChopLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo);
	~ChopLink();;

	void onOuputValueChange();
	void swapTeBuffers();
	void updateTeBuffer();
	void readTeBuffer();

	const bool isUpdated() const { return isUpdated_; }

	const std::vector<float>& channelData() const { return channelData_; }
	const std::vector<const float*>& channelPtrs() const { return chanDataPtrs_; }
	const std::vector<std::string>& names() const { return names_; }
	int32_t channelCount() const { return channelCount_; }
	uint32_t capacity() const { return capacity_; }
	uint32_t valueCount() const { return valueCount_; }
	double rate() const { return rate_; }
	bool isTimeDependent() const { return isTimeDependent_; }


	//const float* channel(const std::string& name);
	//const float* operator[](const std::string& name) { return channel(name); }
	const float* operator[](uint32_t index)
	{
		if (index < channelCount_)
			return chanDataPtrs_[index];
		return nullptr;
	}

	// use setFrom[type] naming for Python bindings
	void set(const std::vector<float>& channels, uint32_t valueCount, double rate = -1.0);
	void set(const std::vector<float>& channels, uint32_t valueCount, double rate, const std::vector<std::string>& names);
	void set(const std::vector<std::vector<float>>& channels, double rate = -1.0);
	void set(const std::vector<std::vector<float>>& channels, double rate, const std::vector<std::string>& names);
	void set(const float** values, int32_t channelCount, uint32_t valueCount, double rate, const char** names = nullptr);

		
private:
	double rate_ { -1.0 };
	int32_t channelCount_ { 0 };
	uint32_t capacity_ { 0 }; // capacity of the TEFloatBuffer not the number of values per channel in channelData_
	uint32_t valueCount_ { 0 };
	bool isTimeDependent_ { false };



	// output only
	//-----------------------------------------------------------------------------------------------------------------
	//ChopDoubleBuffer doubleBuffer_; // output only
	TouchObject<TEFloatBuffer> teBuffer_; // output only

	TouchObject<TEFloatBuffer> teBuffers_[2]; 
	std::atomic<int> activeTeBuffer_{ 0 }; // Index of the buffer that is ready for reading
	std::mutex mutex_;
	std::condition_variable cv_;
	bool ready_{ false };

	std::vector<float> channelData_;
	std::vector<const float*> chanDataPtrs_;
	std::vector<std::string> names_;
	std::vector<const char*> namePtrs_;

	bool isUpdated_ { false };

	// input only
	//-----------------------------------------------------------------------------------------------------------------
	bool bufferCopyable(TouchObject<TEFloatBuffer> buffer, const char** names = nullptr) const;

};

class ChopLinks : public Links<ChopLinks, ChopLink>
{
public:
	ChopLinks() = default;
	ChopLinks(TouchObject<TEInstance> instance) : Links<ChopLinks, ChopLink>(instance) { }
	~ChopLinks() { };

	//using Links<ChopLinks, ChopLink>::begin;
	//using Links<ChopLinks, ChopLink>::end;
};
