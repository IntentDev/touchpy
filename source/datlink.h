#pragma once

#include <TouchEngine/TouchEngine.h>
#include "links.h"
#include <memory>
#include <string>

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
	const std::string getTypeDescription() const { return type_ == DatLinkType::Table ? "Table" : "String"; }

protected:
	TouchObject<TETable>   teTable_;
	std::unique_ptr<DatTable> table_;
	std::string            string_;
	DatLinkType            type_ { DatLinkType::Table };

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

	const DatTable& asTable();
	const std::string& asString();

	void onOuputValueChange();
	const DatTable& getTable() const { return *table_.get(); }
	const std::string& getString() const { return string_; }

};

class OutDatLinks : public Links<OutDatLinks, OutDatLink>
{
public:
	OutDatLinks() = default;
	OutDatLinks(TouchObject<TEInstance> instance) : Links<OutDatLinks, OutDatLink>(instance) { }
	~OutDatLinks() {};
};

