#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/string_view.h>
#include <nanobind/stl/vector.h>

#include "datlink.h"

namespace nb = nanobind;
using namespace nb::literals;

static const char* num_rowsDoc =
R"(The number of rows in the table.
)";

static const char* num_colsDoc =
R"(The number of columns in the table.
)";

static const char* rowDoc =
R"(Returns a list of values from the row matching the index.

Args:
	index (int) : the index of the row to return
)";


static const char* colDoc =
R"(Returns a list of values from the column matching the index.

Args:
	index (int) : the index of the column to return
)";

static const char* cellDoc =
R"(Returns the value at the row and column index.

Args:
	row (int) : the index of the row
	col (int) : the index of the column
)";

static const char* as_listDoc =
R"(Returns the table as a list of lists.
)";

static const char* from_listDoc =
R"(Fills the table from a list of lists.

Args:
	list (list) : the list of lists to fill the table from
	cast (bool) : if True, casts the values to strings (optional)
)";

static const char* as_tableDoc =
R"(Returns the Out DAT as touchpy.DatTable object
)";

static const char* as_stringDoc =
R"(Returns the Out DAT in string format.
)";

static const char* countDocOutDat =
R"(Returns the number of Out DATs in the loaded tox.
)";

static const char* namesDocOutDat =
R"(Returns a list of names of all Out DATs in the loaded tox.
)";

static const char* from_tableDoc =
R"(Fills the In DAT from a touchpy.DatTable object and sets it to Table DAT mode.
)";

static const char* from_stringDoc =
R"(Fills the In DAT from a string and sets it to Text DAT mode.
)";

static const char* countDocInDat =
R"(Returns the number of In DATs in the loaded tox.
)";

static const char* namesDocInDat =
R"(Returns a list of names of all In DATs in the loaded tox.
)";

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

		table.values.resize(numRows * numCols);
		table.numRows = numRows;
		table.numCols = numCols;

		if (cast)
		{
			for (size_t i = 0; i < numRows; ++i)
			{
				auto row = nb::cast<nb::list>(list[i]);
				for (size_t j = 0; j < numCols; ++j)
					if (nb::isinstance<nb::int_>(row[j]))
						table.values[i * numCols + j] = std::to_string(nb::cast<int>(row[j]));
					else if (nb::isinstance<nb::float_>(row[j]))
						table.values[i * numCols + j] = std::to_string(nb::cast<float>(row[j]));
					else if (nb::isinstance<nb::str>(row[j]))
						table.values[i * numCols + j] = nb::cast<std::string>(row[j]);
					else
						table.values[i * numCols + j] = "";
			}
		}
		else
		{
			for (size_t i = 0; i < numRows; ++i)
			{
				auto row = nb::cast<nb::list>(list[i]);
				for (size_t j = 0; j < numCols; ++j)
					table.values[i * numCols + j] = nb::cast<std::string>(row[j]);
			}
		}
	}
	return table;
}

void initDatLinkBindings(nb::module_& m)
{
	nb::class_<DatTable> datTable(m, "DatTable");
	datTable.doc() = "A table of data in a DAT link";
	datTable.def(nb::init<>())
		.def_prop_ro("num_rows", [](DatTable& self) { return self.numRows; })
		.def_prop_ro("num_cols", [](DatTable& self) { return self.numCols; })
		.def("row", &DatTable::row, "index"_a, rowDoc, nb::rv_policy::reference_internal)
		.def("col", &DatTable::col, "index"_a, colDoc, nb::rv_policy::reference_internal)
		.def("cell", &DatTable::cell, "row"_a, "col"_a, cellDoc, nb::rv_policy::reference_internal);

	datTable.def("as_list", [](DatTable& self)
		{
			std::vector<std::vector<std::string_view>> table;
			for (size_t i = 0; i < self.numRows; ++i)
			{
				std::vector<std::string_view> row;
				for (size_t j = 0; j < self.numCols; ++j)
					row.push_back(self.values[i * self.numCols + j]);
				table.push_back(row);
			}
			return table;
		},
		as_listDoc, nb::rv_policy::reference_internal);

	datTable.def("from_list", [](DatTable& self, const nb::list& list, bool cast = false)
		{
			self = tableFromList(list, cast);
		}, "list"_a, "cast"_a = false,
		from_listDoc);

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
		.def("__getitem__", [](OutDatLinks& self, const std::string& name) { return self.getLinkByName(name); }, nb::rv_policy::reference_internal)
		.def("__getitem__", [](OutDatLinks& self, size_t index) { return self.getLinkByIndex(index); }, nb::rv_policy::reference_internal)
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
		.def("__getitem__", [](InDatLinks& self, const std::string& name) { return self.getLinkByName(name); }, nb::rv_policy::reference_internal)
		.def("__getitem__", [](InDatLinks& self, size_t index) { return self.getLinkByIndex(index); }, nb::rv_policy::reference_internal)
		;
}