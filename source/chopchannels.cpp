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

ChopChannels::ChopChannels(const float* data, int32_t channelCount, uint32_t valueCount, double rate, bool isTimeDependent, int64_t startTime, int64_t endTime, const std::vector<std::string>& names)
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

ChopChannels::ChopChannels(uint32_t valueCount, double rate, bool isTimeDependent, int64_t startTime, int64_t endTime, const std::vector<std::string>& names)
{
	channelCount_ = names.size();
	capacity_ = valueCount;
	valueCount_ = valueCount;
	rate_ = rate;
	isTimeDependent_ = isTimeDependent;
	startTime_ = startTime;
	endTime_ = endTime;
	channelsBuffer_.resize(channelCount_ * valueCount);
	channels_.resize(channelCount_);

	if (names.size() > 0)
	{
		std::fill(channelsBuffer_.begin(), channelsBuffer_.end(), 0.0f);
		namesBuffer_.resize(channelCount_);
		names_.resize(channelCount_);

		for (int32_t i = 0; i < channelCount_; ++i)
		{
			channels_[i] = &channelsBuffer_[i * capacity_];
			namesBuffer_[i] = names[i];
			names_[i] = namesBuffer_[i].c_str();
		}
	}
}

void 
ChopChannels::setChannels(
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

void
ChopChannels::removeChannel(const std::string& name)
{
	removeChannel(name.c_str());
}

float* 
ChopChannels::mutableChan(uint32_t index)
{
	if (index < channelCount_)
	{
		return channelsBuffer_.data() + index * capacity_;
	}
	return nullptr;
}

float* 
ChopChannels::mutableChan(const char* name)
{
	for (uint32_t i = 0; i < channelCount_; ++i)
	{
		if (strcmp(names_[i], name) == 0)
		{
			return channelsBuffer_.data() + i * capacity_;
		}
	}
	return nullptr;
}

void 
ChopChannels::setChannelValues(int32_t channelIndex, const float* data, uint32_t size, uint32_t offset)
{
	if (channelIndex < channelCount_)
	{
		if (size + offset <= capacity_)
		{
			std::copy(data, data + size, channelsBuffer_.begin() + channelIndex * capacity_ + offset);
		}
		else
			throw std::runtime_error("size + offset out of range");
	}
}


void 
ChopChannels::setChannelValues(int32_t channelIndex, const std::vector<float>& data, uint32_t offset)
{
	setChannelValues(channelIndex, data.data(), data.size(), offset);
}

void 
ChopChannels::setChannelValues(const std::string& channelName, const float* data, uint32_t size, uint32_t offset)
{
	for (uint32_t i = 0; i < channelCount_; ++i)
	{
		if (strcmp(names_[i], channelName.c_str()) == 0)
		{
			if (size + offset <= capacity_)
				std::copy(data, data + size, channelsBuffer_.begin() + i * capacity_ + offset);
			else
				throw std::runtime_error("size + offset out of range");
			break;
		}
	}
}

void 
ChopChannels::setChannelValues(const std::string& channelName, const std::vector<float>& data, uint32_t offset)
{
	setChannelValues(channelName, data.data(), data.size(), offset);
}

void
ChopChannels::setChannelValue(int32_t channelIndex, uint32_t valueIndex, float value)
{
	if (channelIndex < channelCount_ && valueIndex < valueCount_)
	{
		channelsBuffer_[channelIndex * capacity_ + valueIndex] = value;
	}
	else
		throw std::runtime_error("channelIndex or valueIndex out of range");
}

void 
ChopChannels::setChannelValue(const std::string& channelName, uint32_t valueIndex, float value)
{
	for (uint32_t i = 0; i < channelCount_; ++i)
	{
		if (strcmp(names_[i], channelName.c_str()) == 0)
		{
			if (valueIndex < valueCount_)
			{
				channelsBuffer_[i * capacity_ + valueIndex] = value;
				break;
			}
			else
				throw std::runtime_error("valueIndex out of range");
		}
	}
}

void
ChopChannels::setChannelName(uint32_t index, const char* name)
{
	if (index < channelCount_)
	{
		namesBuffer_[index] = name;
		names_[index] = namesBuffer_[index].c_str();
	}
}

void 
ChopChannels::setRate(double rate) { rate_ = rate; }

void 
ChopChannels::setIsTimeDependent(bool isTimeDependent) { isTimeDependent_ = isTimeDependent; }

void
ChopChannels::setStartTime(int64_t startTime) { startTime_ = startTime; }

void 
ChopChannels::setEndTime(int64_t endTime) { endTime_ = endTime; }


void 
ChopChannels::appendChannel(const float* data, uint32_t size, const char* name)
{
	channelsBuffer_.resize((channelCount_ + 1) * capacity_);
	channels_.resize(channelCount_ + 1);
	
	if (data)
	{
		if (size <= capacity_)
			std::copy(data, data + size, channelsBuffer_.begin() + channelCount_ * capacity_);
		else
			throw std::runtime_error("size out of range");
	}
	else
		std::fill(channelsBuffer_.begin() + channelCount_ * capacity_, channelsBuffer_.begin() + (channelCount_ + 1) * capacity_, 0.0f);
		
	channels_[channelCount_] = &channelsBuffer_[channelCount_ * capacity_];

	auto nameCount = namesBuffer_.size();
	if (name || nameCount > 0)
	{
		if (nameCount != channelCount_)
		{
			namesBuffer_.resize(channelCount_ + 1);
			for (uint32_t i = 0; i < channelCount_; ++i)
			{
				namesBuffer_[i] = "chan" + std::to_string(i + 1);
			}
		}
		else
			namesBuffer_.resize(channelCount_ + 1);

		if (name)
			namesBuffer_[channelCount_] = name;
		else
			namesBuffer_[channelCount_] = "chan" + std::to_string(channelCount_ + 1);
	
		names_.resize(channelCount_ + 1);
		for (uint32_t i = 0; i < channelCount_; ++i)
		{
			names_[i] = namesBuffer_[i].c_str();
		}
	}

	++channelCount_;
}

void 
ChopChannels::appendChannel(const std::string& name, const std::vector<float>& data)
{
	if (!name.empty())
		appendChannel(data.data(), data.size(), name.c_str());
	else
		appendChannel(data.data(), data.size(), nullptr);
}

void 
ChopChannels::insertChannel(uint32_t index, const float* data, uint32_t size, const char* name)
{
	if (index < channelCount_)
	{
		channelsBuffer_.resize((channelCount_ + 1) * capacity_);
		
		if (size <= capacity_)
		{
			std::copy_backward(
				channelsBuffer_.begin() + index * capacity_, 
				channelsBuffer_.begin() + (channelCount_) * capacity_,
				channelsBuffer_.begin() + (channelCount_ + 1) * capacity_
			);
		}
		else
			throw std::runtime_error("size out of range");

		if (data)
		{
			std::copy(data, data + size, channelsBuffer_.begin() + index * capacity_);
		}
		else
		{
			std::fill(channelsBuffer_.begin() + index * capacity_, channelsBuffer_.begin() + (index + 1) * capacity_, 0.0f);
		}

		channels_.resize(channelCount_ + 1);

		if (namesBuffer_.size() == 0 && !name)
		{
			for (uint32_t i = 0; i < index; ++i)
			{
				channels_[i] = &channelsBuffer_[i * capacity_];
			}
		}
		else
		{
			if (namesBuffer_.size() != channelCount_)
			{
				namesBuffer_.resize(channelCount_ + 1);
				names_.resize(channelCount_ + 1);

				for (uint32_t i = 0; i < channelCount_; ++i)
				{
					channels_[i] = &channelsBuffer_[i * capacity_];
					namesBuffer_[i] = "chan" + std::to_string(i + 1);
					names_[i] = namesBuffer_[i].c_str();
				}
				namesBuffer_[index] = name;
				names_[index] = namesBuffer_[index].c_str();
			}
			else
			{
				namesBuffer_.insert(namesBuffer_.begin() + index, name);
				names_.resize(channelCount_ + 1);

				for (uint32_t i = 0; i < channelCount_; ++i)
				{
					channels_[i] = &channelsBuffer_[i * capacity_];
					names_[i] = namesBuffer_[i].c_str();
				}
			}
		}

		++channelCount_;
	}
}

void 
ChopChannels::insertChannel(uint32_t index, const std::string& name, const std::vector<float>& data)
{
	if (!name.empty())
		insertChannel(index, data.data(), data.size(), name.c_str());
	else
		insertChannel(index, data.data(), data.size(), nullptr);
}

void 
ChopChannels::removeChannel(uint32_t index)
{
	if (index < channelCount_)
	{
		channels_.erase(channels_.begin() + index);
		channelsBuffer_.erase(channelsBuffer_.begin() + index * capacity_, channelsBuffer_.begin() + (index + 1) * capacity_);

		if (namesBuffer_.size() == channelCount_)
		{
			namesBuffer_.erase(namesBuffer_.begin() + index);
			names_.erase(names_.begin() + index);
		}

		--channelCount_;
	}
}

void 
ChopChannels::removeChannel(const char* name)
{
	for (uint32_t i = 0; i < channelCount_; ++i)
	{
		if (strcmp(names_[i], name) == 0)
		{
			removeChannel(i);
			break;
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










