#pragma once

#include <TouchEngine/TouchEngine.h>
#include "links.h"
#include "chopchannels.h"

#include <memory> 
#include <vector>
#include <array>
#include <string>
#include <map>

#include <atomic>
#include <condition_variable>
#include <mutex>


class ChopLink : public Link<ChopLink>
{
public:
	ChopLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : Link<ChopLink>(instance, linkInfo) { }
	~ChopLink() { }
};

class InChopLink : public ChopLink
{
public:
	InChopLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : ChopLink(instance, linkInfo) { }
	~InChopLink() { }

	void set(ChopChannelsReference&& channels);

private:
	ChopChannelsReference chopChannels_;

	bool bufferCopyable(TouchObject<TEFloatBuffer> buffer, const ChopChannelsReference& channels) const;
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
	OutChopLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo);
	~OutChopLink() { }

	void update();

	void writeBuffer();
	void moveBuffer();

	const float* data();
	const std::vector<std::string>& channelNames();
	ChopChannels& chopChannels() { return chopChannels_; }

	int32_t channelCount() const { return chopChannels_.channelCount_; }
	uint32_t capacity() const { return chopChannels_.capacity_; }
	uint32_t valueCount() const { return chopChannels_.valueCount_; }
	double rate () const { return chopChannels_.rate_; }
	bool isTimeDependent() const { return chopChannels_.isTimeDependent_; }



private:
	void setChannelsFromBuffer(ChopChannels& chopChannels, TouchObject<TEFloatBuffer>& buffer);

	void swapBuffers();
	std::atomic<int> activeBuffer__{ 0 }; // Index of the buffer that is ready for reading
	std::mutex mutex_;
	std::condition_variable cv_;
	bool bufferReadReady_{ false };

	ChopChannels chansBuffers_[2];
	ChopChannels chopChannels_;
};


class OutChopLinks : public Links<OutChopLinks, OutChopLink>
{
public:
	OutChopLinks() = default;
	OutChopLinks(TouchObject<TEInstance> instance) : Links<OutChopLinks, OutChopLink>(instance) { }
	~OutChopLinks() { };
};
