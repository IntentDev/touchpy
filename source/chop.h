#pragma once

#include <TouchEngine/TouchEngine.h>
#include <memory> 
#include <vector>
#include <string>
#include <map>
#include <array>

class Chop
{
public:
	enum class Mode { Input, Output, Constant };

	Chop(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo, Mode mode)
		:	instance_(instance), 
			identifier_(linkInfo->identifier),
			name_(linkInfo->name), 
			mode_(mode)
	{ 
		std::cout << "chop constructor: " << name_ << std::endl;
	}

	std::string name() const { return name_; }

	const std::vector<float>& getValues() const { return chanData_; }
	const std::vector<std::string>& getNames() const { return chanNames_; }

	const float* channel(const std::string& name)
	{
		auto it = nameChanMap_.find(name);
		if (it != nameChanMap_.end()) return it->second.channel;
		return nullptr;
	}

	const float* operator[](const std::string& name) { return channel(name); }
	const float* operator[](uint32_t index) 
	{
		if (index < channelCount_) 
			return chanDataPtr_[index]; 
		return nullptr;
	}

	void set(std::vector<float>& chanData, int32_t channelCount, uint32_t valueCount, double rate, const std::vector<std::string>* chanNames = nullptr)
	{
		std::cout << "chop set: " << name_ << std::endl;

		chanData_ = chanData;
		channelCount_ = channelCount;
		capacity_ = valueCount;
		valueCount_ = valueCount;
		rate_ = rate;

		chanDataPtr_.resize(channelCount_);

		
		if (!chanNames)
		{
			chanNames_ = std::vector<std::string>(channelCount, "chan");
		}
		else
		{
			chanNames_ = *chanNames;
		}
		chanNamesPtr_.resize(channelCount_);

		
		for (size_t chan = 0; chan < channelCount_; ++chan)
		{
			chanDataPtr_[chan] = &chanData_[chan * valueCount_];
			chanNames_[chan] += std::to_string(chan + 1);

			chanNamesPtr_[chan] = chanNames_[chan].c_str();
			nameChanMap_[chanNames_[chan]] = { &chanData_[chan * valueCount_], nameMapChanVersion_ };
		}

		pending_ = true;
	}



	void updateInput() 
	{
		std::cout << "chop input update: " << name_ << " values: " << *chanDataPtr_[0] << ", " << *chanDataPtr_[1] << ", " << *chanDataPtr_[2] << std::endl;
		//if (pending_)
		//{
			pending_ = false;

			if (chanData_.size() > 0)
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
						//buffer.take(TEFloatBufferCreate(rate_, channelCount_, capacity_, chanNamesPtr_.data()));
						buffer.take(TEFloatBufferCreate(rate_, channelCount_, capacity_, nullptr));
					}
					TEResult result = TEFloatBufferSetValues(buffer, chanDataPtr_.data(), valueCount_);

	/*				float value1 = 1.0f;
					float value2 = 2.0f;
					float value3 = 3.0f;
					float value4 = 4.0f;
					std::array<const float*, 4> channels{ &value1, &value2, &value3, &value4 };
					TEResult result = TEFloatBufferSetValues(buffer, channels.data(), valueCount_);*/

					std::cout << "set values result: " << TEResultGetDescription(result) << std::endl;

