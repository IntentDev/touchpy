#include <gtest/gtest.h>

#include "chopchannels.h"


TEST(ChopChannelsTest, chopChannelsReference)
{
	std::vector<float> chanData;
	std::vector<std::string> chanNames;

	auto channelCount = 5;
	auto valueCount = 10;

	for (int i = 0; i < channelCount; i++)
	{
		for (int j = 0; j < valueCount; j++)
		{
			chanData.push_back(i * valueCount + j);
		}
		chanNames.push_back("chan" + std::to_string(i));
	}


	std::vector<const float*> channels(channelCount);
	std::vector<const char*> names(channelCount);
	{
		for (int32_t i = 0; i < channelCount; ++i)
		{
			channels[i] = chanData.data() + i * valueCount;
			names[i] = chanNames[i].c_str();
		}
	}

	ChopChannelsView chopChannels(std::move(channels), channelCount, valueCount, valueCount, -1.0, false, std::move(names));

	EXPECT_EQ(chopChannels.channelCount(), channelCount);
	EXPECT_EQ(chopChannels.capacity(), valueCount);
	EXPECT_EQ(chopChannels.valueCount(), valueCount);
	EXPECT_EQ(chopChannels.rate(), -1.0);
	EXPECT_EQ(chopChannels.isTimeDependent(), false);

	for (int i = 0; i < channelCount; i++)
	{
		EXPECT_EQ(strcmp(chopChannels.names()[i], chanNames[i].c_str()), 0);
		for (int j = 0; j < valueCount; j++)
		{
			EXPECT_EQ(chopChannels.channels()[i][j], chanData[i * valueCount + j]);
		}
	}
}

TEST(ChopChannelsTest, chopChannels)
{
	std::vector<float> chanData;
	std::vector<std::string> chanNames;

	auto channelCount = 5;
	auto valueCount = 10;

	for (int i = 0; i < channelCount; i++)
	{
		for (int j = 0; j < valueCount; j++)
		{
			chanData.push_back(i * valueCount + j);
		}
		chanNames.push_back("chan" + std::to_string(i));
	}


	std::vector<const float*> channels(channelCount);
	std::vector<const char*> names(channelCount);
	{
		for (int32_t i = 0; i < channelCount; ++i)
		{
			channels[i] = chanData.data() + i * valueCount;
			names[i] = chanNames[i].c_str();
		}
	}

	ChopChannels chopChannels(channels.data(), channelCount, valueCount, valueCount, -1.0, false, 0, 0, names.data());

	EXPECT_EQ(chopChannels.channelCount(), channelCount);
	EXPECT_EQ(chopChannels.capacity(), valueCount);
	EXPECT_EQ(chopChannels.valueCount(), valueCount);
	EXPECT_EQ(chopChannels.rate(), -1.0);
	EXPECT_EQ(chopChannels.isTimeDependent(), false);

	for (int i = 0; i < channelCount; i++)
	{
		EXPECT_EQ(strcmp(chopChannels.names()[i], chanNames[i].c_str()), 0);
		for (int j = 0; j < valueCount; j++)
		{
			EXPECT_EQ(chopChannels.channels()[i][j], chanData[i * valueCount + j]);
		}
	}

}

TEST(ChopChannelsTest, setChannels)
{
	std::vector<float> chanData;
	std::vector<std::string> chanNames;

	auto channelCount = 5;
	auto valueCount = 10;

	for (int i = 0; i < channelCount; i++)
	{
		for (int j = 0; j < valueCount; j++)
		{
			chanData.push_back(i * valueCount + j);
		}
		chanNames.push_back("chan" + std::to_string(i));
	}


	std::vector<const float*> channels(channelCount);
	std::vector<const char*> names(channelCount);
	{
		for (int32_t i = 0; i < channelCount; ++i)
		{
			channels[i] = chanData.data() + i * valueCount;
			names[i] = chanNames[i].c_str();
		}
	}

	ChopChannels chopChannels;
	chopChannels.setChannels(channels.data(), channelCount, valueCount, valueCount, -1.0, false, 0, 0, names.data());

	EXPECT_EQ(chopChannels.channelCount(), channelCount);
	EXPECT_EQ(chopChannels.capacity(), valueCount);
	EXPECT_EQ(chopChannels.valueCount(), valueCount);
	EXPECT_EQ(chopChannels.rate(), -1.0);
	EXPECT_EQ(chopChannels.isTimeDependent(), false);

	for (int i = 0; i < channelCount; i++)
	{
		EXPECT_EQ(strcmp(chopChannels.names()[i], chanNames[i].c_str()), 0);
		for (int j = 0; j < valueCount; j++)
		{
			EXPECT_EQ(chopChannels.channels()[i][j], chanData[i * valueCount + j]);
		}
	}
}

