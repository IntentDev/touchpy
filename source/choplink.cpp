#include "choplink.h"

#include <iostream>
#include <algorithm>


void
OutChopLink::onOuputValueChange()
{
	TouchObject<TEFloatBuffer> buffer;
	if (TEInstanceLinkGetFloatBufferValue(instance_, identifier_.c_str(), TELinkValueCurrent, buffer.take()) == TEResultSuccess)
	{
		if (buffer)
		{
			channelCount_ = TEFloatBufferGetChannelCount(buffer);
			capacity_ = TEFloatBufferGetCapacity(buffer);
			valueCount_ = TEFloatBufferGetValueCount(buffer);
			rate_ = TEFloatBufferGetRate(buffer);
			isTimeDependent_ = TEFloatBufferIsTimeDependent(buffer);

			//std::cout << "Update: " << identifier << " Channel count: " << channelCount_ 
			// << ", Capacity: " << capacity_ << ", Value count: " << valueCount_ << ", Rate: " << rate_ 
			// << ", Time dependent: " << isTimeDependent_ << std::endl;

			const float* const* data = TEFloatBufferGetValues(buffer);
			const char* const* names = TEFloatBufferGetChannelNames(buffer);

			//size_t arraySize = static_cast<size_t>(channelCount_ * capacity_);
			size_t arraySize = static_cast<size_t>(channelCount_ * valueCount_); 
			channelData_.resize(arraySize);
			names_.resize(channelCount_);
			chanDataPtrs_.resize(channelCount_);
			namePtrs_.resize(channelCount_);

			for (size_t chan = 0; chan < channelCount_; ++chan)
			{
				//std::copy(data[chan], data[chan] + valueCount_, channelData_.begin() + chan * capacity_);
				std::copy(data[chan], data[chan] + valueCount_, channelData_.begin() + chan * valueCount_);

				//chanDataPtrs_[chan] = &channelData_[chan * capacity_];
				chanDataPtrs_[chan] = &channelData_[chan * valueCount_];
				names_[chan] = names[chan];
				namePtrs_[chan] = names_[chan].c_str();
			}
			isUpdated_ = true;
	
		}
		else
			isUpdated_ = false;
	}
	else
		isUpdated_ = false;
	
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
			channelCount_ = TEFloatBufferGetChannelCount(buffer);
			capacity_ = TEFloatBufferGetCapacity(buffer);
			valueCount_ = TEFloatBufferGetValueCount(buffer);
			rate_ = TEFloatBufferGetRate(buffer);
			isTimeDependent_ = TEFloatBufferIsTimeDependent(buffer);

			// Lock the mutex as short as possible
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

	if (teBuffers_[bufferIndex])
	{
		const float* const* data = TEFloatBufferGetValues(teBuffers_[bufferIndex]);
		const char* const* names = TEFloatBufferGetChannelNames(teBuffers_[bufferIndex]);

		size_t arraySize = static_cast<size_t>(channelCount_ * valueCount_);
		channelData_.resize(arraySize);
		names_.resize(channelCount_);
		chanDataPtrs_.resize(channelCount_);
		namePtrs_.resize(channelCount_);

		for (size_t chan = 0; chan < channelCount_; ++chan)
		{
			std::copy(data[chan], data[chan] + valueCount_, channelData_.begin() + chan * valueCount_);

			chanDataPtrs_[chan] = &channelData_[chan * valueCount_];
			names_[chan] = names[chan];
			namePtrs_[chan] = names_[chan].c_str();
		}

		teBufferReadReady_ = false; // Reset ready state after reading
	}
}

void 
InChopLink::set(const float** values, int32_t channelCount, uint32_t valueCount, double rate, const char** names)
{
	if (!values || channelCount <= 0 || valueCount == 0) return;

	channelCount_ = channelCount;
	capacity_ = valueCount;
	valueCount_ = valueCount;
	rate_ = rate;

	TouchObject<TEFloatBuffer> buffer;
	TEResult result = TEInstanceLinkGetFloatBufferValue(instance_, identifier_.c_str(), TELinkValueCurrent, buffer.take());
	if(result == TEResultSuccess)
	{
		if (buffer && !bufferCopyable(buffer, names))
			buffer.reset();

		if (buffer)
		{
			TouchObject <TEFloatBuffer> newBuffer;
			newBuffer.take(TEFloatBufferCreateCopy(buffer));
			buffer = newBuffer;
		}
		else
			buffer.take(TEFloatBufferCreate(rate_, channelCount_, capacity_, names));
		
		result = TEFloatBufferSetValues(buffer, values, valueCount_);
		if (result == TEResultSuccess)
			result = TEInstanceLinkSetFloatBufferValue(instance_, identifier_.c_str(), buffer);
	}

	if (result != TEResultSuccess)
	{
		std::cout << "Error setting float buffer value: " << TEResultGetDescription(result) << std::endl;
	}	
}

bool
InChopLink::bufferCopyable(TouchObject<TEFloatBuffer> buffer, const char** names) const
{
	auto newChannelCount = TEFloatBufferGetChannelCount(buffer);
	if (newChannelCount != channelCount_
		|| TEFloatBufferGetCapacity(buffer) != capacity_
		|| TEFloatBufferGetValueCount(buffer) != valueCount_
		|| TEFloatBufferGetRate(buffer) != rate_
		|| TEFloatBufferIsTimeDependent(buffer) != isTimeDependent_) return false;

	if (names)
	{
		const char* const* newNames = TEFloatBufferGetChannelNames(buffer);
		if (newNames)
		{
			for (int32_t chan = 0; chan < newChannelCount; ++chan)
			{
				if (strcmp(newNames[chan], names[chan]) != 0) return false;
			}
		}
	}

	return	true;
}





