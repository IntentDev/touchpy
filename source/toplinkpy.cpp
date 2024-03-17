#include <nanobind/nanobind.h>
//#include <nanobind/stl/string.h>
#include <nanobind/ndarray.h>
#include <nanobind/operators.h>

#include "toplink.h"

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

void initTopLinkBindings(nb::module_& m)
{

	nb::enum_<ComponentMask>(m, "ComponentMask")
		.value("None", ComponentMask::None)
		.value("R", ComponentMask::R)
		.value("G", ComponentMask::G)
		.value("B", ComponentMask::B)
		.value("A", ComponentMask::A)
		.value("RG", ComponentMask::RG)
		.value("RGB", ComponentMask::RGB)
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
		.value("UInt8", CUDADataType::UInt8)
		.value("Float16", CUDADataType::Float16)
		.value("Float32", CUDADataType::Float32)
		.value("Undefined", CUDADataType::Undefined).doc() = "The data type of a CUDA memory buffer";
		;

	nb::class_<CUDAMemoryShape> cudaMemoryShape(m, "CudaMemoryShape");
	cudaMemoryShape.def(nb::init<>())
		.def_rw("width", &CUDAMemoryShape::width)
		.def_rw("height", &CUDAMemoryShape::height)
		.def_rw("num_components", &CUDAMemoryShape::numComponents)
		.def_rw("component_size", &CUDAMemoryShape::componentSize)
		.def_rw("data_type", &CUDAMemoryShape::dataType)
		.def_rw("strides", &CUDAMemoryShape::strides)
		.def("__repr__", [](CUDAMemoryShape& self)
			{
				return "CudaMemoryShape(" + std::to_string(self.width) + ", " + std::to_string(self.height)
					+ ", " + std::to_string(self.numComponents) + ", " + std::to_string(self.componentSize)
					+ ", " + cudaDataTypeToString(self.dataType) + ")";
			}
	);

	nb::class_<CUDAMemory> cudaMemory(m, "CudaMemory");
	cudaMemory.doc() = "A pointer to CUDA memory and its attributes";
	cudaMemory.def(nb::init<>())
		.def_ro("size", &CUDAMemory::size, nb::rv_policy::reference_internal)
		.def_rw("shape", &CUDAMemory::shape, nb::rv_policy::reference_internal)
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


}