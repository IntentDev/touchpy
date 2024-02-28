#pragma once

#include <TouchEngine/TouchEngine.h>
#include <memory> 
#include <vector>
#include <string>
#include <map>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>



class Chop
{
public:
	enum class Mode { Input, Output, Constant };

	Chop(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo, Mode mode)
		:	instance_(instance), 
			identifier_(linkInfo->identifier),
			name_(linkInfo->name), 
			mode_(mode) { }

	~Chop() { };

	void set(const std::vector<float>& channels, uint32_t valueCount, double rate = -1.0);
	void set(const std::vector<float>& channels, uint32_t valueCount, double rate, const std::vector<std::string>& names);
	void set(const std::vector<std::vector<float>>& channels, double rate = -1.0);
	void set(const std::vector<std::vector<float>>& channels, double rate, const std::vector<std::string>& names);
	void set(const float** values, int32_t channelCount, uint32_t valueCount, double rate, const char** names = nullptr);

	std::string name() const { return name_; }
	std::string identifier() const { return identifier_; }
	Mode mode() const { return mode_; }


	const std::vector<float>& channelData() const { return channelData_; }
	const std::vector<const float*>& channelPtrs() const { return chanDataPtrs_; }
	const std::vector<std::string>& names() const { return names_; }
	int32_t channelCount() const { return channelCount_; }
	uint32_t valueCount() const { return valueCount_; }
	double rate() const { return rate_; }

	const float* channel(const std::string& name);

	const float* operator[](const std::string& name) { return channel(name); }
	const float* operator[](uint32_t index)
	{
		if (index < channelCount_)
			return chanDataPtrs_[index];
		return nullptr;
	}


	void swapTeBuffers();
	void updateTeBuffer();
	void readTeBuffer();

	void updateChannelData();

		
private:
	TouchObject<TEInstance> instance_;
	std::string identifier_;
	std::string name_;
	Mode mode_ { Mode::Input };

	double rate_ { -1.0 };
	int32_t channelCount_ { 0 };
	uint32_t capacity_ { 0 };
	uint32_t valueCount_ { 0 };

	
	std::vector<const float*> chanDataPtrs_;
	std::vector<const char*> namePtrs_;

	// input only
	//-----------------------------------------------------------------------------------------------------------------
	bool bufferCopyable(TouchObject<TEFloatBuffer> buffer, bool checkNames = true) const;


	// output only
	//-----------------------------------------------------------------------------------------------------------------
	//ChopDoubleBuffer doubleBuffer_; // output only
	TouchObject<TEFloatBuffer> teBuffer_; // output only

	TouchObject<TEFloatBuffer> teBuffers_[2]; 
	std::atomic<int> activeTeBuffer_{ 0 }; // Index of the buffer that is ready for reading
	std::mutex mutex_;
	std::condition_variable cv_;
	bool ready_{ false };

	std::vector<float> channelData_;
	std::vector<std::string> names_;

};


class ChopCollection
{
public:
	ChopCollection() = default;
	ChopCollection(TouchObject<TEInstance> instance) : instance_(instance) { }
	~ChopCollection() { };

	void addChop(TouchObject<TELinkInfo> linkInfo, Chop::Mode mode)
	{
		std::unique_ptr<Chop> chop = std::make_unique<Chop>(instance_, linkInfo, mode);
		chops_.push_back(std::move(chop));
		identifierMap_[linkInfo->identifier] = chops_.back().get();
		nameMap_[linkInfo->name] = chops_.back().get();
	}

	const std::vector<std::unique_ptr<Chop>>& chops() { return chops_; }
	Chop& get(const std::string& name) { return *nameMap_[name]; } // not safe
	Chop& getById(const std::string& identifier) { return *identifierMap_[identifier]; } // not safe
	Chop& operator[](const std::string& name) { return get(name); } // not safe
	Chop& operator[](uint32_t index) { return *chops_[index].get(); } // not safe

	void reset() 
	{ 
		chops_.clear(); 
		identifierMap_.clear();
		nameMap_.clear();
	}

private:
	TouchObject<TEInstance> instance_;
	std::vector<std::unique_ptr<Chop>> chops_;
	std::map<std::string, Chop*> identifierMap_;
	std::map<std::string, Chop*> nameMap_;

	void updatePointers() 
	{
		identifierMap_.clear();
		nameMap_.clear();
		for (auto& chop : chops_) 
		{
			identifierMap_[chop->identifier()] = chop.get(); 
			nameMap_[chop->name()] = chop.get();
		}
	}
};