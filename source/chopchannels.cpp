#include "chopchannels.h"

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
	bool isTimeDependent)
	:	channelCount_(channelCount), capacity_(capacity), valueCount_(valueCount), rate_(rate), isTimeDependent_(isTimeDependent)
{ }

ChopChannelsReference::ChopChannelsReference(
	std::vector<const float*>&& channels,
	int32_t channelCount,
	uint32_t capacity,
	uint32_t valueCount,
	double rate,
	bool isTimeDependent,
	std::vector<const char*>&& names)
	:	ChopChannelsBase(channelCount, capacity, valueCount, rate, isTimeDependent)
{
		channels_ = std::move(channels);
		names_ = std::move(names);
}

ChopChannels::ChopChannels(
	const float* const* data, 
	int32_t channelCount, 
	uint32_t capacity, 
	uint32_t valueCount, 
	double rate, 
	bool isTimeDependent,
	const char* const* names)
	:	ChopChannelsBase(channelCount, capacity, valueCount, rate, isTimeDependent)
{
	channelData_.resize(channelCount * capacity);
	channels_.resize(channelCount);

	if (names)
	{
		channelNames_.resize(channelCount);
		names_.resize(channelCount);
		for (int32_t i = 0; i < channelCount; ++i)
		{
			std::copy(data[i], data[i] + valueCount_, channelData_.begin() + i * valueCount_);
			channels_[i] = &channelData_[i * valueCount_];

			channelNames_[i] = names[i];
			names_[i] = channelNames_[i].c_str();
		}
	}
	else
	{
		for (int32_t i = 0; i < channelCount; ++i)
		{
			std::copy(data[i], data[i] + valueCount_, channelData_.begin() + i * valueCount_);
			channels_[i] = &channelData_[i * valueCount_];
		}
	}
}


