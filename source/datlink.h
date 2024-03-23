#pragma once

#include <TouchEngine/TouchEngine.h>
#include "links.h"
#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include <condition_variable>
#include <mutex>

struct DatTable
{
	std::vector<std::string> values;
	uint32_t numRows{ 0 };
	uint32_t numCols{ 0 };

	std::vector<std::string_view> row(uint32_t i)
	{ 
		return std::vector<std::string_view>(values.begin() + i * numCols, values.begin() + (i + 1) * numCols);
	}

	std::vector<std::string_view> col(uint32_t i) 
	{ 
		std::vector<std::string_view> column;
		for (uint32_t j = 0; j < numRows; ++j)
		{
			column.push_back(values[j * numCols + i]);
		}
		return column;
	}

	std::string_view cell(uint32_t i, uint32_t j) 
	{
		return values[i * numCols + j]; 
	}

	std::string asString() const
	{
		std::string str;
		auto lastRow = numRows - 1;
		auto lastCol = numCols - 1;

		for (uint32_t i = 0; i < numRows; ++i)
		{
			for (uint32_t j = 0; j < numCols; ++j)
			{
				str += values[i * numCols + j];
				if (j < lastCol) str += "\t";
			}
			if (i < lastRow) str += "\n";
		}
		return str;
	}
};

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

	DatLinkType	type_ { DatLinkType::Table };

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
	std::atomic<int> activeBuffer__{ 0 }; // Index of the buffer that is ready for reading
	std::mutex mutex_;
	std::condition_variable cv_;
	bool bufferReadReady_{ false };

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

