#pragma once

#include <vector>
#include <string>


class ChopChannelsBase
{
public:
	ChopChannelsBase() = default;
	~ChopChannelsBase() { }

	const float* chan(const std::string& name);
	const float* chan(uint32_t index);
	const float* operator[](const std::string& name) { return chan(name); }
	const float* operator[](uint32_t index) { return chan(index); }

	const float* const* channels() const { return channels_.data(); }
	const char* const* names() const { return names_.data(); }

	int32_t  channelCount() const    { return channelCount_; }
	uint32_t capacity() const        { return capacity_; }
	uint32_t valueCount() const      { return valueCount_; }
	double   rate() const            { return rate_; }
	bool     isTimeDependent() const { return isTimeDependent_; }

protected:
	
	ChopChannelsBase(int32_t channelCount, uint32_t capacity, uint32_t valueCount, double rate, bool isTimeDependent);

	int32_t  channelCount_    { 0 };
	uint32_t capacity_        { 0 };
	uint32_t valueCount_      { 0 };
	double   rate_            { -1.0 };
	bool     isTimeDependent_ { false };

	std::vector<const float*> channels_;
	std::vector<const char*> names_;
};

class ChopChannelsReference : public ChopChannelsBase
{
public:
	ChopChannelsReference() = default;
	ChopChannelsReference(
		std::vector<const float*>&& channels,
		int32_t channelCount, 
		uint32_t capacity, 
		uint32_t valueCount, 
		double rate, 
		bool isTimeDependent,
		std::vector<const char*>&& names = {});

	~ChopChannelsReference() { }

private:

};

class ChopChannels : public ChopChannelsBase
{
public:
	ChopChannels() = default;
	ChopChannels(const float* const* data, int32_t channelCount, uint32_t capacity, 
		uint32_t valueCount, double rate, bool isTimeDependent, const char* const* names = nullptr);

	~ChopChannels() { }

	const float* data() const { return channelData_.data(); }
	const std::vector<float>& channelData() const { return channelData_; }
	const std::vector<std::string>& channelNames() const { return channelNames_; }

private:
	std::vector<float> channelData_;
	std::vector<std::string> channelNames_;
};


