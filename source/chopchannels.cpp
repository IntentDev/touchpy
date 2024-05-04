#include "chopchannels.h"

#include <iostream>

const float* 
ChopChannelsBase::chan(const std::string& name)
{
	for (uint32_t i = 0; i < channelCount_; ++i)
	{
		if (strcmp(names_[i], name.c_str()) == 0)
			return channels_[i];
	}
	return nullptr;
}

const float* 
ChopChannelsBase::chan(uint32_t index)
{
	if (index < channelCount_)
		return channels_[index];
	return nullptr;
}

ChopChannelsBase::ChopChannelsBase(
	int32_t channelCount, 
	uint32_t capacity, 
	uint32_t valueCount, 
	double rate, 
	bool isTimeDependent,
	int64_t startTime,
	int64_t endTime)
	:	channelCount_(channelCount), 
		capacity_(capacity), 
		valueCount_(valueCount), 
		rate_(rate), 
		isTimeDependent_(isTimeDependent),
		startTime_(startTime),
		endTime_(endTime)
{ }

ChopChannels::ChopChannels(
	const float* const* data,
	int32_t channelCount,
	uint32_t capacity,
	uint32_t valueCount,
	double rate,
	bool isTimeDependent,
	int64_t startTime,
	int64_t endTime,
	const char* const* names)
{
	setChannels(data, channelCount, capacity, valueCount, rate, isTimeDependent, startTime, endTime, names);
}

void ChopChannels::setChannels(
	const float* const* data, 
	int32_t channelCount, 
	uint32_t capacity, 
	uint32_t valueCount, 
	double rate, 
	bool isTimeDependent,
	int64_t startTime,
	int64_t endTime,
	const char* const* names)

{
	channelCount_ = channelCount;
	capacity_ = capacity;
	valueCount_ = valueCount;
	rate_ = rate;
	isTimeDependent_ = isTimeDependent;
	startTime_ = startTime;
	endTime_ = endTime;
	channelsBuffer_.resize(channelCount * capacity);
	channels_.resize(channelCount);

	if (names)
	{
		namesBuffer_.resize(channelCount);
		names_.resize(channelCount);

		for (int32_t i = 0; i < channelCount; ++i)
		{
			std::copy(data[i], data[i] + valueCount_, channelsBuffer_.begin() + i * valueCount_);
			channels_[i] = &channelsBuffer_[i * capacity_];

			namesBuffer_[i] = names[i];
			names_[i] = namesBuffer_[i].c_str();
		}
	}
	else
	{
		for (int32_t i = 0; i < channelCount; ++i)
		{
			std::copy(data[i], data[i] + valueCount_, channelsBuffer_.begin() + i * valueCount_);
			channels_[i] = &channelsBuffer_[i * capacity_];
		}
	}
}

void
ChopChannels::setChannels(const float* data, int32_t channelCount, uint32_t valueCount, double rate, bool isTimeDependent, int64_t startTime, int64_t endTime, const std::vector<std::string>& names)
{
	channelCount_ = channelCount;
	capacity_ = valueCount;
	valueCount_ = valueCount;
	rate_ = rate;
	isTimeDependent_ = isTimeDependent;
	startTime_ = startTime;
	endTime_ = endTime;
	channelsBuffer_.resize(channelCount * valueCount);
	channels_.resize(channelCount);

	std::copy(data, data + channelCount * valueCount, channelsBuffer_.begin());


	if (names.size() > 0)
	{
		namesBuffer_.resize(channelCount);
		names_.resize(channelCount);

		for (int32_t i = 0; i < channelCount; ++i)
		{
			channels_[i] = &channelsBuffer_[i * capacity_];
			namesBuffer_[i] = names[i];
			names_[i] = namesBuffer_[i].c_str();
		}
	}
	else
	{
		for (int32_t i = 0; i < channelCount; ++i)
		{
			channels_[i] = &channelsBuffer_[i * capacity_];
		}
	}
}

ChopChannelsView::ChopChannelsView(
	std::vector<const float*>&& channels,
	int32_t channelCount,
	uint32_t capacity,
	uint32_t valueCount,
	double rate,
	bool isTimeDependent,
	std::vector<const char*>&& names)
	: ChopChannelsBase(channelCount, capacity, valueCount, rate, isTimeDependent)
{
	channels_ = std::move(channels);
	names_ = std::move(names);
}

ChopChannelsView::ChopChannelsView(ChopChannels& chopChannels)
{
	channelCount_ = chopChannels.channelCount_;
	capacity_ = chopChannels.valueCount_;
	valueCount_ = chopChannels.valueCount_;
	rate_ = chopChannels.rate_;
	isTimeDependent_ = chopChannels.isTimeDependent_;
	startTime_ = chopChannels.startTime_;
	endTime_ = chopChannels.endTime_;

	//std::cout
	//	<< "isTimeDependent: " << isTimeDependent_
	//	<< " channelCount: " << channelCount_
	//	<< " capacity: " << capacity_
	//	<< " valueCount: " << valueCount_
	//	<< " rate: " << rate_
	//	<< " startTime: " << startTime_
	//	<< " endTime: " << endTime_
	//	<< std::endl;

	channels_.resize(channelCount_);
	if (chopChannels.names_.size() > 0)
	{
		names_.resize(channelCount_);
		for (int32_t i = 0; i < channelCount_; ++i)
		{
			names_[i] = chopChannels.namesBuffer_[i].c_str();
			channels_[i] = chopChannels.valuesArray() + i * capacity_;
		}
	}
	else
	{
		for (int32_t i = 0; i < channelCount_; ++i)
		{
			channels_[i] = chopChannels.valuesArray() + i * capacity_;
		}
	}
}










