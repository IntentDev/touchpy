#include "datlink.h"

#include <iostream>


DatLink::DatLink(TouchObject<TEInstance> instance, TouchObject<TELinkInfo> linkInfo, LinkScope linkScope)
	:	Link<DatLink>(instance, linkInfo, linkScope)
{
	table_ = std::make_unique<Table>();
}

DatLink::~DatLink()
{
}

void
DatLink::updateOutput()
{
	TouchObject<TEObject> value;
	TEResult result = TEInstanceLinkGetObjectValue(instance, identifier.c_str(), TELinkValueCurrent, value.take());
	// String data can be a TETable or TEString, so check the type
	if (value && TEGetType(value) == TEObjectTypeTable)
	{
		type_ = DatLinkType::Table;
		TouchObject<TETable> teTable;
		teTable.reset();
		teTable.set(static_cast<TETable*>(value.get()));

		table_->numRows = static_cast<uint32_t>(TETableGetRowCount(teTable.get()));
		table_->numCols = static_cast<uint32_t>(TETableGetColumnCount(teTable.get()));
		table_->data.resize(static_cast<size_t>(table_->numRows * table_->numCols));

		for (int32_t row = 0; row < table_->numRows; ++row)
			for (int32_t col = 0; col < table_->numCols; ++col)
				table_->data[static_cast<size_t>(row * table_->numCols + col)] = TETableGetStringValue(teTable.get(), row, col);

		std::cout << "DatLink::updateOutput() " << getName() << " - rows / cols: " << TETableGetRowCount(teTable.get()) << " / " << TETableGetColumnCount(teTable.get()) << std::endl;
	}
	else if (value && TEGetType(value) == TEObjectTypeString)
	{
		type_ = DatLinkType::String;
		TouchObject<TEString> teString;
		teString.reset();
		teString.set(static_cast<TEString*>(value.get()));
		string_ = teString->string;
	}
}

void 
DatLink::set(const Table& table)
{
	type_ = DatLinkType::Table;
	TouchObject<TEObject> currentValue;
	TEResult result = TEInstanceLinkGetObjectValue(instance, identifier.c_str(), TELinkValueCurrent, currentValue.take());

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
				TETableSetStringValue(teTable, row, col, table.data[static_cast<size_t>(row * table.numCols + col)].c_str());
		
		result = TEInstanceLinkSetTableValue(instance, identifier.c_str(), teTable);
	}

	if (result != TEResultSuccess)
		std::cerr << "Failed to set table value: " << TEResultGetDescription(result) << std::endl;
}

void 
DatLink::set(const char* string)
{
	type_ = DatLinkType::String;
	TEResult result = TEInstanceLinkSetStringValue(instance, identifier.c_str(), string);
	if (result != TEResultSuccess)
		std::cerr << "Failed to set table value: " << TEResultGetDescription(result) << std::endl;
}


