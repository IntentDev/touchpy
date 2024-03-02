#pragma once

#include <TouchEngine/TouchEngine.h>
#include "links.h"

#include <memory> 
#include <vector>
#include <string>
#include <map>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>



class ChopLink : public Link<ChopLink>
{
public:

	ChopLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo, LinkScope linkScope)
		:	Link<ChopLink>(instance, linkInfo, linkScope) { }

	~ChopLink() { };

	void updateOutput();
	void swapTeBuffers();
	void updateTeBuffer();
	void readTeBuffer();

	const std::vector<float>& channelData() const { return channelData_; }
	const std::vector<const float*>& channelPtrs() const { return chanDataPtrs_; }
	const std::vector<std::string>& names() const { return names_; }
	int32_t channelCount() const { return channelCount_; }
	uint32_t valueCount() const { return valueCount_; }
	double rate() const { return rate_; }

	//const float* channel(const std::string& name);
	//const float* operator[](const std::string& name) { return channel(name); }
	const float* operator[](uint32_t index)
	{
		if (index < channelCount_)
			return chanDataPtrs_[index];
		return nullptr;
	}

	// use setFrom[type] naming for Python bindings
	void set(const std::vector<float>& channels, uint32_t valueCount, double rate = -1.0);
	void set(const std::vector<float>& channels, uint32_t valueCount, double rate, const std::vector<std::string>& names);
	void set(const std::vector<std::vector<float>>& channels, double rate = -1.0);
	void set(const std::vector<std::vector<float>>& channels, double rate, const std::vector<std::string>& names);
	void set(const float** values, int32_t channelCount, uint32_t valueCount, double rate, const char** names = nullptr);

		
private:
	double rate_ { -1.0 };
	int32_t channelCount_ { 0 };
	uint32_t capacity_ { 0 };
	uint32_t valueCount_ { 0 };

	std::vector<const float*> chanDataPtrs_;
	std::vector<const char*> namePtrs_;

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

	// input only
	//-----------------------------------------------------------------------------------------------------------------
	bool bufferCopyable(TouchObject<TEFloatBuffer> buffer, const char** names = nullptr) const;

};

class ChopLinks : public Links<ChopLinks, ChopLink>
{
public:
	ChopLinks() = default;
	ChopLinks(TouchObject<TEInstance> instance, LinkScope linkScope) : Links<ChopLinks, ChopLink>(instance, linkScope) { }
	~ChopLinks() { };
};

//class ChopCollection
//{
//public:
//	ChopCollection() = default;
//	ChopCollection(TouchObject<TEInstance> instance) : instance_(instance) { }
//	~ChopCollection() { };
//
//	void addChop(TouchObject<TELinkInfo> linkInfo)
//	{
//		std::unique_ptr<ChopLink> chop = std::make_unique<ChopLink>(instance_, linkInfo);
//		chops_.push_back(std::move(chop));
//		identifierMap_[linkInfo->identifier] = chops_.back().get();
//		nameMap_[linkInfo->name] = chops_.back().get();
//	}
//
//	const std::vector<std::unique_ptr<ChopLink>>& chops() { return chops_; }
//	ChopLink& get(const std::string& name) { return *nameMap_[name]; } // not safe
//	ChopLink& getById(const std::string& identifier) { return *identifierMap_[identifier]; } // not safe
//	ChopLink& operator[](const std::string& name) { return get(name); } // not safe
//	ChopLink& operator[](uint32_t index) { return *chops_[index].get(); } // not safe
//
//	void reset() 
//	{ 
//		chops_.clear(); 
//		identifierMap_.clear();
//		nameMap_.clear();
//	}
//
//private:
//	TouchObject<TEInstance> instance_;
//	std::vector<std::unique_ptr<ChopLink>> chops_;
//	std::map<std::string, ChopLink*> identifierMap_;
//	std::map<std::string, ChopLink*> nameMap_;
//
//	void updatePointers() 
//	{
//		identifierMap_.clear();
//		nameMap_.clear();
//		for (auto& chop : chops_) 
//		{
//			identifierMap_[chop->identifier()] = chop.get(); 
//			nameMap_[chop->name()] = chop.get();
//		}
//	}
//};