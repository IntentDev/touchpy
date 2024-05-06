#pragma once

#include <TouchEngine/TouchEngine.h>
#include "links.h"
#include "dattable.h"

#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include <condition_variable>
#include <mutex>


class DatLink : public Link<DatLink>
{
public:
	DatLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : Link<DatLink>(instance, linkInfo) { }
	~DatLink() {};

	enum class DatLinkType
	{
		Table,
		String
	};

	const DatLinkType type() const { return type_; }

protected:

	DatLinkType	type_{ DatLinkType::Table };

};


class InDatLink : public DatLink
{
public:
	InDatLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo) : DatLink(instance, linkInfo) { }
	~InDatLink() {};

	void set(const DatTable& table);
	void set(const char* string);
	void set(const std::string& string) { set(string.c_str()); }
};

class InDatLinks : public Links<InDatLinks, InDatLink>
{
public:
	InDatLinks() = default;
	InDatLinks(TouchObject<TEInstance> instance) : Links<InDatLinks, InDatLink>(instance) { }
	~InDatLinks() {};
};

class OutDatLink : public DatLink
{
public:
	OutDatLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo);
	~OutDatLink();

	void writeBuffer();
	void moveBuffer();
	void setUsingSwapBuffer(bool usingSwapBuffer) { usingSwapBuffer_ = usingSwapBuffer; }

	const DatTable& asTable();
	const std::string& asString();


private:
	void setTableFromValue(DatTable& table, const TouchObject<TEObject>& value);
	void setStringFromValue(std::string& string, const TouchObject<TEObject>& value);
	void update();

	void swapBuffers();

	bool usingSwapBuffer_{ false };
	std::atomic<int> activeBuffer_{ 0 }; // Index of the buffer that is ready for reading
	std::mutex mutex_;
	std::condition_variable cv_;
	bool bufferMoveReady_{ false };

	std::vector<DatTable>	  tableSwapBuffer_;
	std::unique_ptr<DatTable> table_;

	std::vector<std::string>  stringSwapBuffer_;
	std::string               string_;

};

class OutDatLinks : public Links<OutDatLinks, OutDatLink>
{
public:
	OutDatLinks() = default;
	OutDatLinks(TouchObject<TEInstance> instance) : Links<OutDatLinks, OutDatLink>(instance) { }
	~OutDatLinks() {};
};

