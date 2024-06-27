#include "datlinkpy.h"
#include "datlink.h"

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/string_view.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/pair.h>

namespace nb = nanobind;
using namespace nb::literals;

DatTable tableFromList(const nb::list& list, bool cast = false)
{
	DatTable table;
	auto numRows = list.size();
	if (numRows == 0)
		return table;

	if (numRows > 0 && nb::isinstance<nb::list>(list[0]))
	{
		auto firstRow = nb::cast<nb::list>(list[0]);
		auto numCols = firstRow.size();

		if (numCols == 0)
			return table;

		table.resize(numRows, numCols);

		if (cast)
		{
			for (size_t i = 0; i < numRows; ++i)
			{
				auto row = nb::cast<nb::list>(list[i]);
				for (size_t j = 0; j < numCols; ++j)
					if (nb::isinstance<nb::int_>(row[j]))
						table.setCell(i, j, std::to_string(nb::cast<int>(row[j])));
					else if (nb::isinstance<nb::float_>(row[j]))
						table.setCell(i, j, std::to_string(nb::cast<float>(row[j])));
					else if (nb::isinstance<nb::str>(row[j]))
						table.setCell(i, j, nb::cast<std::string>(row[j]));
					else
						table.setCell(i, j, "");
			}
		}
		else
		{
			for (size_t i = 0; i < numRows; ++i)
			{
				auto row = nb::cast<nb::list>(list[i]);
				for (size_t j = 0; j < numCols; ++j)
					table.setCell(i, j, nb::cast<std::string>(row[j]));
			}
		}
	}
	return table;
}

