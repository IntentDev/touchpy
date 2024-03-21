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


class ChopLink;
class InChopLink;
class OutChopLink;

class ChopChannels
{
	friend class ChopLink;
	friend class InChopLink;
	friend class OutChopLink;

public:
	ChopChannels() = default;
	ChopChannels(int32_t channelCount, uint32_t valueCount) : channelCount_(channelCount), valueCount_(valueCount) { }
	~ChopChannels() { }

	void set(const std::vector<float>& channels, uint32_t valueCount);
	void set(const std::vector<std::vector<float>>& channels);
	void set(const float** values, int32_t channelCount, uint32_t valueCount);

	const float* channel(const std::string& name);
	const float* operator[](const std::string& name) { return channel(name); }
	const float* operator[](uint32_t index)
	{
		if (index < channelCount_)
			return chanDataPtrs_[index];
		return nullptr;
	}

	const float* data() const { return channelData_.data(); }
	const float* const* channels() const { return chanDataPtrs_.data(); }
	const char* const* names() const { return namePtrs_.data(); }

	const std::vector<float>& channelData() const { return channelData_; }
	const std::vector<std::string>& channelNames() const { return names_; }
	

	int32_t channelCount() const { return channelCount_; }
	uint32_t capacity() const { return capacity_; }
	uint32_t valueCount() const { return valueCount_; }
	double rate() const { return rate_; }
	bool isTimeDependent() const { return isTimeDependent_; }

private:
	int32_t channelCount_ { 0 };
	uint32_t capacity_ { 0 };
	uint32_t valueCount_ { 0 };
	double rate_ { -1.0 };
	bool isTimeDependent_ { false };

	std::vector<float> channelData_;
	std::vector<std::string> names_;
	std::vector<const float*> chanDataPtrs_;
	std::vector<const char*> namePtrs_;


};



class ChopLink : public Link<ChopLink>
{
public:
	ChopLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : Link<ChopLink>(instance, linkInfo) { }
	~ChopLink() { }

	int32_t channelCount() const { return channelCount_; }
	uint32_t capacity() const { return capacity_; }
	uint32_t valueCount() const { return valueCount_; }
	double rate() const { return rate_; }
	bool isTimeDependent() const { return isTimeDependent_; }

protected:
	double rate_ { -1.0 };
	int32_t channelCount_ { 0 };
	uint32_t capacity_ { 0 }; // capacity of the TEFloatBuffer not the number of values per channel in channelData_
	uint32_t valueCount_ { 0 };
	bool isTimeDependent_ { false };

	std::vector<const float*> chanDataPtrs_;
	std::vector<const char*> namePtrs_;
};

class InChopLink : public ChopLink
{
public:
	InChopLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : ChopLink(instance, linkInfo) { }
	~InChopLink() { }

	void set(const float** values, int32_t channelCount, uint32_t valueCount, double rate = -1., const char** names = nullptr);

private:
	bool bufferCopyable(TouchObject<TEFloatBuffer> buffer, const char** names = nullptr) const;

};


class InChopLinks : public Links<InChopLinks, InChopLink>
{
public:
	InChopLinks() = default;
	InChopLinks(TouchObject<TEInstance> instance) : Links<InChopLinks, InChopLink>(instance) { }
	~InChopLinks() { };
};


class OutChopLink : public ChopLink
{
public:
	OutChopLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : ChopLink(instance, linkInfo) { }
	~OutChopLink() { }

	void onOuputValueChange();
	void swapTeBuffers();
	void updateTeBuffer();
	void copyTeBuffer();

	void setDoubleBuffered(bool doubleBuffered) { doubleBuffered_ = doubleBuffered; }
	bool doubleBuffered() const { return doubleBuffered_; }

	const bool isUpdated() const { return isUpdated_; }

	const float* data() const { return channelData_.data(); }
	const std::vector<float>& channelData() const { return channelData_; }
	const std::vector<const float*>& channelPtrs() const { return chanDataPtrs_; }
	const std::vector<std::string>& names() const { return names_; }

	//const float* channel(const std::string& name);
	//const float* operator[](const std::string& name) { return channel(name); }
	const float* operator[](uint32_t index)
	{
		if (index < channelCount_)
			return chanDataPtrs_[index];
		return nullptr;
	}

private:
	TouchObject<TEFloatBuffer> teBuffers_[2];
	std::atomic<int> activeTeBuffer_{ 0 }; // Index of the buffer that is ready for reading
	std::mutex mutex_;
	std::condition_variable cv_;
	bool doubleBuffered_{ false };
	bool teBufferReadReady_{ false };
	bool isUpdated_{ false };

	std::vector<float> channelData_;
	std::vector<std::string> names_;

};


class OutChopLinks : public Links<OutChopLinks, OutChopLink>
{
public:
	OutChopLinks() = default;
	OutChopLinks(TouchObject<TEInstance> instance) : Links<OutChopLinks, OutChopLink>(instance) { }
	~OutChopLinks() { };
};
