#pragma once

#include <TouchEngine/TouchEngine.h>
#include "links.h"
#include <memory>
#include <string>

class DatLink : public Link<DatLink>
{
public:
	DatLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo, LinkScope linkScope);
	~DatLink();

	enum class DatLinkType
	{
		Table,
		String
	};

	struct Table
	{
		std::vector<std::string> data;
		uint32_t numRows { 0 };
		uint32_t numCols { 0 };
	};

	const DatLinkType type() const { return type_; }
	const std::string getTypeDescription() const { return type_ == DatLinkType::Table ? "Table" : "String"; }

	void updateOutput();

	const Table& getTable() const { return *table_.get(); }
	const std::string& getString() const { return string_; }

	void set(const Table& table);
	void set(const char* string);
	void set(const std::string& string) { set(string.c_str()); }

private:
	std::unique_ptr<Table> table_;
	std::string            string_;
	DatLinkType            type_ { DatLinkType::Table };

};

class DatLinks : public Links<DatLinks, DatLink>
{
public:
	DatLinks() = default;
	DatLinks(TouchObject<TEInstance> instance, LinkScope linkScope) : Links<DatLinks, DatLink>(instance, linkScope) { }
	~DatLinks() {};
};

