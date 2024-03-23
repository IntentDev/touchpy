#include "datlink.h"

#include <iostream>


void
InDatLink::set(const DatTable& table)
{
	type_ = DatLinkType::Table;
	TouchObject<TEObject> currentValue;
	TEResult result = TEInstanceLinkGetObjectValue(instance_, identifier_.c_str(), TELinkValueCurrent, currentValue.take());

	if (result == TEResultSuccess)
	{
		TouchObject<TETable> teTable;
		if (currentValue && TEGetType(currentValue) == TEObjectTypeTable)
			teTable.take(TETableCreateCopy(static_cast<TETable*>(currentValue.get())));
		else
			teTable.take(TETableCreate());

		TETableResize(teTable, table.numRows, table.numCols);
		for (int32_t col = 0; col < table.numRows; ++col)
			for (int32_t row = 0; row < table.numCols; ++row)
				TETableSetStringValue(teTable, row, col, table.values[static_cast<size_t>(row * table.numCols + col)].c_str());

		result = TEInstanceLinkSetTableValue(instance_, identifier_.c_str(), teTable);
	}

	if (result != TEResultSuccess)
		std::cerr << "Failed to set table value: " << TEResultGetDescription(result) << std::endl;
}

void
InDatLink::set(const char* string)
{
	type_ = DatLinkType::String;
	TEResult result = TEInstanceLinkSetStringValue(instance_, identifier_.c_str(), string);
	if (result != TEResultSuccess)
		std::cerr << "Failed to set table value: " << TEResultGetDescription(result) << std::endl;
}



OutDatLink::OutDatLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo)
	:	DatLink(instance, linkInfo)
{
	table_ = std::make_unique<DatTable>();
}

OutDatLink::~OutDatLink()
{
}

const DatTable& 
OutDatLink::asTable()
{
	TouchObject<TEObject> value;
	TEResult result = TEInstanceLinkGetObjectValue(instance_, identifier().c_str(), TELinkValueCurrent, value.take());
	if (result == TEResultSuccess)
	{
		if (value && TEGetType(value) == TEObjectTypeTable)
		{
			TouchObject<TETable> teTable;
			teTable.set(static_cast<TETable*>(value.get()));

			table_->numRows = static_cast<uint32_t>(TETableGetRowCount(teTable.get()));
			table_->numCols = static_cast<uint32_t>(TETableGetColumnCount(teTable.get()));
			table_->values.resize(static_cast<size_t>(table_->numRows * table_->numCols));

			for (int32_t row = 0; row < table_->numRows; ++row)
				for (int32_t col = 0; col < table_->numCols; ++col)
					table_->values[static_cast<size_t>(row * table_->numCols + col)] = TETableGetStringValue(teTable.get(), row, col);

		}
		else if (value && TEGetType(value) == TEObjectTypeString)
		{
			TouchObject<TEString> teString;
			teString.reset();
			teString.set(static_cast<TEString*>(value.get()));
			table_->numRows = 1u;
			table_->numCols = 1u;
			table_->values.resize(1u);
			table_->values[0] = teString->string;
		}
	}
	return *table_;
}

const std::string& 
OutDatLink::asString()
{
	TouchObject<TEObject> value;
	TEResult result = TEInstanceLinkGetObjectValue(instance_, identifier().c_str(), TELinkValueCurrent, value.take());
	if (result == TEResultSuccess)
	{
		if (value && TEGetType(value) == TEObjectTypeTable)
		{
			TouchObject<TETable> teTable;
			teTable.set(static_cast<TETable*>(value.get()));
			auto numRows = static_cast<uint32_t>(TETableGetRowCount(teTable.get()));
			auto numCols = static_cast<uint32_t>(TETableGetColumnCount(teTable.get()));
			auto lastRow = numRows - 1;

			string_ = "";
			for (int32_t row = 0; row < numRows; ++row)
			{
				for (int32_t col = 0; col < numCols; ++col)
				{
					if (col > 0) string_ += "\t";
					string_ += TETableGetStringValue(teTable.get(), row, col);
				}
				if (row < lastRow) string_ += "\n";
			}
		}
		else if (value && TEGetType(value) == TEObjectTypeString)
		{
			TouchObject<TEString> teString;
			teString.reset();
			teString.set(static_cast<TEString*>(value.get()));
			string_ = teString->string;
		}
	}
	return string_;
}
