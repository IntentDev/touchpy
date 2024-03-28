#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/string_view.h>
#include <nanobind/stl/vector.h>

#include "datlink.h"

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
		.def("num_rows", [](DatTable& self) { return self.numRows; })
		.def("num_cols", [](DatTable& self) { return self.numCols; })
		.def("row", &DatTable::row, nb::rv_policy::reference_internal)
		.def("col", &DatTable::col, nb::rv_policy::reference_internal)
		.def("cell", &DatTable::cell, nb::rv_policy::reference_internal);

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
		nb::rv_policy::reference_internal);

	datTable.def("from_list", [](DatTable& self, const nb::list& list, bool cast = false)
		{
			self = tableFromList(list, cast);
		}, "list"_a, "cast"_a = false
	);

	nb::class_<OutDatLink> outDatLink(m, "OutDatLink");
	outDatLink.doc() = "A in or out DAT in a TouchDesigner component";
	outDatLink.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("as_table", &OutDatLink::asTable, nb::rv_policy::reference_internal)
		.def("as_string", &OutDatLink::asString, nb::rv_policy::reference_internal)
		;

	nb::class_<OutDatLinks> outDatLinks(m, "OutDatLinks");
	outDatLinks.doc() = "A collection of DAT links in a TouchDesigner component";
	outDatLinks.def(nb::init<>())
		.def_prop_ro("count", [](OutDatLinks& self) { return self.size(); } )
		.def_prop_ro("names", [](OutDatLinks& self) { return self.getLinkNames(); })
		.def("__getitem__", [](OutDatLinks& self, const std::string& name) { return self.getLinkByName(name); }, nb::rv_policy::reference_internal)
		.def("__getitem__", [](OutDatLinks& self, size_t index) { return self.getLinkByIndex(index); }, nb::rv_policy::reference_internal)
		;

	nb::class_<InDatLink> inDatLink(m, "InDatLink");
	inDatLink.doc() = "A in or out DAT in a TouchDesigner component";
	inDatLink.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("from_table", nb::overload_cast<const DatTable&>(&InDatLink::set))
		.def("from_string", nb::overload_cast<const std::string&>(&InDatLink::set))
		;

	inDatLink.def("from_list", [](InDatLink& self, const nb::list& list, bool cast = false)
		{
			self.set(tableFromList(list, cast));
		}, "list"_a, "cast"_a = false
	);

	nb::class_<InDatLinks> inDatLinks(m, "InDatLinks");
	inDatLinks.doc() = "A collection of DAT links in a TouchDesigner component";
	inDatLinks.def(nb::init<>())
		.def_prop_ro("count", [](InDatLinks& self) { return self.size(); } )
		.def_prop_ro("names", [](InDatLinks& self) { return self.getLinkNames(); })
		.def("__getitem__", [](InDatLinks& self, const std::string& name) { return self.getLinkByName(name); }, nb::rv_policy::reference_internal)
		.def("__getitem__", [](InDatLinks& self, size_t index) { return self.getLinkByIndex(index); }, nb::rv_policy::reference_internal)
		;
}