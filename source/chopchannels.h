#pragma once

#include <vector>
#include <string>

class ChopLink;
class InChopLink;
class OutChopLink;

class ChopChannelsBase
{
public:
	~ChopChannelsBase() { }

	const float* chan(const std::string& name);
	const float* chan(uint32_t index);
	const float* operator[](const std::string& name) { return chan(name); }
	const float* operator[](uint32_t index) { return chan(index); }

	const float** channels()		 { return channels_.data(); }
	const char* const* names() const { return names_.data(); }

	int32_t  channelCount() const    { return channelCount_; }
	uint32_t capacity() const        { return capacity_; }
	uint32_t valueCount() const      { return valueCount_; }
	double   rate() const            { return rate_; }
	bool     isTimeDependent() const { return isTimeDependent_; }

protected:
	ChopChannelsBase() = default;
	ChopChannelsBase(int32_t channelCount, uint32_t capacity, uint32_t valueCount, double rate, bool isTimeDependent);

	int32_t  channelCount_    { 0 };
	uint32_t capacity_        { 0 };
	uint32_t valueCount_      { 0 };
	double   rate_            { -1.0 };
	bool     isTimeDependent_ { false };

	std::vector<const float*> channels_;
	std::vector<const char*> names_;

	friend class ChopLink;
	friend class InChopLink;
	friend class OutChopLink;
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

	friend class InChopLink;

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

	void setChannels(const float* const* data, int32_t channelCount, uint32_t capacity, 
		uint32_t valueCount, double rate, bool isTimeDependent, const char* const* names = nullptr);


	// not implemented, need to make tests for each of these... 
	//void setCapacity(uint32_t capacity);
	//void setRate(double rate);
	//void setIsTimeDependent(bool isTimeDependent);
	//void appendChannel(const float* data = nullptr, uint32_t size = 0, const char* name = nullptr);
	//void appendChannel(const float* data, const std::string& name);
	//void appendChannel(const std::vector<float>& data, const std::string& name);
	//void appendChannel(const std::string& name);
	//void removeChannel(const std::string& name);
	//void removeChannel(uint32_t index);
	//void clear();



private:
	std::vector<float> channelData_;
	std::vector<std::string> channelNames_;

	friend class OutChopLink;
};


