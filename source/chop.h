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

	std::string name() const { return name_; }

	//const std::vector<float>& getChannels() const { return channels_; }
	//const std::vector<std::string>& getNames() const { return names_; }

	const float* channel(const std::string& name);

	const float* operator[](const std::string& name) { return channel(name); }
	const float* operator[](uint32_t index) 
	{
		if (index < channelCount_) 
			return chanDataPtrs_[index]; 
		return nullptr;
	}


	void set(const std::vector<float>& channels, uint32_t valueCount, double rate);
	void set(const std::vector<float>& channels, uint32_t valueCount, double rate, const std::vector<std::string>& names);
	void set(const std::vector<std::vector<float>>& channels, double rate);
	void set(const std::vector<std::vector<float>>& channels, double rate, const std::vector<std::string>& names);
	void set(const float** values, int32_t channelCount, uint32_t valueCount, double rate, const char** names = nullptr);



	//void updateOutput();

	void swapTeBuffers();
	void updateTeBuffer();
	void readTeBuffer();

		
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
		chopPtrs_[linkInfo->name] = chops_.back().get();
	}

	std::vector<std::unique_ptr<Chop>>& chops() { return chops_; }
	Chop& get(const std::string& name) { return *chopPtrs_[name]; } // not safe
	Chop& operator[](const std::string& name) { return get(name); }
	Chop& operator[](uint32_t index) { return *chops_[index].get(); }


	void reset() { chops_.clear(); chopPtrs_.clear(); }

private:
	TouchObject<TEInstance> instance_;
	std::vector<std::unique_ptr<Chop>> chops_;
	std::map<std::string, Chop*> chopPtrs_;

	// Helper function to update pointers in chopPtrs_ after moves
	void updatePointers() {
		chopPtrs_.clear();
		for (auto& chop : chops_) {
			chopPtrs_[chop->name()] = chop.get(); // Assuming Chop has a getName() method
		}
	}
};