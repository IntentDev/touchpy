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
			channels_[i] = &channelData_[i * capacity_];

			channelNames_[i] = names[i];
			names_[i] = channelNames_[i].c_str();
		}
	}
	else
	{
		for (int32_t i = 0; i < channelCount; ++i)
		{
			std::copy(data[i], data[i] + valueCount_, channelData_.begin() + i * valueCount_);
			channels_[i] = &channelData_[i * capacity_];
		}
	}
}

void ChopChannels::setChannels(
	const float* const* data, 
	int32_t channelCount, 
	uint32_t capacity, 
	uint32_t valueCount, 
	double rate, 
	bool isTimeDependent, 
	const char* const* names)
{
	channelCount_ = channelCount;
	capacity_ = capacity;
	valueCount_ = valueCount;
	rate_ = rate;
	isTimeDependent_ = isTimeDependent;
	channelData_.resize(channelCount * capacity);
	channels_.resize(channelCount);

	if (names)
	{
		channelNames_.resize(channelCount);
		names_.resize(channelCount);

		for (int32_t i = 0; i < channelCount; ++i)
		{
			std::copy(data[i], data[i] + valueCount_, channelData_.begin() + i * valueCount_);
			channels_[i] = &channelData_[i * capacity_];

			channelNames_[i] = names[i];
			names_[i] = channelNames_[i].c_str();
		}
	}
	else
	{
		for (int32_t i = 0; i < channelCount; ++i)
		{
			std::copy(data[i], data[i] + valueCount_, channelData_.begin() + i * valueCount_);
			channels_[i] = &channelData_[i * capacity_];
		}
	}
}

//void ChopChannels::setCapacity(uint32_t capacity)
//{
//	capacity_ = capacity;
//	channelData_.resize(channelCount_ * capacity_);
//	valueCount_ = std::min(valueCount_, capacity_);
//}
//
//void ChopChannels::setRate(double rate)
//{
//	rate_ = rate;
//}
//
//void ChopChannels::setIsTimeDependent(bool isTimeDependent)
//{
//	isTimeDependent_ = isTimeDependent;
//}
//
//void ChopChannels::appendChannel(const float* data, uint32_t size, const char* name)
//{
//	if (channelCount_ == 0)
//	{
//		channelCount_ = 1;
//		capacity_ = size;
//		valueCount_ = size;
//		channelData_.resize(size);
//		channels_.resize(1);
//		channelNames_.resize(1);
//		names_.resize(1);
//		std::copy(data, data + size, channelData_.begin());
//		channels_[0] = &channelData_[0];
//		channelNames_[0] = name;
//		names_[0] = channelNames_[0].c_str();
//	}
//	else
//	{
//		++channelCount_;
//		channelData_.resize(channelCount_ * capacity_);
//		channels_.resize(channelCount_);
//		channelNames_.resize(channelCount_);
//		names_.resize(channelCount_);
//
//		auto i = channelCount_ - 1;
//		std::copy(data, data + size, channelData_.begin() + i * std::min(size, valueCount_));
//		channels_[i] = &channelData_[i * capacity_];
//		if (name)
//		{
//			channelNames_[i] = name;
//			names_[i] = channelNames_[i].c_str();
//		}
//		channelNames_[i] = name;
//		names_[i] = channelNames_[i].c_str();
//	}
//}










