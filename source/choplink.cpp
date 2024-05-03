#include "choplink.h"
#include "logging.h"

#include <iostream>
#include <algorithm>

void 
InChopLink::set(ChopChannelsView&& chopChannels)
{

	if (chopChannels.channelCount_ <= 0 || chopChannels.capacity_ == 0 || chopChannels.valueCount_ == 0) return;

	chopChannels_ = std::move(chopChannels);

	//std::cout 
	//	<< "isTimeDependent: " << chopChannels_.isTimeDependent_
	//	<< " channelCount: " << chopChannels_.channelCount_
	//	<< " capacity: " << chopChannels_.capacity_
	//	<< " valueCount: " << chopChannels_.valueCount_
	//	<< " rate: " << chopChannels_.rate_
	//	<< " startTime: " << chopChannels_.startTime_
	//	<< " endTime: " << chopChannels_.endTime_
	//	<< std::endl;


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

			//buffer.take(TEFloatBufferCreate(
			//	chopChannels.rate_,
			//	chopChannels.channelCount_,
			//	chopChannels.capacity_,
			//	chopChannels.names_.data()));
			if (!chopChannels.isTimeDependent_)
				buffer.take(TEFloatBufferCreate(
					chopChannels.rate_,
					chopChannels.channelCount_,
					chopChannels.capacity_,
					chopChannels.names_.data()));
			else
				buffer.take(TEFloatBufferCreateTimeDependent(
					chopChannels.rate_,
					chopChannels.channelCount_,
					chopChannels.capacity_,
					chopChannels.names_.data()));

		result = TEFloatBufferSetValues(buffer, chopChannels_.channels_.data(), chopChannels_.valueCount_);

		if (result == TEResultSuccess && chopChannels.isTimeDependent_)
			result = TEFloatBufferSetStartTime(buffer, chopChannels_.startTime_);
	

		if (result == TEResultSuccess)
			result = TEInstanceLinkSetFloatBufferValue(instance_, identifier_.c_str(), buffer);
	}

	if (result != TEResultSuccess)
	{
		spdlog::error("Error setting float buffer value: {}", TEResultGetDescription(result));
	}
}


bool 
InChopLink::bufferCopyable(TouchObject<TEFloatBuffer> buffer, const ChopChannelsView& chopChannels) const
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

OutChopLink::OutChopLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) 
	:	ChopLink(instance, linkInfo) { }


void 
OutChopLink::setChannelsFromBuffer(ChopChannels& chopChannels, TouchObject<TEFloatBuffer>& buffer)
{
	auto channelCount = TEFloatBufferGetChannelCount(buffer);
	auto capacity = TEFloatBufferGetCapacity(buffer);
	auto valueCount = TEFloatBufferGetValueCount(buffer);
	auto rate = TEFloatBufferGetRate(buffer);
	auto isTimeDependent = TEFloatBufferIsTimeDependent(buffer);
	auto startTime = TEFloatBufferGetStartTime(buffer);
	auto endTime = TEFloatBufferGetEndTime(buffer);

	const float* const* data = TEFloatBufferGetValues(buffer);
	const char* const* names = TEFloatBufferGetChannelNames(buffer);

	chopChannels.setChannels(data, channelCount, capacity, valueCount, rate, isTimeDependent, startTime, endTime, names);
}

void
OutChopLink::update()
{
	TouchObject<TEFloatBuffer> buffer;
	if (TEInstanceLinkGetFloatBufferValue(instance_, identifier_.c_str(), TELinkValueCurrent, buffer.take()) == TEResultSuccess)
	{
		if (buffer)
		{
			setChannelsFromBuffer(chopChannels_, buffer);
			updated_ = true;
		}
	}
}

const float* 
OutChopLink::valuesArray()
{ 
	if (!usingSwapBuffer_)
	{
		if (!updated_) update();
		return chopChannels_.valuesArray();
	}
	else
	{
		std::unique_lock<std::mutex> lock(mutex_);
		return chopChannels_.valuesArray();
	}
}

const std::vector<std::string>& 
OutChopLink::channelNames()
{ 
	if (!usingSwapBuffer_)
	{
		if (!updated_) update();
		return chopChannels_.namesBuffer();
	}
	else
	{
		std::unique_lock<std::mutex> lock(mutex_);
		return chopChannels_.namesBuffer();
	}
}

ChopChannels& 
OutChopLink::chopChannels() 
{ 
	if (!usingSwapBuffer_) 
	{
		if (!updated_) update();
		return chopChannels_;
	}

	std::unique_lock<std::mutex> lock(mutex_);
	return chopChannels_; 
}


void
OutChopLink::swapBuffers()
{
	activeBuffer_.fetch_xor(1, std::memory_order_release);
}

void
OutChopLink::writeBuffer()
{
	TouchObject<TEFloatBuffer> teBuffer;

	if (TEInstanceLinkGetFloatBufferValue(instance_, identifier_.c_str(), TELinkValueCurrent, teBuffer.take()) == TEResultSuccess)
	{
		if (teBuffer)
		{
			if (swapBuffer_.size() != 2) swapBuffer_.resize(2);

			int nextBufferIndex = activeBuffer_.load(std::memory_order_acquire) ^ 1;
			setChannelsFromBuffer(swapBuffer_[nextBufferIndex], teBuffer);
			{
				std::lock_guard<std::mutex> lock(mutex_);
				bufferMoveReady_ = true;
				cv_.notify_one();
			}
			swapBuffers();
			updated_ = true;
		}
	}
}

void
OutChopLink::moveBuffer()
{
	std::unique_lock<std::mutex> lock(mutex_);
	cv_.wait(lock, [this] { return bufferMoveReady_; }); // Wait until data is ready
	int bufferIndex = activeBuffer_.load(std::memory_order_acquire);

	chopChannels_ = std::move(swapBuffer_[bufferIndex]);
	bufferMoveReady_ = false;
}