void initDatLinkBindings(nb::module_& m)
{
	nb::class_<DatTable> datTable(m, "DatTable");
	datTable.doc() = DatTableDoc;
	datTable.def(nb::init<>())

		.def(nb::init<uint32_t, uint32_t>(), "num_rows"_a, "num_cols"_a, DatTableDoc)
		.def(nb::init<const std::vector<std::string>&, uint32_t, uint32_t>(), "values"_a, "num_rows"_a, "num_cols"_a, DatTableDoc)
		.def(nb::init<const std::vector<std::vector<std::string>>&>(), "values"_a, DatTableDoc)

		.def("__getitem__", [](DatTable& self, std::pair<uint32_t, uint32_t> index) 
			{ return self.cell(index.first, index.second); }, cellDoc)
		.def("__getitem__", [](DatTable& self, std::pair<const std::string&, uint32_t> index) 
			{ return self.cell(index.first, index.second); }, cellDoc)
		.def("__getitem__", [](DatTable& self, std::pair<uint32_t, const std::string&> index) 
			{ return self.cell(index.first, index.second); }, cellDoc)
		.def("__getitem__", [](DatTable& self, std::pair<const std::string&, const std::string&> index) 
			{ return self.cell(index.first, index.second); }, cellDoc)

		.def("__setitem__", [](DatTable& self, std::pair<uint32_t, uint32_t> index, const std::string& value) 
			{ self.setCell(index.first, index.second, value); }, set_cellDoc)
		.def("__setitem__", [](DatTable& self, std::pair<const std::string&, uint32_t> index, const std::string& value) 
			{ self.setCell(index.first, index.second, value); }, set_cellDoc)
		.def("__setitem__", [](DatTable& self, std::pair<uint32_t, const std::string&> index, const std::string& value) 
			{ self.setCell(index.first, index.second, value); }, set_cellDoc)
		.def("__setitem__", [](DatTable& self, std::pair<const std::string&, const std::string&> index, const std::string& value) 
			{ self.setCell(index.first, index.second, value); }, set_cellDoc)

		.def_prop_rw("num_rows", 
			[](DatTable& self) { return self.numRows(); }, 
			[](DatTable& self, uint32_t numRows) { self.setNumRows(numRows); }, 
			num_rowsDoc)
		.def_prop_rw("num_cols", 
			[](DatTable& self) { return self.numCols(); }, 
			[](DatTable& self, uint32_t numCols) { self.setNumCols(numCols); }, 
			num_colsDoc)

		.def("row", nb::overload_cast<uint32_t>(&DatTable::row), "index"_a, rowDoc, nb::rv_policy::reference_internal)
		.def("row", nb::overload_cast<const std::string&>(&DatTable::row), "name"_a, rowDoc, nb::rv_policy::reference_internal)

		.def("col", nb::overload_cast<uint32_t>(&DatTable::col), "index"_a, colDoc, nb::rv_policy::reference_internal)
		.def("col", nb::overload_cast<const std::string&>(&DatTable::col), "name"_a, colDoc, nb::rv_policy::reference_internal)

		.def("cell", nb::overload_cast<uint32_t, uint32_t>(&DatTable::cell), 
			"row"_a, "col"_a, cellDoc, nb::rv_policy::reference_internal)
		.def("cell", nb::overload_cast<const std::string&, uint32_t>(&DatTable::cell), 
			"row_name"_a, "col"_a, cellDoc, nb::rv_policy::reference_internal)
		.def("cell", nb::overload_cast<uint32_t, const std::string&>(&DatTable::cell), 
			"row"_a, "col_name"_a, cellDoc, nb::rv_policy::reference_internal)
		.def("cell", nb::overload_cast<const std::string&, const std::string&>(&DatTable::cell), 
			"row_name"_a, "col_name"_a, cellDoc, nb::rv_policy::reference_internal)

		.def("resize",       &DatTable::resize, "num_rows"_a, "num_cols"_a, reseizeDoc)
		.def("set_num_rows", &DatTable::setNumRows, "num_rows"_a, set_num_rowsDoc)
		.def("set_num_cols", &DatTable::setNumCols, "num_cols"_a, set_num_colsDoc)

		.def("set_cell", nb::overload_cast<size_t, size_t, const std::string&>(&DatTable::setCell), 
			"i"_a, "j"_a, "value"_a, set_cellDoc)
		.def("set_cell", nb::overload_cast<const std::string&, size_t, const std::string&>(&DatTable::setCell), 
			"rowName"_a, "j"_a, "value"_a, set_cellDoc)
		.def("set_cell", nb::overload_cast<size_t, const std::string&, const std::string&>(&DatTable::setCell), 
			"i"_a, "colName"_a, "value"_a, set_cellDoc)
		.def("set_cell", nb::overload_cast<const std::string&, const std::string&, const std::string&>(&DatTable::setCell), 
			"rowName"_a, "colName"_a, "value"_a, set_cellDoc)

		.def("set_row", nb::overload_cast<size_t, const std::vector<            std::string> &>(&DatTable::setRow), 
			"i"_a, "row"_a, set_rowDoc)
		.def("set_row", nb::overload_cast<const std::string&, const std::vector<std::string> &>(&DatTable::setRow), 
			"name"_a, "row"_a, set_rowDoc)

		.def("set_col", nb::overload_cast<size_t, const std::vector<            std::string> &>(&DatTable::setCol), 
			"j"_a, "col"_a, set_colDoc)
		.def("set_col", nb::overload_cast<const std::string&, const std::vector<std::string> &>(&DatTable::setCol), 
			"name"_a, "col"_a, set_colDoc)

		.def("append_row", &DatTable::appendRow, "row"_a        = nb::list { }, append_rowDoc)
		.def("append_col", &DatTable::appendCol, "col"_a        = nb::list { }, append_colDoc)
		.def("insert_row", &DatTable::insertRow, "i"_a, "row"_a = nb::list { }, insert_rowDoc)
		.def("insert_col", &DatTable::insertCol, "j"_a, "col"_a = nb::list { }, insert_colDoc)

		.def("remove_row", nb::overload_cast<size_t>(&DatTable::removeRow), "i"_a, remove_rowDoc)
		.def("remove_row", nb::overload_cast<const std::string&>(&DatTable::removeRow), "name"_a, remove_rowDoc)

		.def("remove_col", nb::overload_cast<size_t>(&DatTable::removeCol), "j"_a, remove_colDoc)
		.def("remove_col", nb::overload_cast<const std::string&>(&DatTable::removeCol), "name"_a, remove_colDoc)

		.def("clear",        &DatTable::clear, clearDoc)

		.def("as_list", 
			[](DatTable& self) {
				std::vector<std::vector<std::string_view>> list;
				for (size_t i = 0; i < self.numRows(); ++i)
				{
					std::vector<std::string_view> row;
					for (size_t j = 0; j < self.numCols(); ++j)
						row.push_back(self.cell(i, j));
					list.push_back(row);
				}
				return list;
			},
			as_listDoc, nb::rv_policy::reference_internal)

		.def("as_string", &DatTable::asString, as_stringDoc, nb::rv_policy::reference_internal)

		.def("from_list", 
			[](DatTable& self, const nb::list& list, bool cast = false) {
				self = tableFromList(list, cast);
			}, 
			"list"_a, "cast"_a = false, from_listDoc);

	nb::class_<OutDatLink> outDat(m, "OutDat");
	outDat.doc() = "An interface for an OutDAT in a loaded TouchDesigner component";
	outDat.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("as_table", &OutDatLink::asTable, as_tableDoc, nb::rv_policy::reference_internal)
		.def("as_string", &OutDatLink::asString, as_stringDoc, nb::rv_policy::reference_internal)
		;

	nb::class_<OutDatLinks> outDats(m, "OutDats");
	outDats.doc() = "A container of OutDat objects.";
	outDats.def(nb::init<>())
		.def_prop_ro("count", [](OutDatLinks& self) { return self.size(); }, countDocOutDat )
		.def_prop_ro("names", [](OutDatLinks& self) { return self.getLinkNames(); }, namesDocOutDat)
		.def("__getitem__", [](OutDatLinks& self, const std::string& name) 
			{ return self.getLinkByName(name); }, nb::rv_policy::reference_internal)
		.def("__getitem__", [](OutDatLinks& self, size_t index) 
			{ return self.getLinkByIndex(index); }, nb::rv_policy::reference_internal)
		;

	nb::class_<InDatLink> inDat(m, "InDat");
	inDat.doc() = "An interface for an InDAT in a loaded TouchDesigner component";
	inDat.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("from_table", nb::overload_cast<const DatTable&>(&InDatLink::set), from_tableDoc)
		.def("from_string", nb::overload_cast<const std::string&>(&InDatLink::set), from_stringDoc)
		;

	inDat.def("from_list", [](InDatLink& self, const nb::list& list, bool cast = false)
		{
			self.set(tableFromList(list, cast));
		}, "list"_a, "cast"_a = false,
	from_listDoc);

	nb::class_<InDatLinks> inDats(m, "InDats");
	inDats.doc() = "A container of InDat objects.";
	inDats.def(nb::init<>())
		.def_prop_ro("count", [](InDatLinks& self) { return self.size(); }, countDocInDat )
		.def_prop_ro("names", [](InDatLinks& self) { return self.getLinkNames(); }, namesDocInDat)
		.def("__getitem__", [](InDatLinks& self, const std::string& name) 
			{ return self.getLinkByName(name); }, nb::rv_policy::reference_internal)
		.def("__getitem__", [](InDatLinks& self, size_t index) 
			{ return self.getLinkByIndex(index); }, nb::rv_policy::reference_internal)
		;
}