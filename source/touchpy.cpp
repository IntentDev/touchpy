#include <nanobind/nanobind.h>
#include <nanobind/stl/unique_ptr.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/string_view.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/function.h>
#include <nanobind/stl/variant.h>
#include <nanobind/ndarray.h>


#include "comp.h"
#include "toplink.h"
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

CUDADataType cudaDataTypeFromArray(nb::ndarray<> array)
{
	auto dtype = array.dtype();
	if (dtype.code == static_cast<uint8_t>(nb::dlpack::dtype_code::UInt))
	{
		if (dtype.bits == 8)
			return CUDADataType::UInt8;
	}
	else if (dtype.code == static_cast<uint8_t>(nb::dlpack::dtype_code::Float))
	{
		if (dtype.bits == 16)
			return CUDADataType::Float16;
		else if (dtype.bits == 32)
			return CUDADataType::Float32;
	}
	return CUDADataType::Undefined;
}

nb::dlpack::dtype dtypeFromCUDADataType(CUDADataType type)
{
	nb::dlpack::dtype dtype;
	dtype.lanes = 1;
	switch (type)
	{
	case CUDADataType::UInt8:
		dtype.code = static_cast<uint8_t>(nb::dlpack::dtype_code::UInt);
		dtype.bits = 8;
		break;
	case CUDADataType::Float32:
		dtype.code = static_cast<uint8_t>(nb::dlpack::dtype_code::Float);
		dtype.bits = 32;
		break;
	case CUDADataType::Float16:
		dtype.code = static_cast<uint8_t>(nb::dlpack::dtype_code::Float);
		dtype.bits = 16;
		break;
	default:
		dtype.code = static_cast<uint8_t>(nb::dlpack::dtype_code::UInt);
		dtype.bits = 8;
		break;
	}
	return dtype;
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
		.def_prop_ro("in_tops", &Comp::inputTopLinks, nb::rv_policy::reference_internal)
		.def_prop_ro("out_tops", &Comp::outputTopLinks, nb::rv_policy::reference_internal)
		.def_prop_ro("in_chops", &Comp::inChopLinks, nb::rv_policy::reference_internal)
		.def_prop_ro("out_chops", &Comp::outChopLinks, nb::rv_policy::reference_internal)
		.def_prop_ro("in_dats", &Comp::inDatLinks, nb::rv_policy::reference_internal)
		.def_prop_ro("out_dats", &Comp::outDatLinks, nb::rv_policy::reference_internal)
		.def_prop_ro("par", &Comp::parLinks, nb::rv_policy::reference_internal)
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
	// TopLinks
	//--------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------

	//nb::class_<cudaStream_t>(m, "cudaStream");



	nb::class_<CUDAMemory> cudaMemory(m, "CudaMemory");
	cudaMemory.doc() = "Represents a memory block on the GPU";
	cudaMemory.def(nb::init<>());
	cudaMemory.def_prop_ro("ptr", [](CUDAMemory& self) -> uintptr_t 
		{ return reinterpret_cast<uintptr_t>(self.ptr); }, nb::rv_policy::reference_internal);
	cudaMemory.def_ro("size", &CUDAMemory::size, nb::rv_policy::reference_internal);

	nb::class_<OutTopLink> outTopLink(m, "OutTopLink");
	outTopLink.doc() = "Represents an OutTOP in a TouchDesigner component";
	outTopLink.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>());
	outTopLink.def("cuda_memory", &OutTopLink::cudaMemory);

	//outTopLink.def("as_dlpack", [](OutTopLink& self) 
	//	{ 
	//		auto shape = self.shape();
	//		// (numBytes, num_bytes_px * self.w, num_bytes_px)
	//		const std::array<int64_t, 3> strides = { 
	//			static_cast<int64_t>(shape[0] * shape[1] * shape[2]),
	//			static_cast<int64_t>(shape[0] * shape[2]),
	//			static_cast<int64_t>(shape[2])
	//		};

	//		nb::dlpack::dtype dtype;
	//		dtype.code = static_cast<uint8_t>(nb::dlpack::dtype_code::UInt);
	//		dtype.bits = 8;
	//		dtype.lanes = 1;

	//		void* data = self.cudaMemory();

	//		//nb::pytorch, uint8_t, nb::ndim<3>, const size_t*, nb::handle, const int64_t*, nb::dlpack::dtype, int32_t, int32_t
	//		return nb::ndarray<uint8_t, nb::ndim<3>>(
	//			self.cudaMemory(),
	//			3u, 
	//			shape.data(),
	//			nb::handle(),
	//			strides.data(),
	//			dtype,
	//			nb::device::cuda::value,
	//			0
	//		);
	//	}, nb::rv_policy::reference_internal);

	//outTopLink.def("as_tensor", [](OutTopLink& self)
	//	{
	//		auto shape = self.shape();
	//		// (numBytes, num_bytes_px * self.w, num_bytes_px)
	//		const std::array<int64_t, 3> strides = {
	//			static_cast<int64_t>(shape[0] * shape[1] * shape[2]),
	//			static_cast<int64_t>(shape[0] * shape[2]),
	//			static_cast<int64_t>(shape[2])
	//		};

	//		nb::dlpack::dtype dtype;
	//		dtype.code = static_cast<uint8_t>(nb::dlpack::dtype_code::UInt);
	//		dtype.bits = 8;
	//		dtype.lanes = 1;

	//		void* data = self.cudaMemory().ptr;

	//		//nb::pytorch, uint8_t, nb::ndim<3>, const size_t*, nb::handle, const int64_t*, nb::dlpack::dtype, int32_t, int32_t
	//		return nb::ndarray<nb::pytorch, uint8_t, nb::ndim<3>>(
	//			&data,
	//			3u,
	//			shape.data(),
	//			nb::handle(),
	//			strides.data(),
	//			dtype,
	//			nb::device::cuda::value,
	//			0
	//		);
	//	}, nb::rv_policy::reference_internal);

	nb::class_<OutTopLinks> outTopLinks(m, "OutTopLinks");
	outTopLinks.doc() = "Represents a collection of OutTOP links in a TouchDesigner component";
	outTopLinks.def(nb::init<>())
		.def("num_links", &OutTopLinks::size)
		.def("link_names", &OutTopLinks::getLinkNames)
		.def("__getitem__", [](OutTopLinks& self, const std::string& name) { return self.getLinkByName(name); }, nb::rv_policy::reference_internal)
		.def("__getitem__", [](OutTopLinks& self, size_t index) { return self.getLinkByIndex(index); }, nb::rv_policy::reference_internal)
		;

	nb::class_<InTopLink> inTopLink(m, "InTopLink");
	inTopLink.doc() = "Represents an InTOP in a TouchDesigner component";
	inTopLink.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>());
	//inTopLink.def("from_dlpack", [](
	//	InTopLink& self, 
	//	nb::ndarray<uint8_t, nb::ndim<3>, nb::device::cuda> array, 
	//	uint32_t width, 
	//	uint32_t height)
	//	{
	//		self.copyCudaMemory(array.data(), width, height, 4, nullptr);
	//	});

	//inTopLink.def("from_tensor", [](InTopLink& self, nb::ndarray<nb::pytorch, uint8_t, nb::ndim<3>> array, uint32_t width, uint32_t height)
	//	{
	//		self.copyCudaMemory(array.data(), width, height, 4, nullptr);
	//	});

	inTopLink.def("copy_cuda_memory", [](InTopLink& self, const CUDAMemory& memory)
		{
			self.copyCudaMemory(memory, nullptr);
		});

	nb::class_<InTopLinks> inTopLinks(m, "InTopLinks");
	inTopLinks.doc() = "Represents a collection of InTOP links in a TouchDesigner component";
	inTopLinks.def(nb::init<>())
		.def("num_links", &InTopLinks::size)
		.def("link_names", &InTopLinks::getLinkNames)
		.def("__getitem__", [](InTopLinks& self, const std::string& name) { return self.getLinkByName(name); }, nb::rv_policy::reference_internal)
		.def("__getitem__", [](InTopLinks& self, size_t index) { return self.getLinkByIndex(index); }, nb::rv_policy::reference_internal)
		;




	// ChopLinks
	//--------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------

	nb::class_<OutChopLink> outChopLink(m, "OutChopLink");
	outChopLink.doc() = "Represents a in or out CHOP in a TouchDesigner component";
	outChopLink.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>());
	outChopLink.def("chan_names", &OutChopLink::names, nb::rv_policy::reference_internal);

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
		.def("__getitem__", [](OutChopLinks& self, const std::string& name) { return self.getLinkByName(name); }, nb::rv_policy::reference_internal)
		.def("__getitem__", [](OutChopLinks& self, size_t index) { return self.getLinkByIndex(index); }, nb::rv_policy::reference_internal)
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
		.def("__getitem__", [](InChopLinks& self, const std::string& name) { return self.getLinkByName(name); }, nb::rv_policy::reference_internal)
		.def("__getitem__", [](InChopLinks& self, size_t index) { return self.getLinkByIndex(index); }, nb::rv_policy::reference_internal)
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
		.def("__getitem__", [](InDatLinks& self, const std::string& name) { return self.getLinkByName(name); }, nb::rv_policy::reference_internal)
		.def("__getitem__", [](InDatLinks& self, size_t index) { return self.getLinkByIndex(index); }, nb::rv_policy::reference_internal)
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
		.def("__getitem__", [](OutDatLinks& self, const std::string& name) { return self.getLinkByName(name); }, nb::rv_policy::reference_internal)
		.def("__getitem__", [](OutDatLinks& self, size_t index) { return self.getLinkByIndex(index); }, nb::rv_policy::reference_internal)
		;

	// ParLinks
	//--------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------
	nb::class_<Int2> int2(m, "Int2");
	int2.def(nb::init<int32_t, int32_t>()).def_rw("x", &Int2::x).def_rw("y", &Int2::y);

	nb::class_<Int3> int3(m, "Int3");
	int3.def(nb::init<int32_t, int32_t>()).def_rw("x", &Int3::x).def_rw("y", &Int3::y).def_rw("z", &Int3::z);

	nb::class_<Int4> int4(m, "Int4");
	int4.def(nb::init<int32_t, int32_t>()).def_rw("x", &Int4::x).def_rw("y", &Int4::y).def_rw("z", &Int4::z).def_rw("w", &Int4::w);

	nb::class_<Double2> double2(m, "Float2");
	double2.def(nb::init<double, double>()).def_rw("x", &Double2::x).def_rw("y", &Double2::y);

	nb::class_<Double3> double3(m, "Float3");
	double3.def(nb::init<double, double, double>()).def_rw("x", &Double3::x).def_rw("y", &Double3::y).def_rw("z", &Double3::z);

	nb::class_<Double4> double4(m, "Float4");
	double4.def(nb::init<double, double, double, double>())
		.def_rw("x", &Double4::x).def_rw("y", &Double4::y).def_rw("z", &Double4::z).def_rw("w", &Double4::w);

	nb::class_<ColorRGBA> colorRGBA(m, "Color");
	colorRGBA.def(nb::init<double, double, double, double>(), "r"_a = 1.0, "g"_a = 1.0, "b"_a = 1.0, "a"_a = 1.0)
		.def_rw("r", &ColorRGBA::r).def_rw("g", &ColorRGBA::g).def_rw("b", &ColorRGBA::b).def_rw("a", &ColorRGBA::a);

	nb::class_ <ParLink> parLink(m, "ParLink");
	parLink.doc() = "Represents a parameter in a TouchDesigner component";
	parLink.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("set", &ParLink::set)
		.def("get", &ParLink::get)
		.def_prop_rw("val", &ParLink::get, &ParLink::set)
		;

	nb::class_<ParLinkCollection> parLinks(m, "ParLinkCollection");
	parLinks.doc() = "Represents a collection of parameters in a TouchDesigner component";
	parLinks.def(nb::init<>())
		.def("count", &ParLinkCollection::size)
		.def("names", &ParLinkCollection::getParNames)
		.def("__getitem__", [](ParLinkCollection& self, const std::string& name) -> std::shared_ptr<ParLink> 
			{ return self.getParLinkByName(name); }, nb::rv_policy::reference_internal)
		;
}