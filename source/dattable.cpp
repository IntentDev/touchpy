#include "dattable.h"
#include "logging.h"

DatTable::DatTable(uint32_t numRows, uint32_t numCols) { resize(numRows, numCols); }

DatTable::DatTable(const std::vector<std::string>& values, uint32_t numRows, uint32_t numCols)
{
	if (values.size() == numRows * numCols)
	{
		values_ = values;
		numRows_ = numRows;
		numCols_ = numCols;
	}
	else
		throw std::runtime_error("DatTable::DatTable: values.size() != numRows * numCols");
}

DatTable::DatTable(const std::vector<std::vector<std::string>>& values)
{
	if (values.size() == 0)
	{
		numRows_ = 0;
		numCols_ = 0;
		return;
	}

	numRows_ = values.size();
	numCols_ = values[0].size();
	values_.resize(numRows_ * numCols_);

	for (size_t i = 0; i < numRows_; ++i)
	{
		if (values[i].size() != numCols_)
			throw std::runtime_error("DatTable::DatTable: values[i].size() != numCols_");

		for (size_t j = 0; j < numCols_; ++j)
		{
			values_[i * numCols_ + j] = values[i][j];
		}
	}
}

void 
DatTable::setValues(const std::vector<std::string>& values)
{
	if (values.size() == values_.size())
	{
		values_ = values;
		return;
	}
	else
		throw std::runtime_error("DatTable::setValues: values.size() != values_.size()");
}

void 
DatTable::setValue(size_t valueIndex, const std::string& value) { values_[valueIndex] = value; }

std::vector<std::string_view> 
DatTable::row(uint32_t i)
{
	if (i < numRows_)
		return std::vector<std::string_view>(values_.begin() + i * numCols_, values_.begin() + (i + 1) * numCols_);
	else
		return {};
}

std::vector<std::string_view>
DatTable::row(const std::string& name)
{
	auto i = getRowIndex(name);
	if (i < numRows_)
		return row(i);
	else
		return {};
}

std::vector<std::string_view> 
DatTable::col(uint32_t i)
{
	if (i >= numCols_)
		return {};

	std::vector<std::string_view> column;
	for (size_t j = 0; j < numRows_; ++j)
	{
		column.push_back(values_[j * numCols_ + i]);
	}
	return column;
}

std::vector<std::string_view>
DatTable::col(const std::string& name)
{
	auto i = getColIndex(name);
	if (i < numCols_)
		return col(i);
	else
		return {};
}

std::string_view 
DatTable::cell(uint32_t i, uint32_t j)
{
	if (i < numRows_ && j < numCols_)
		return values_[i * numCols_ + j];
	else
		return {};
}

std::string_view
DatTable::cell(const std::string& rowName, uint32_t j)
{
	auto i = getRowIndex(rowName);
	if (i < numRows_)
		return cell(i, j);
	else
		return {};
}

std::string_view
DatTable::cell(uint32_t i, const std::string& colName)
{
	auto j = getColIndex(colName);
	if (j < numCols_)
		return cell(i, j);
	else
		return {};
}

std::string_view
DatTable::cell(const std::string& rowName, const std::string& colName)
{
	auto i = getRowIndex(rowName);
	auto j = getColIndex(colName);
	if (i < numRows_ && j < numCols_)
		return cell(i, j);
	else
		return {};
}

std::string 
DatTable::asString() const
{
	std::string str;
	auto lastRow = numRows_ - 1;
	auto lastCol = numCols_ - 1;

	for (size_t i = 0; i < numRows_; ++i)
	{
		for (size_t j = 0; j < numCols_; ++j)
		{
			str += values_[i * numCols_ + j];
			if (j < lastCol) str += "\t";
		}
		if (i < lastRow) str += "\n";
	}
	return str;
}

void 
DatTable::resize(uint32_t numRows, uint32_t numCols)
{
	numRows_ = numRows;
	numCols_ = numCols;
	values_.resize(numRows_ * numCols_);
}

void 
DatTable::setNumRows(uint32_t numRows) { resize(numRows, numCols_); }

void 
DatTable::setNumCols(uint32_t numCols) { resize(numRows_, numCols); }

void 
DatTable::setCell(size_t i, size_t j, const std::string& value) 
{ 
	if (i < numRows_ && j < numCols_)
	{
		values_[i * numCols_ + j] = value;
	}
}

void
DatTable::setCell(const std::string& rowName, size_t j, const std::string& value)
{
	setCell(getRowIndex(rowName), j, value);
}

