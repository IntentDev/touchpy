#pragma once

#include <vector>
#include <string>
#include <string_view>

class DatLink;
class OutDatLink;
class InDatLink;

class DatTable
{
public:	
	DatTable() = default;
	DatTable(uint32_t numRows, uint32_t numCols);
	DatTable(const std::vector<std::string>& values, uint32_t numRows, uint32_t numCols);
	DatTable(const std::vector<std::vector<std::string>>& values);
	~DatTable() = default;

	std::vector<std::string>& values() { return values_; }
	void setValues(const std::vector<std::string>& values);
	void setValue(size_t valueIndex, const std::string& value);
	//void setValue(size_t valueIndex, std::string&& value);

	uint32_t numRows() const { return numRows_; }
	uint32_t numCols() const { return numCols_; }
	
	std::vector<std::string_view> row(uint32_t i);
	std::vector<std::string_view> col(uint32_t i);
	std::string_view cell(uint32_t i, uint32_t j);

	std::string asString() const;

	void resize(uint32_t numRows, uint32_t numCols);
	void setNumRows(uint32_t numRows);
	void setNumCols(uint32_t numCols);

	void setCell(size_t i, size_t j, const std::string& value);
	//void setCell(size_t i, size_t j, std::string&& value);

	void setRow(size_t i, const std::vector<std::string>& row);
	void setCol(size_t i, const std::vector<std::string>& col);

	void appendRow(const std::vector<std::string>& row = {});
	void appendCol(const std::vector<std::string>& col = {});

	void insertRow(size_t i, const std::vector<std::string>& row = {});
	void insertCol(size_t i, const std::vector<std::string>& col = {});

	void removeRow(size_t i);
	void removeCol(size_t i);

	void clear();




protected:
	uint32_t numRows_{ 0 };
	uint32_t numCols_{ 0 };
	std::vector<std::string> values_;

	friend class DatLink;
	friend class OutDatLink;
	friend class InDatLink;
};