					result = TEInstanceLinkSetFloatBufferValue(instance_, identifier_.c_str(), buffer);

				}
				else
				{
					std::cout << "TEInstanceLinkGetFloatBufferValue Result: " << TEResultGetDescription(result) << std::endl;
				}


				//TEInstanceLinkSetFloatBufferValue(instance_, linkInfo_->identifier, buffer);
			}
		//}
	
	}

	void updateOutput()
	{
		buffer_.reset();

		if (TEInstanceLinkGetFloatBufferValue(instance_, identifier_.c_str(), TELinkValueCurrent, buffer_.take()) == TEResultSuccess)
		{
			channelCount_ = TEFloatBufferGetChannelCount(buffer_);
			capacity_ = TEFloatBufferGetCapacity(buffer_);
			valueCount_ = TEFloatBufferGetValueCount(buffer_);
			rate_ = TEFloatBufferGetRate(buffer_);

			if (buffer_)
			{
				const float* const* data = TEFloatBufferGetValues(buffer_);
				const char* const* names = TEFloatBufferGetChannelNames(buffer_);

				size_t arraySize = static_cast<size_t>(channelCount_ * valueCount_);
				chanData_.resize(arraySize);
				chanNames_.resize(channelCount_);

				++nameMapChanVersion_;

				for (size_t chan = 0; chan < channelCount_; ++chan)
				{
					for (size_t sample = 0; sample < valueCount_; ++sample)
					{
						chanData_[chan * valueCount_ + sample] = data[chan][sample];
					}

					chanDataPtr_[chan] = &chanData_[chan * valueCount_];

					chanNames_[chan] = names[chan];
					nameChanMap_[names[chan]] = { &chanData_[chan * valueCount_], nameMapChanVersion_ };
					chanNamesPtr_[chan] = chanNames_[chan].c_str();
				}

				cleanupNameChanMap();
			}
		}
	}

	~Chop() { };
		
private:
	TouchObject<TEInstance> instance_;
	std::string identifier_;
	std::string name_;

	Mode mode_ { Mode::Input };

	bool pending_ { false };

	double rate_ { -1.0 };
	int32_t channelCount_ { 0 };
	uint32_t capacity_ { 0 };
	uint32_t valueCount_ { 0 };

	std::vector<float> chanData_;
	std::vector<const float*> chanDataPtr_;

	std::vector<std::string> chanNames_;
	std::vector<const char*> chanNamesPtr_;



	struct NameChanMapVersion 
	{ 
		float* channel;
		uint32_t version;
	};

	uint32_t nameMapChanVersion_{ 0 };
	std::unordered_map<std::string, NameChanMapVersion> nameChanMap_;

	TouchObject<TEFloatBuffer> buffer_; // output only


	bool bufferCopyable(TouchObject<TEFloatBuffer> buffer) const
	{
		const char* const* names = TEFloatBufferGetChannelNames(buffer);
		auto newChannelCount = TEFloatBufferGetChannelCount(buffer);

		//if (names)
		//{
		//	for (int32_t chan = 0; chan < newChannelCount; ++chan)
		//	{
		//		if (chanNames_[chan] != names[chan]) return false;
		//	}
		//}

		return	newChannelCount == channelCount_
				|| TEFloatBufferGetCapacity(buffer) == capacity_
				|| TEFloatBufferGetValueCount(buffer) == valueCount_
				|| TEFloatBufferGetRate(buffer) == rate_;
	}

	void cleanupNameChanMap()
	{
		for (auto it = nameChanMap_.begin(); it != nameChanMap_.end();)
		{
			if (it->second.version != nameMapChanVersion_) it = nameChanMap_.erase(it);
			else ++it;
		}
	}
};


class ChopCollection
{
public:
	ChopCollection() = default;
	ChopCollection(TouchObject<TEInstance> instance) : instance_(instance) { }
	~ChopCollection() { };

	void addChop(TouchObject<TELinkInfo> linkInfo, Chop::Mode mode)
	{
		chops_.push_back(Chop(instance_, linkInfo, mode));
		chopPtrs_[linkInfo->name] = &chops_.back();
	}

	Chop& get(const std::string& name) { return *chopPtrs_[name]; } // not safe
	Chop& operator[](const std::string& name) { return get(name); }
	Chop& operator[](uint32_t index) { return chops_[index]; }


	void reset() { chops_.clear(); chopPtrs_.clear(); }

private:
	TouchObject<TEInstance> instance_;
	std::vector<Chop> chops_;
	std::map<std::string, Chop*> chopPtrs_;
};