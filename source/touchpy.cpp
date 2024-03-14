#include <nanobind/nanobind.h>
#include <nanobind/stl/unique_ptr.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/string_view.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/array.h>
#include <nanobind/stl/function.h>
#include <nanobind/stl/variant.h>
#include <nanobind/ndarray.h>
#include <nanobind/operators.h>


#include "comp.h"
#include "toplink.h"
#include "choplink.h"
#include "datlink.h"
#include "parlink.h"

#include <memory>

namespace nb = nanobind;
using namespace nb::literals;


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

CUDADataType cudaDataTypeFromDtype(nb::dlpack::dtype dtype)
{
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

template<typename ...Args>
nb::ndarray<Args...> arrayFromCudaMem(OutTopLink& outTopLink, ComponentMask componentMask = ComponentMask::RGBA)
{
	const auto& cudaMem = outTopLink.cudaMemory();
	auto numActiveComps = std::min(numActiveComponents(componentMask), cudaMem.shape.numComponents);
	auto compSize = static_cast<int64_t>(cudaMem.shape.componentSize);

	return nb::ndarray<Args...>(
		cudaMem.ptr,
		{ cudaMem.shape.height, cudaMem.shape.width, numActiveComps },
		nb::handle(),
		{   
			// dlpack strides are in elements, not bytes so we can't use the strides from 
			// the CUDAMemoryShape unless we divide by the component size or set directly
			cudaMem.shape.numComponents * cudaMem.shape.width, // distance to next row
			cudaMem.shape.numComponents, // distance to next pixel
			1 // distance to next component
		},

		dtypeFromCUDADataType(cudaMem.shape.dataType),
		nb::device::cuda::value,
		0
	);
}

template<typename T>
void copyArrayToCudaMemory(InTopLink& inTopLink, T array)
{
	if (array.is_valid())
	{
		CUDAMemory cudaMemory;
		cudaMemory.ptr = array.data();
		cudaMemory.size = array.size();

		auto shape = array.shape_ptr();
		auto strides = array.stride_ptr();

		CUDAMemoryShape memoryShape;
		memoryShape.width = shape[1];
		memoryShape.height = shape[0];
		memoryShape.numComponents = shape[2];
		memoryShape.componentSize = array.itemsize();
		memoryShape.dataType = cudaDataTypeFromDtype(array.dtype());

		// dlpack strides are in elements, not bytes so we need to convert
		memoryShape.strides[0] = strides[2] * memoryShape.numComponents;
		memoryShape.strides[1] = strides[1] * memoryShape.numComponents;
		memoryShape.strides[2] = strides[0] * memoryShape.numComponents;

		cudaMemory.shape = memoryShape;

		inTopLink.copyCudaMemory(cudaMemory, nullptr);
	}
}

using arrayShape4 = nb::shape<nb::any, nb::any, 4>;
using arrayShape3 = nb::shape<nb::any, nb::any, 3>;
using arrayShape2 = nb::shape<nb::any, nb::any, 2>;
using arrayShape1 = nb::shape<nb::any, nb::any, 1>;

void fromNumpyToChopLink(
	InChopLink& inChopLink, 
	nb::ndarray<float, nb::ndim<2>, nb::device::cpu> array,
	const std::vector<std::string>& names)
{
	auto view = array.view();
	int32_t channelCount = static_cast<int32_t>(view.shape(0));
	uint32_t valueCount = static_cast<uint32_t>(view.shape(1));
	std::vector<const float*> chanPtrs(channelCount);

	if (names.size() != channelCount)
	{
		for (int32_t i = 0; i < channelCount; ++i)
			chanPtrs[i] = view.data() + i * valueCount;

		inChopLink.set(chanPtrs.data(), channelCount, valueCount);
	}
	else
	{
		std::vector<const char*> namePtrs(channelCount);
		for (int32_t i = 0; i < channelCount; ++i)
		{
			chanPtrs[i] = view.data() + i * valueCount;
			namePtrs[i] = names[i].c_str();
		}
		inChopLink.set(chanPtrs.data(), channelCount, valueCount, -1.0, namePtrs.data());
	}
}

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

	nb::enum_<ComponentMask>(m, "ComponentMask")
		.value("None", ComponentMask::None)
		.value("R",    ComponentMask::R)
		.value("G",    ComponentMask::G)
		.value("B",    ComponentMask::B)
		.value("A",    ComponentMask::A)
		.value("RG",   ComponentMask::RG)
		.value("RGB",  ComponentMask::RGB)
		.value("RGBA", ComponentMask::RGBA)
		.def(nb::self | nb::self)
		.def(nb::self & nb::self)
		.def(nb::self ^ nb::self)
		.def(~nb::self)
		.def(nb::self |= nb::self)
		.def(nb::self &= nb::self)
		.def(nb::self ^= nb::self)
		.def(!nb::self)
		.def(nb::self == nb::self)
		.def(nb::self != nb::self)
		;

	nb::enum_<CUDADataType>(m, "CUDADataType")
		.value("UInt8",     CUDADataType::UInt8)
		.value("Float16",   CUDADataType::Float16)
		.value("Float32",   CUDADataType::Float32)
		.value("Undefined", CUDADataType::Undefined)
		;

	nb::class_<CUDAMemoryShape> cudaMemoryShape(m, "CudaMemoryShape");
	cudaMemoryShape.def(nb::init<>())
		.def_rw("width",          &CUDAMemoryShape::width)
		.def_rw("height",         &CUDAMemoryShape::height)
		.def_rw("num_components", &CUDAMemoryShape::numComponents)
		.def_rw("component_size", &CUDAMemoryShape::componentSize)
		.def_rw("data_type",      &CUDAMemoryShape::dataType)
		.def_rw("strides",        &CUDAMemoryShape::strides)
		.def("__repr__", []       (CUDAMemoryShape& self)
			{
				return "CudaMemoryShape(" + std::to_string(self.width) + ", " + std::to_string(self.height) 
					+ ", " + std::to_string(self.numComponents) + ", " + std::to_string(self.componentSize) 
					+ ", " + cudaDataTypeToString(self.dataType) + ")";
			}
		);

	nb::class_<CUDAMemory> cudaMemory(m, "CudaMemory");
	cudaMemory.doc() = "A pointer to CUDA memory and its attributes";
	cudaMemory.def(nb::init<>())
		.def_ro("size",       &CUDAMemory::size, nb::rv_policy::reference_internal)
		.def_rw("shape",      &CUDAMemory::shape, nb::rv_policy::reference_internal)
		.def_prop_ro("ptr", [](CUDAMemory& self) -> uintptr_t
			{ return reinterpret_cast<uintptr_t>(self.ptr); }, nb::rv_policy::reference_internal);


	nb::class_<OutTopLink> outTopLink(m, "OutTopLink");
	outTopLink.doc() = "Represents an OutTOP in a TouchDesigner component";
	outTopLink.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("cuda_memory", &OutTopLink::cudaMemory)
		.def("as_dlpack", &arrayFromCudaMem<>, "componentMask"_a = ComponentMask::RGBA, nb::rv_policy::reference_internal)
		.def("as_tensor", &arrayFromCudaMem<nb::pytorch>, "componentMask"_a = ComponentMask::RGBA, nb::rv_policy::reference_internal)
		;

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

	inTopLink.def("from_dlpack", 
			[](InTopLink& self, nb::ndarray<arrayShape4, nb::device::cuda> array)
			{ copyArrayToCudaMemory<nb::ndarray<arrayShape4, nb::device::cuda>>(self, array); })
		.def("from_dlpack",
			[](InTopLink& self, nb::ndarray<arrayShape3, nb::device::cuda> array)
			{ copyArrayToCudaMemory<nb::ndarray<arrayShape3, nb::device::cuda>>(self, array); })
		.def("from_dlpack",
			[](InTopLink& self, nb::ndarray<arrayShape2, nb::device::cuda> array)
			{ copyArrayToCudaMemory<nb::ndarray<arrayShape2, nb::device::cuda>>(self, array); })
		.def("from_dlpack",
			[](InTopLink& self, nb::ndarray<arrayShape1, nb::device::cuda> array)
			{ copyArrayToCudaMemory<nb::ndarray<arrayShape1, nb::device::cuda>>(self, array); })
		;

	inTopLink.def("from_tensor",
			[](InTopLink& self, nb::ndarray<nb::pytorch, arrayShape4, nb::device::cuda> array)
			{ copyArrayToCudaMemory<nb::ndarray<nb::pytorch, arrayShape4, nb::device::cuda>>(self, array); })
		.def("from_tensor",
			[](InTopLink& self, nb::ndarray<nb::pytorch, arrayShape3, nb::device::cuda> array)
			{ copyArrayToCudaMemory<nb::ndarray<nb::pytorch, arrayShape3, nb::device::cuda>>(self, array); })
		.def("from_tensor",
			[](InTopLink& self, nb::ndarray<nb::pytorch, arrayShape2, nb::device::cuda> array)
			{ copyArrayToCudaMemory<nb::ndarray<nb::pytorch, arrayShape2, nb::device::cuda>>(self, array); })
		.def("from_tensor",
			[](InTopLink& self, nb::ndarray<nb::pytorch, arrayShape1, nb::device::cuda> array)
			{ copyArrayToCudaMemory<nb::ndarray<nb::pytorch, arrayShape1, nb::device::cuda>>(self, array); })
		;

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
	inChopLink.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("from_numpy", &fromNumpyToChopLink, "array"_a, "names"_a = nb::list());

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

	nb::class_ <Int2ParLink> int2ParLink(m, "Int2ParLink");
	int2ParLink.doc() = "Represents an int2 parameter in a TouchDesigner component";
	int2ParLink.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("set", &Int2ParLink::set)
		.def("get", &Int2ParLink::get)
		.def_prop_rw("val", &Int2ParLink::get, &Int2ParLink::set)
		.def_prop_rw("x", &Int2ParLink::getX, &Int2ParLink::setX)
		.def_prop_rw("y", &Int2ParLink::getY, &Int2ParLink::setY)
		.def("set", [](Int2ParLink& self, int32_t x = 0, int32_t y = 0) { self.set(Int2(x, y)); }, "x"_a = 0, "y"_a = 0)
		.def("set", [](Int2ParLink& self, const nb::list& list)
			{
				if (list.size() != 2)
					throw std::invalid_argument("List must have 2 elements");

				auto val = Int2(
					nb::cast<int32_t>(list[0]),
					nb::cast<int32_t>(list[1])
				);
				self.set(val);
			})
		;

	nb::class_ <Int3ParLink> int3ParLink(m, "Int3ParLink");
	int3ParLink.doc() = "Represents an int3 parameter in a TouchDesigner component";
	int3ParLink.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("set", &Int3ParLink::set)
		.def("get", &Int3ParLink::get)
		.def_prop_rw("val", &Int3ParLink::get, &Int3ParLink::set)
		.def_prop_rw("x", &Int3ParLink::getX, &Int3ParLink::setX)
		.def_prop_rw("y", &Int3ParLink::getY, &Int3ParLink::setY)
		.def_prop_rw("z", &Int3ParLink::getZ, &Int3ParLink::setZ)
		.def("set", [](Int3ParLink& self, int32_t x = 0, int32_t y = 0, int32_t z = 0) 
			{ 
				self.set(Int3(x, y, z)); 
			}, "x"_a = 0, "y"_a = 0, "z"_a = 0)

		.def("set", [](Int3ParLink& self, const nb::list& list)
			{
				if (list.size() != 3)
					throw std::invalid_argument("List must have 3 elements");

				auto val = Int3(
					nb::cast<int32_t>(list[0]),
					nb::cast<int32_t>(list[1]),
					nb::cast<int32_t>(list[2])
				);
				self.set(val);
			})
		;

	nb::class_ <Int4ParLink> int4ParLink(m, "Int4ParLink");
	int4ParLink.doc() = "Represents an int4 parameter in a TouchDesigner component";
	int4ParLink.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("set", &Int4ParLink::set)
		.def("get", &Int4ParLink::get)
		.def_prop_rw("val", &Int4ParLink::get, &Int4ParLink::set)
		.def_prop_rw("x", &Int4ParLink::getX, &Int4ParLink::setX)
		.def_prop_rw("y", &Int4ParLink::getY, &Int4ParLink::setY)
		.def_prop_rw("z", &Int4ParLink::getZ, &Int4ParLink::setZ)
		.def_prop_rw("w", &Int4ParLink::getW, &Int4ParLink::setW)
		.def("set", [](Int4ParLink& self, int32_t x = 0, int32_t y = 0, int32_t z = 0, int32_t w = 0) 
			{ 
				self.set(Int4(x, y, z, w)); 
			}, "x"_a = 0, "y"_a = 0, "z"_a = 0, "w"_a = 0)

		.def("set", [](Int4ParLink& self, const nb::list& list)
			{
				if (list.size() != 4)
					throw std::invalid_argument("List must have 4 elements");

				auto val = Int4(
					nb::cast<int32_t>(list[0]),
					nb::cast<int32_t>(list[1]),
					nb::cast<int32_t>(list[2]),
					nb::cast<int32_t>(list[3])
				);
				self.set(val);
			})
		;

	nb::class_ <Double2ParLink> double2ParLink(m, "Double2ParLink");
	double2ParLink.doc() = "Represents a float2 parameter in a TouchDesigner component";
	double2ParLink.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("set", &Double2ParLink::set)
		.def("get", &Double2ParLink::get)
		.def_prop_rw("val", &Double2ParLink::get, &Double2ParLink::set)
		.def_prop_rw("x", &Double2ParLink::getX, &Double2ParLink::setX)
		.def_prop_rw("y", &Double2ParLink::getY, &Double2ParLink::setY)
		.def("set", [](Double2ParLink& self, double x = 0., double y = 0.) 
			{ 
				self.set(Double2(x, y)); 
			}, "x"_a = 0., "y"_a = 0.)

		.def("set", [](Double2ParLink& self, const nb::list& list)
			{
				if (list.size() != 2)
					throw std::invalid_argument("List must have 2 elements");

				auto val = Double2(
					nb::cast<double>(list[0]),
					nb::cast<double>(list[1])
				);
				self.set(val);
			})
		;

	nb::class_ <Double3ParLink> double3ParLink(m, "Double3ParLink");
	double3ParLink.doc() = "Represents a float3 parameter in a TouchDesigner component";
	double3ParLink.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("set", &Double3ParLink::set)
		.def("get", &Double3ParLink::get)
		.def_prop_rw("val", &Double3ParLink::get, &Double3ParLink::set)
		.def_prop_rw("x", &Double3ParLink::getX, &Double3ParLink::setX)
		.def_prop_rw("y", &Double3ParLink::getY, &Double3ParLink::setY)
		.def_prop_rw("z", &Double3ParLink::getZ, &Double3ParLink::setZ)
		.def("set", [](Double3ParLink& self, double x = 0., double y = 0., double z = 0.) 
			{ 
				self.set(Double3(x, y, z)); 
			}, "x"_a = 0., "y"_a = 0., "z"_a = 0.)
		.def("set", [](Double3ParLink& self, const nb::list& list)
			{
				if (list.size() != 3)
					throw std::invalid_argument("List must have 3 elements");

				auto val = Double3(
					nb::cast<double>(list[0]),
					nb::cast<double>(list[1]),
					nb::cast<double>(list[2])
				);
				self.set(val);
			}, "values"_a)
		;

	nb::class_ <Double4ParLink> double4ParLink(m, "Double4ParLink");
	double4ParLink.doc() = "Represents a float4 parameter in a TouchDesigner component";
	double4ParLink.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("set", &Double4ParLink::set)
		.def("get", &Double4ParLink::get)
		.def_prop_rw("val", &Double4ParLink::get, &Double4ParLink::set)
		.def_prop_rw("x", &Double4ParLink::getX, &Double4ParLink::setX)
		.def_prop_rw("y", &Double4ParLink::getY, &Double4ParLink::setY)
		.def_prop_rw("z", &Double4ParLink::getZ, &Double4ParLink::setZ)
		.def_prop_rw("w", &Double4ParLink::getW, &Double4ParLink::setW)
		.def("set", [](Double4ParLink& self, double x = 0., double y = 0., double z = 0., double w = 0.)
			{
				self.set(Double4(x, y, z, w));
			}, "x"_a = 0., "y"_a = 0., "z"_a = 0., "w"_a = 0.)

		.def("set", [](Double4ParLink& self, const nb::list& list)
			{ 
				if (list.size() != 4)
					throw std::invalid_argument("List must have 4 elements");

				auto val = Double4(
					nb::cast<double>(list[0]), 
					nb::cast<double>(list[1]), 
					nb::cast<double>(list[2]), 
					nb::cast<double>(list[3])
				);
				self.set(val); 
			})
		;

	nb::class_ <ColorParLink> colorParLink(m, "ColorParLink");
	colorParLink.doc() = "Represents a color parameter in a TouchDesigner component";
	colorParLink.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("set", &ColorParLink::set)
		.def("get", &ColorParLink::get)
		.def_prop_rw("val", &ColorParLink::get, &ColorParLink::set)
		.def_prop_rw("r", &ColorParLink::getR, &ColorParLink::setR)
		.def_prop_rw("g", &ColorParLink::getG, &ColorParLink::setG)
		.def_prop_rw("b", &ColorParLink::getB, &ColorParLink::setB)
		.def_prop_rw("a", &ColorParLink::getA, &ColorParLink::setA)
		.def("set", [](ColorParLink& self, double r = 1., double g = 1., double b = 1., double a = 1.)
			{
				self.set(ColorRGBA(r, g, b, a));
			}, "r"_a = 1., "g"_a = 1., "b"_a = 1., "a"_a = 1.)

		.def("set", [](ColorParLink& self, const nb::list& list)
			{
				if (list.size() != 4)
					throw std::invalid_argument("List must have 4 elements");

				auto val = ColorRGBA(
					nb::cast<double>(list[0]),
					nb::cast<double>(list[1]),
					nb::cast<double>(list[2]),
					nb::cast<double>(list[3])
				);
				self.set(val);
			})
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