void
DatTable::setCell(size_t i, const std::string& colName, const std::string& value)
{
	setCell(i, getColIndex(colName), value);
}

void
DatTable::setCell(const std::string& rowName, const std::string& colName, const std::string& value)
{
	setCell(getRowIndex(rowName), getColIndex(colName), value);
}

void 
DatTable::setRow(size_t i, const std::vector<std::string>& row)
{
	if (i < numRows_)
	{
		if (row.size() <= numCols_)
			std::copy(row.begin(), row.end(), values_.begin() + i * numCols_);
		else
			std::copy(row.begin(), row.begin() + numCols_, values_.begin() + i * numCols_);
	}
}

void
DatTable::setRow(const std::string& name, const std::vector<std::string>& row)
{
	auto i = getRowIndex(name);
	setRow(i, row);
}

void 
DatTable::setCol(size_t i, const std::vector<std::string>& col)
{
	if (i < numCols_)
	{
		if (col.size() <= numRows_)
		{
			for (size_t j = 0; j < col.size(); ++j)
				values_[j * numCols_ + i] = col[j];
		}
		else
		{
			for (size_t j = 0; j < numRows_; ++j)
				values_[j * numCols_ + i] = col[j];
		}
	}
}

void
DatTable::setCol(const std::string& name, const std::vector<std::string>& col)
{
	auto i = getColIndex(name);
	setCol(i, col);
}

void 
DatTable::appendRow(const std::vector<std::string>& row)
{
	auto rowSize = row.size();

	if (rowSize < numCols_)
	{
		values_.insert(values_.end(), row.begin(), row.end());
		values_.insert(values_.end(), numCols_ - rowSize, "");
	}
	else if (rowSize == numCols_)
		values_.insert(values_.end(), row.begin(), row.end());
	else
		values_.insert(values_.end(), row.begin(), row.begin() + numCols_);
	
	numRows_++;
}

void 
DatTable::appendCol(const std::vector<std::string>& col)
{
	insertCol(numCols_, col);
}

void 
DatTable::insertRow(size_t i, const std::vector<std::string>& row)
{
	auto rowSize = row.size();

	if (rowSize < numCols_)
	{
		values_.insert(values_.begin() + i * numCols_, row.begin(), row.end());
		values_.insert(values_.begin() + (i + 1) * numCols_, numCols_ - rowSize, "");
	}
	else if (rowSize == numCols_)
		values_.insert(values_.begin() + i * numCols_, row.begin(), row.end());
	else
		values_.insert(values_.begin() + i * numCols_, row.begin(), row.begin() + numCols_);

	numRows_++;
}

void 
DatTable::insertCol(size_t j, const std::vector<std::string>& col)
{
	// insert value at index i in each row
	auto colSize = col.size();
	for (size_t i = 0; i < numRows_; ++i)
	{
		if (i < colSize)
			values_.insert(values_.begin() + i * numCols_ + j + i, col[i]);
		else
			values_.insert(values_.begin() + i * numCols_ + j + i, "");
	}

	numCols_++;
}

void
DatTable::removeRow(size_t i)
{
	if (i < numRows_)
	{
		values_.erase(values_.begin() + i * numCols_, values_.begin() + (i + 1) * numCols_);
		numRows_--;
	}
}

void
DatTable::removeRow(const std::string& name)
{
	auto i = getRowIndex(name);
	removeRow(i);
}

void 
DatTable::removeCol(size_t j)
{
	if (j < numCols_)
	{
		for (int64_t i = numRows_ - 1; i >= 0; --i)
		{
			values_.erase(values_.begin() + static_cast<size_t>(i * numCols_ + j));
		}
		--numCols_;
	}
}

void
DatTable::removeCol(const std::string& name)
{
	auto j = getColIndex(name);
	removeCol(j);
}

void 
DatTable::clear()
{
	values_.clear();
	numRows_ = 0;
	numCols_ = 0;
}

uint32_t 
DatTable::getRowIndex(const std::string& name)
{
	for (size_t i = 0; i < numRows_; ++i)
	{
		if (values_[i * numCols_] == name)
			return static_cast<uint32_t>(i);
	}
	return numRows_;
}

uint32_t 
DatTable::getColIndex(const std::string& name)
{
	for (size_t i = 0; i < numCols_; ++i)
	{
		if (values_[i] == name)
			return static_cast<uint32_t>(i);
	}
	return numCols_;
}








