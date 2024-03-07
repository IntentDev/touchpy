#include <nanobind/nanobind.h>
#include <nanobind/stl/unique_ptr.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/string_view.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/function.h>
#include <nanobind/ndarray.h>

#include "comp.h"
#include "texturelink.h"
#include "choplink.h"
#include "datlink.h"
#include "parlink.h"

#include <memory>

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




NB_MODULE(touchpy, m)
{

	// Comp
	//--------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------

	nb::class_<Comp> comp(m, "Comp");
	comp.doc() = "Runs a TouchDesigner component loaded from .tox file";
	comp.def(nb::init<>())
		.def(nb::init<const std::string&>(), nb::rv_policy::reference_internal)
		.def("load_tox", &Comp::loadTox, nb::rv_policy::reference_internal)
		.def("loaded", &Comp::loaded, nb::rv_policy::reference_internal)
		.def("update", &Comp::update, nb::rv_policy::reference_internal)
		.def("start", &Comp::runUpdateLoop, nb::rv_policy::reference_internal)
		.def("stop", &Comp::stopUpdateLoop, nb::rv_policy::reference_internal)
		.def_prop_ro("in_textures", &Comp::inputTextureLinks, nb::rv_policy::reference_internal)
		.def_prop_ro("out_textures", &Comp::outputTextureLinks, nb::rv_policy::reference_internal)
		.def_prop_ro("in_chops", &Comp::inChopLinks, nb::rv_policy::reference_internal)
		.def_prop_ro("out_chops", &Comp::outChopLinks, nb::rv_policy::reference_internal)
		.def_prop_ro("in_dats", &Comp::inDatLinks, nb::rv_policy::reference_internal)
		.def_prop_ro("out_dats", &Comp::outDatLinks, nb::rv_policy::reference_internal)
		.def_prop_ro("par_links", &Comp::parLinks, nb::rv_policy::reference_internal)
		;

	comp.def("set_on_frame_callback", [](Comp& self, nb::callable pythonCallback, nb::object userData)
		{
			auto userDataPtr = std::make_shared<nb::object>(userData);
			self.setOnFrameStartCallback([pythonCallback](Comp& comp, std::shared_ptr<void> userData)
				{
					auto& userDataPyObj = *std::static_pointer_cast<nb::object>(userData);
					pythonCallback(nb::cast(comp, nb::rv_policy::reference_internal), userDataPyObj);
				},
				userDataPtr);
		}
	);


	// ChopLinks
	//--------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------

	nb::class_<OutChopLink> outChopLink(m, "OutChopLink");
	outChopLink.doc() = "Represents a in or out CHOP in a TouchDesigner component";
	outChopLink.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>());

	outChopLink.def("as_numpy", [](OutChopLink& self) 
		{  
			size_t shape[2] = { static_cast<size_t>(self.channelCount()), static_cast<size_t>(self.valueCount()) };
			return nb::ndarray<nb::numpy, const float, nb::ndim<2>> (self.data(), 2, shape);
		},
		nb::rv_policy::automatic);

	outChopLink.def("as_numpy_ref", [](OutChopLink& self)
		{
			size_t shape[2] = { static_cast<size_t>(self.channelCount()), static_cast<size_t>(self.valueCount()) };
			return nb::ndarray<nb::numpy, const float, nb::ndim<2>>(self.data(), 2, shape);
		},
		nb::rv_policy::reference_internal);


	nb::class_<OutChopLinks> outChopLinks(m, "OutChopLinks");
	outChopLinks.doc() = "Represents a collection of CHOP links in a TouchDesigner component";
	outChopLinks.def(nb::init<>())
		.def("num_links", &OutChopLinks::size)
		.def("link_names", &OutChopLinks::getLinkNames)
		.def("__getitem__", [](OutChopLinks& self, const std::string& name) { return self.getLinkByName(name); }, nb::rv_policy::reference)
		.def("__getitem__", [](OutChopLinks& self, size_t index) { return self.getLinkByIndex(index); }, nb::rv_policy::reference)
		;

	nb::class_<InChopLink> inChopLink(m, "InChopLink");
	inChopLink.doc() = "Represents an inCHOP in a TouchDesigner component";
	inChopLink.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>());

	inChopLink.def("from_numpy", [](InChopLink& self, nb::ndarray<float, nb::ndim<2>, nb::device::cpu> array)
		{
			auto view = array.view();
			int32_t channelCount = static_cast<int32_t>(view.shape(0));
			uint32_t valueCount = static_cast<uint32_t>(view.shape(1));

			std::vector<const float*> chanPtrs(channelCount);
			for (int32_t i = 0; i < channelCount; ++i)
				chanPtrs[i] = view.data() + i * valueCount;

			self.set(chanPtrs.data(), channelCount, valueCount);
		});

	nb::class_<InChopLinks> inChopLinks(m, "InChopLinks");
	inChopLinks.doc() = "Represents a collection of CHOP links in a TouchDesigner component";
	inChopLinks.def(nb::init<>())
		.def("num_links", &InChopLinks::size)
		.def("link_names", &InChopLinks::getLinkNames)
		.def("__getitem__", [](InChopLinks& self, const std::string& name) { return self.getLinkByName(name); }, nb::rv_policy::reference)
		.def("__getitem__", [](InChopLinks& self, size_t index) { return self.getLinkByIndex(index); }, nb::rv_policy::reference)
		;


	// DatLinks
	//--------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------

	nb::class_<DatTable> datTable(m, "DatTable");
	datTable.doc() = "Represents a table of data in a DAT link";
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

	nb::class_<InDatLink> inDatLink(m, "InDatLink");
	inDatLink.doc() = "Represents a in or out DAT in a TouchDesigner component";
	inDatLink.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("from_table", nb::overload_cast<const DatTable&>(&InDatLink::set))
		.def("from_string", nb::overload_cast<const std::string&>(&InDatLink::set))
		.def("type_desc", &InDatLink::getTypeDescription)
		;

	inDatLink.def("from_list", [](InDatLink& self, const nb::list& list, bool cast = false)
		{
			self.set(tableFromList(list, cast));
		}, "list"_a, "cast"_a = false
	);

	nb::class_<InDatLinks> inDatLinks(m, "InDatLinks");
	inDatLinks.doc() = "Represents a collection of DAT links in a TouchDesigner component";
	inDatLinks.def(nb::init<>())
		.def("num_links", &InDatLinks::size)
		.def("link_names", &InDatLinks::getLinkNames)
		.def("__getitem__", [](InDatLinks& self, const std::string& name) { return self.getLinkByName(name); }, nb::rv_policy::reference)
		.def("__getitem__", [](InDatLinks& self, size_t index) { return self.getLinkByIndex(index); }, nb::rv_policy::reference)
		;

	nb::class_<OutDatLink> outDatLink(m, "OutDatLink");
	outDatLink.doc() = "Represents a in or out DAT in a TouchDesigner component";
	outDatLink.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("as_table", &OutDatLink::asTable)
		.def("as_string", &OutDatLink::asString)
		;

	nb::class_<OutDatLinks> outDatLinks(m, "OutDatLinks");
	outDatLinks.doc() = "Represents a collection of DAT links in a TouchDesigner component";
	outDatLinks.def(nb::init<>())
		.def("num_links", &OutDatLinks::size)
		.def("link_names", &OutDatLinks::getLinkNames)
		.def("__getitem__", [](OutDatLinks& self, const std::string& name) { return self.getLinkByName(name); }, nb::rv_policy::reference)
		.def("__getitem__", [](OutDatLinks& self, size_t index) { return self.getLinkByIndex(index); }, nb::rv_policy::reference)
		;

}