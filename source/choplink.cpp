#include "choplink.h"

#include <iostream>
#include <algorithm>

void InChopLink::set(ChopChannelsReference&& chopChannels)
{

	if (chopChannels.channelCount_ <= 0 || chopChannels.capacity_ == 0 || chopChannels.valueCount_ == 0) return;

	chopChannels_ = std::move(chopChannels);

	TouchObject<TEFloatBuffer> buffer;
	TEResult result = TEInstanceLinkGetFloatBufferValue(instance_, identifier_.c_str(), TELinkValueCurrent, buffer.take());
	if (result == TEResultSuccess)
	{
		if (buffer && !bufferCopyable(buffer, chopChannels_))
			buffer.reset();

		if (buffer)
		{
			TouchObject <TEFloatBuffer> newBuffer;
			newBuffer.take(TEFloatBufferCreateCopy(buffer));
			buffer = newBuffer;
		}
		else
			buffer.take(TEFloatBufferCreate(
				chopChannels_.rate_,
				chopChannels_.channelCount_,
				chopChannels_.capacity_,
				chopChannels_.names_.data()));

		result = TEFloatBufferSetValues(buffer, chopChannels_.channels_.data(), chopChannels_.valueCount_);
		if (result == TEResultSuccess)
			result = TEInstanceLinkSetFloatBufferValue(instance_, identifier_.c_str(), buffer);
	}

	if (result != TEResultSuccess)
	{
		std::cout << "Error setting float buffer value: " << TEResultGetDescription(result) << std::endl;
	}
}

bool InChopLink::bufferCopyable(TouchObject<TEFloatBuffer> buffer, const ChopChannelsReference& chopChannels) const
{
	auto newChannelCount = TEFloatBufferGetChannelCount(buffer);
	if (newChannelCount != chopChannels.channelCount()
		|| TEFloatBufferGetCapacity(buffer) != chopChannels.capacity()
		|| TEFloatBufferGetValueCount(buffer) != chopChannels.valueCount()
		|| TEFloatBufferGetRate(buffer) != chopChannels.rate()
		|| TEFloatBufferIsTimeDependent(buffer) != chopChannels.isTimeDependent()) return false;

	if (chopChannels.names())
	{
		const char* const* newNames = TEFloatBufferGetChannelNames(buffer);
		if (newNames)
		{
			for (int32_t chan = 0; chan < newChannelCount; ++chan)
			{
				if (strcmp(newNames[chan], chopChannels.names()[chan]) != 0) return false;
			}
		}
	}

	return true;
}


OutChopLink::OutChopLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo, bool doubleBuffered)
	:	ChopLink(instance, linkInfo),
		doubleBuffered_(doubleBuffered) { }

OutChopLink::OutChopLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) 
	:	ChopLink(instance, linkInfo) { }


void OutChopLink::setChannelsFromBuffer(TouchObject<TEFloatBuffer>& buffer)
{
	auto channelCount = TEFloatBufferGetChannelCount(buffer);
	auto capacity = TEFloatBufferGetCapacity(buffer);
	auto valueCount = TEFloatBufferGetValueCount(buffer);
	auto rate = TEFloatBufferGetRate(buffer);
	auto isTimeDependent = TEFloatBufferIsTimeDependent(buffer);

	const float* const* data = TEFloatBufferGetValues(buffer);
	const char* const* names = TEFloatBufferGetChannelNames(buffer);

	chopChannels_.setChannels(data, channelCount, capacity, valueCount, rate, isTimeDependent, names);
}

void
OutChopLink::update()
{
	TouchObject<TEFloatBuffer> buffer;
	if (TEInstanceLinkGetFloatBufferValue(instance_, identifier_.c_str(), TELinkValueCurrent, buffer.take()) == TEResultSuccess)
	{
		if (buffer)
		{
			setChannelsFromBuffer(buffer);
			updated_ = true;
		}
	}
}

const float* OutChopLink::data() 
{ 
	if (!updated_) update();
	return chopChannels_.data();
}

const std::vector<std::string>& OutChopLink::channelNames()
{ 
	if (!updated_) update();
	return chopChannels_.channelNames();
}



void
OutChopLink::swapTeBuffers()
{
	activeTeBuffer_.fetch_xor(1, std::memory_order_release);
}

void
OutChopLink::updateTeBuffer()
{
	int nextBufferIndex = activeTeBuffer_.load(std::memory_order_acquire) ^ 1;
	TouchObject<TEFloatBuffer>& buffer = teBuffers_[nextBufferIndex];

	if (TEInstanceLinkGetFloatBufferValue(instance_, identifier_.c_str(), TELinkValueCurrent, buffer.take()) == TEResultSuccess)
	{
		if (buffer)
		{
			{
				std::lock_guard<std::mutex> lock(mutex_);
				teBufferReadReady_ = true; // Mark as ready for reading
				cv_.notify_one(); // Notify the reading thread

			}
		}
	}
}

void 
OutChopLink::copyTeBuffer()
{
	std::unique_lock<std::mutex> lock(mutex_);
	cv_.wait(lock, [this] { return teBufferReadReady_; }); // Wait until data is ready
	int bufferIndex = activeTeBuffer_.load(std::memory_order_acquire);

	TouchObject<TEFloatBuffer>& buffer = teBuffers_[bufferIndex];

	if (buffer)
	{
		setChannelsFromBuffer(buffer);
		updated_ = true;
		teBufferReadReady_ = false; // Reset ready state after reading
	}
}

