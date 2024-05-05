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

	void set(ChopChannelsView&& chopChannels);

private:
	ChopChannelsView chopChannels_;

	bool bufferCopyable(TouchObject<TEFloatBuffer> buffer, const ChopChannelsView& channels) const;
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

	void writeBuffer();
	void moveBuffer();
	void setUsingSwapBuffer(bool usingSwapBuffer) { usingSwapBuffer_ = usingSwapBuffer; }

	const float* valuesArray();
	const std::vector<std::string>& channelNames();
	ChopChannels& chopChannels();

private:
	void update();
	void setChannelsFromBuffer(ChopChannels& chopChannels, TouchObject<TEFloatBuffer>& buffer);

	void swapBuffers();
	bool usingSwapBuffer_ { false };
	std::atomic<int> activeBuffer_{ 0 }; // Index of the buffer that is ready for reading
	std::mutex mutex_;
	std::condition_variable cv_;
	bool bufferMoveReady_{ false };

	std::vector<ChopChannels> swapBuffer_;
	ChopChannels chopChannels_;
};


class OutChopLinks : public Links<OutChopLinks, OutChopLink>
{
public:
	OutChopLinks() = default;
	OutChopLinks(TouchObject<TEInstance> instance) : Links<OutChopLinks, OutChopLink>(instance) { }
	~OutChopLinks() { };
};
