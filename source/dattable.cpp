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

void DatTable::setValues(const std::vector<std::string>& values)
{
	if (values.size() == values_.size())
	{
		values_ = values;
		return;
	}
	else
		throw std::runtime_error("DatTable::setValues: values.size() != values_.size()");
}

void DatTable::setValue(size_t valueIndex, const std::string& value) { values_[valueIndex] = value; }

//void DatTable::setValue(size_t valueIndex, std::string&& value) { values_[valueIndex] = std::move(value); }

std::vector<std::string_view> DatTable::row(uint32_t i)
{
	return std::vector<std::string_view>(values_.begin() + i * numCols_, values_.begin() + (i + 1) * numCols_);
}

std::vector<std::string_view> DatTable::col(uint32_t i)
{
	std::vector<std::string_view> column;
	for (size_t j = 0; j < numRows_; ++j)
	{
		column.push_back(values_[j * numCols_ + i]);
	}
	return column;
}

std::string_view DatTable::cell(uint32_t i, uint32_t j)
{
	return values_[i * numCols_ + j];
}

std::string DatTable::asString() const
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

void DatTable::resize(uint32_t numRows, uint32_t numCols)
{
	numRows_ = numRows;
	numCols_ = numCols;
	values_.resize(numRows_ * numCols_);
}

void DatTable::setNumRows(uint32_t numRows) { resize(numRows, numCols_); }

void DatTable::setNumCols(uint32_t numCols) { resize(numRows_, numCols); }

void DatTable::setCell(size_t i, size_t j, const std::string& value) { values_[i * numCols_ + j] = value; }

//void DatTable::setCell(size_t i, size_t j, std::string&& value) { values_[i * numCols_ + j] = std::move(value); }

void DatTable::setRow(size_t i, const std::vector<std::string>& row)
{
	if (row.size() <= numCols_)
		std::copy(row.begin(), row.end(), values_.begin() + i * numCols_);
	else
		throw std::runtime_error("DatTable::setRow: row.size() > numCols_");
}

void DatTable::setCol(size_t i, const std::vector<std::string>& col)
{
	if (col.size() <= numRows_)
	{
		for (size_t j = 0; j < col.size(); ++j)
			values_[j * numCols_ + i] = col[j];
	}
	else
		throw std::runtime_error("DatTable::setCol: col.size() > numRows_");
}

void DatTable::appendRow(const std::vector<std::string>& row)
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

void DatTable::appendCol(const std::vector<std::string>& col)
{
	auto colSize = col.size();

	if (colSize < numRows_)
	{
		for (size_t i = 0; i < colSize; ++i)
			values_.push_back(col[i]);
		for (size_t i = colSize; i < numRows_; ++i)
			values_.push_back("");
	}
	else // colSize >= numRows_
	{
		for (size_t i = 0; i < numRows_; ++i)
			values_.push_back(col[i]);
	}

	numCols_++;
}

void DatTable::insertRow(size_t i, const std::vector<std::string>& row)
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

void DatTable::insertCol(size_t i, const std::vector<std::string>& col)
{
	auto colSize = col.size();

	if (colSize < numRows_)
	{
		for (size_t j = 0; j < colSize; ++j)
			values_.insert(values_.begin() + j * numCols_ + i, col[j]);
		for (size_t j = colSize; j < numRows_; ++j)
			values_.insert(values_.begin() + j * numCols_ + i, "");
	}
	else // colSize >= numRows_
	{
		for (size_t j = 0; j < numRows_; ++j)
			values_.insert(values_.begin() + j * numCols_ + i, col[j]);
	}

	numCols_++;
}

void DatTable::removeRow(size_t i)
{
	values_.erase(values_.begin() + i * numCols_, values_.begin() + (i + 1) * numCols_);
	numRows_--;
}

void DatTable::removeCol(size_t i)
{
	for (size_t j = 0; j < numRows_; ++j)
		values_.erase(values_.begin() + j * numCols_ + i);
	numCols_--;
}

void DatTable::clear()
{
	values_.clear();
	numRows_ = 0;
	numCols_ = 0;
}
