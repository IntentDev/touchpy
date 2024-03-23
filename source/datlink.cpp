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

void 
OutDatLink::setTableFromValue(DatTable& table, const TouchObject<TEObject>& value)
{
	TouchObject<TETable> teTable;
	teTable.set(static_cast<TETable*>(value.get()));

	table.numRows = static_cast<uint32_t>(TETableGetRowCount(teTable.get()));
	table.numCols = static_cast<uint32_t>(TETableGetColumnCount(teTable.get()));
	table.values.resize(static_cast<size_t>(table.numRows * table.numCols));

	for (int32_t row = 0; row < table.numRows; ++row)
		for (int32_t col = 0; col < table.numCols; ++col)
			table.values[static_cast<size_t>(row * table.numCols + col)] = TETableGetStringValue(teTable.get(), row, col);
}

void 
OutDatLink::setStringFromValue(std::string& string, const TouchObject<TEObject>& value)
{
	TouchObject<TEString> teString;
	teString.reset();
	teString.set(static_cast<TEString*>(value.get()));
	string = teString->string;
}

void 
OutDatLink::update()
{
	TouchObject<TEObject> value;
	TEResult result = TEInstanceLinkGetObjectValue(instance_, identifier().c_str(), TELinkValueCurrent, value.take());
	if (result == TEResultSuccess)
	{
		if (value && TEGetType(value) == TEObjectTypeTable)
		{
			type_ = DatLinkType::Table;
			setTableFromValue(*table_, value);
		}
		else if (value && TEGetType(value) == TEObjectTypeString)
		{
			type_ = DatLinkType::String;
			setStringFromValue(string_, value);
		}
		updated_ = true;
	}
}

const DatTable&
OutDatLink::asTable()
{
	if (!usingSwapBuffer_ && !updated_) update();

	if (type_ == DatLinkType::Table)
	{
		return *table_;
	}
	else
	{
		table_->numRows = 1u;
		table_->numCols = 1u;
		table_->values.resize(1u);
		table_->values[0] = string_;
		return *table_;
	}
}

const std::string& 
OutDatLink::asString()
{
	if (!usingSwapBuffer_ && !updated_) update();

	if (type_ == DatLinkType::String) return string_;
	
	else return string_ = table_->asString();
}

void 
OutDatLink::swapBuffers()
{
	activeBuffer__.fetch_xor(1, std::memory_order_release);
}

void OutDatLink::writeBuffer()
{
	TouchObject<TEObject> value;
	TEResult result = TEInstanceLinkGetObjectValue(instance_, identifier().c_str(), TELinkValueCurrent, value.take());
	if (result == TEResultSuccess)
	{
		int nextBufferIndex = activeBuffer__.load(std::memory_order_acquire) ^ 1;
		if (value && TEGetType(value) == TEObjectTypeTable)
		{
			if (tableSwapBuffer_.size() != 2) tableSwapBuffer_.resize(2);
			auto& table = tableSwapBuffer_[nextBufferIndex];
			type_ = DatLinkType::Table;
			setTableFromValue(table, value);
		}
		else if (value && TEGetType(value) == TEObjectTypeString)
		{
			if (stringSwapBuffer_.size() != 2) stringSwapBuffer_.resize(2);
			auto& string = stringSwapBuffer_[nextBufferIndex];
			type_ = DatLinkType::String;
			setStringFromValue(string, value);
		}
		{
			std::lock_guard<std::mutex> lock(mutex_);
			bufferReadReady_ = true; // Mark as ready for writing
			cv_.notify_one(); // Notify the writing thread
		}
		swapBuffers();
		updated_ = true;
	}
}

void OutDatLink::moveBuffer()
{
	std::unique_lock<std::mutex> lock(mutex_);
	cv_.wait(lock, [this] { return bufferReadReady_; }); // Wait until data is ready
	int bufferIndex = activeBuffer__.load(std::memory_order_acquire);

	if (type_ == DatLinkType::Table) *table_ = std::move(tableSwapBuffer_[bufferIndex]);
	
	else string_ = std::move(stringSwapBuffer_[bufferIndex]);
	
	bufferReadReady_ = false;
}





