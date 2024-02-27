#include "chop.h"

#include <iostream>
#include <algorithm>


const float* 
Chop::channel(const std::string& name)
{
	//auto it = nameChanMap_.find(name);
	//if (it != nameChanMap_.end()) return it->second.channel;
	return nullptr;
}

void 
Chop::set(const std::vector<float>& channels, uint32_t valueCount, double rate)
{
	int32_t channelCount = static_cast<int32_t>(channels.size());
	if (channelCount == 0) return;

	chanDataPtrs_.resize(channelCount);

	for (size_t chan = 0; chan < channelCount_; ++chan)
	{
		chanDataPtrs_[chan] = &channels[chan * valueCount_];
	}

	set(chanDataPtrs_.data(), channelCount, valueCount, rate);
}

void 
Chop::set(const std::vector<float>& channels, uint32_t valueCount, double rate, const std::vector<std::string>& names)
{
	int32_t channelCount = static_cast<int32_t>(channels.size());
	if (channelCount == 0) return;

	if (names.size() != channelCount) set(channels, valueCount, rate); // or return or throw() ?

	chanDataPtrs_.resize(channelCount);
	namePtrs_.resize(channelCount);

	for (size_t chan = 0; chan < channelCount_; ++chan)
	{
		chanDataPtrs_[chan] = &channels[chan * valueCount_];
		namePtrs_[chan] = names[chan].c_str();
	}

	set(chanDataPtrs_.data(), channelCount, valueCount, rate, namePtrs_.data());
}

void 
Chop::set(const std::vector<std::vector<float>>& channels, double rate)
{
	int32_t channelCount = static_cast<int32_t>(channels.size());
	if (channelCount == 0) return;
	if (channels[0].size() == 0) return;


	uint32_t valueCount = static_cast<uint32_t>(channels[0].size());
	chanDataPtrs_.resize(channelCount);

	for (size_t chan = 0; chan < channelCount_; ++chan)
	{
		chanDataPtrs_[chan] = channels[chan].data();
	}

	set(chanDataPtrs_.data(), channelCount, valueCount, rate);
}

void 
Chop::set(const std::vector<std::vector<float>>& channels, double rate, const std::vector<std::string>& names)
{
	int32_t channelCount = static_cast<int32_t>(channels.size());
	if (channelCount == 0) return;
	if (channels[0].size() == 0) return;

	if (names.size() != channels.size()) set(channels, rate); // or return or throw() ?

	uint32_t valueCount = static_cast<uint32_t>(channels[0].size());
	chanDataPtrs_.resize(channelCount);
	namePtrs_.resize(channelCount);

	for (size_t chan = 0; chan < channelCount_; ++chan)
	{
		chanDataPtrs_[chan] = channels[chan].data();
		namePtrs_[chan] = names[chan].c_str();
	}

	set(chanDataPtrs_.data(), channelCount, valueCount, rate, namePtrs_.data());
}

void 
Chop::set(const float** values, int32_t channelCount, uint32_t valueCount, double rate, const char** names)
{
	if (!values || channelCount <= 0 || valueCount == 0) return;

	channelCount_ = channelCount;
	capacity_ = valueCount;
	valueCount_ = valueCount;
	rate_ = rate;

	if (channelCount > 0)
	{
		TouchObject<TEFloatBuffer> buffer;
		TEResult result = TEInstanceLinkGetFloatBufferValue(instance_, identifier_.c_str(), TELinkValueCurrent, buffer.take());
		if (result == TEResultSuccess)
		{
			if (buffer && !bufferCopyable(buffer))
			{
				buffer.reset();
			}
			if (buffer)
			{
				TouchObject <TEFloatBuffer> newBuffer;
				newBuffer.take(TEFloatBufferCreateCopy(buffer));
				buffer = newBuffer;
			}
			else
			{
				buffer.take(TEFloatBufferCreate(rate_, channelCount_, capacity_, names));
				//buffer.take(TEFloatBufferCreate(rate_, channelCount_, capacity_, nullptr));
			}
			TEResult result = TEFloatBufferSetValues(buffer, values, valueCount_);
			result = TEInstanceLinkSetFloatBufferValue(instance_, identifier_.c_str(), buffer);
		}
		else
		{
			std::cout << "TEInstanceLinkGetFloatBufferValue Result: " << TEResultGetDescription(result) << std::endl;
		}
	}
}

bool
Chop::bufferCopyable(TouchObject<TEFloatBuffer> buffer, bool checkNames) const
{
	auto newChannelCount = TEFloatBufferGetChannelCount(buffer);
	if (newChannelCount != channelCount_
		|| TEFloatBufferGetCapacity(buffer) == capacity_
		|| TEFloatBufferGetValueCount(buffer) == valueCount_
		|| TEFloatBufferGetRate(buffer) == rate_) return false;

	//if (checkNames)
	//{
	//	const char* const* names = TEFloatBufferGetChannelNames(buffer);
	//	if (names)
	//	{
	//		for (int32_t chan = 0; chan < newChannelCount; ++chan)
	//		{
	//			if (names_[chan] != names[chan]) return false;
	//		}
	//	}
	//}

	return	true;
}


void 
Chop::swapTeBuffers()
{
	activeTeBuffer_.fetch_xor(1, std::memory_order_release);
}

void
Chop::updateTeBuffer()
{

	int bufferIndex = activeTeBuffer_.load(std::memory_order_acquire) ^ 1;
	TouchObject<TEFloatBuffer>& buffer = teBuffers_[bufferIndex];

	if (TEInstanceLinkGetFloatBufferValue(instance_, identifier_.c_str(), TELinkValueCurrent, buffer.take()) == TEResultSuccess)
	{
		if (buffer)
		{
			channelCount_ = TEFloatBufferGetChannelCount(buffer);
			capacity_ = TEFloatBufferGetCapacity(buffer);
			valueCount_ = TEFloatBufferGetValueCount(buffer);
			rate_ = TEFloatBufferGetRate(buffer);

			// Lock the mutex as short as possible
			{
				std::lock_guard<std::mutex> lock(mutex_);
				ready_ = true; // Mark as ready for reading
				cv_.notify_one(); // Notify the reading thread
				
			}
		}
	}
	

}

void Chop::readTeBuffer()
{
	std::unique_lock<std::mutex> lock(mutex_);
	cv_.wait(lock, [this] { return ready_; }); // Wait until data is ready
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
			std::copy(data[chan], data[chan] + capacity_, channelData_.begin() + chan * capacity_);

			chanDataPtrs_[chan] = &channelData_[chan * valueCount_];
			names_[chan] = names[chan];
			namePtrs_[chan] = names_[chan].c_str();
		}

		ready_ = false; // Reset ready state after reading
	}

	
}
