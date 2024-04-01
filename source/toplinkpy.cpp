#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/operators.h>

#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

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
nb::ndarray<Args...> arrayFromCudaMem(OutTopLink& outTopLink)
{
	const auto& cudaMem = outTopLink.cudaMemory();
	auto compSize = static_cast<int64_t>(cudaMem.desc.componentSize);

	return nb::ndarray<Args...>(
		cudaMem.ptr,
		{ cudaMem.desc.shape[0], cudaMem.desc.shape[1], cudaMem.desc.shape[2] },
		nb::handle(),
		{ cudaMem.desc.strides[0], cudaMem.desc.strides[1], cudaMem.desc.strides[2] },
		dtypeFromCUDADataType(cudaMem.desc.dataType),
		nb::device::cuda::value,
		0
	);
}

// need pass flags here and use in texture copy
template<typename T>
void copyArrayToCudaMemory(InTopLink& inTopLink, T array, CudaFlags flags)
{
	if (array.is_valid())
	{
		CUDAMemory cudaMemory;
		cudaMemory.ptr = array.data();
		cudaMemory.size = array.size();

		auto shape = array.shape_ptr();
		auto strides = array.stride_ptr();

		CUDAMemoryDesc desc;
		for (int i = 0; i < array.ndim(); i++)
		{
			desc.shape[i] = shape[i];
			desc.strides[i] = strides[i];
		}

		desc.componentSize = array.itemsize();
		desc.dataType = cudaDataTypeFromDtype(array.dtype());
		desc.flags = flags;
		cudaMemory.desc = desc;

		inTopLink.copyCudaMemory(cudaMemory, nullptr);
	}
}


using arrayShapeCHW4 = nb::shape<4, nb::any, nb::any>;
using arrayShapeCHW3 = nb::shape<3, nb::any, nb::any>;
using arrayShapeCHW2 = nb::shape<2, nb::any, nb::any>;
using arrayShapeCHW1 = nb::shape<1, nb::any, nb::any>;

using arrayShapeHWC4 = nb::shape<nb::any, nb::any, 4>;
using arrayShapeHWC3 = nb::shape<nb::any, nb::any, 3>;
using arrayShapeHWC2 = nb::shape<nb::any, nb::any, 2>;
using arrayShapeHWC1 = nb::shape<nb::any, nb::any, 1>;

void initTopLinkBindings(nb::module_& m)
{
	nb::enum_<CudaFlagBits>(m, "CudaFlags")
		.value("NONE", CudaFlagBits::None)
		.value("RGBA", CudaFlagBits::RGBA)
		.value("RGB", CudaFlagBits::RGB)
		.value("RG", CudaFlagBits::RG)
		.value("R", CudaFlagBits::R)
		.value("BGRA", CudaFlagBits::BGRA)
		.value("BGR", CudaFlagBits::BGR)
		.value("CHW", CudaFlagBits::CHW)
		.value("HWC", CudaFlagBits::HWC)
		.def(nb::self | nb::self)
		.def(nb::self & nb::self)
		.def(nb::self ^ nb::self)
		.def(~nb::self)
		.def(nb::self |= nb::self)
		.def(nb::self &= nb::self)
		.def(nb::self ^= nb::self)
		.def(nb::self == nb::self)
		.def(nb::self != nb::self)
		;


	nb::enum_<CUDADataType>(m, "CUDADataType")
		.value("UInt8", CUDADataType::UInt8)
		.value("Float16", CUDADataType::Float16)
		.value("Float32", CUDADataType::Float32)
		.value("Undefined", CUDADataType::Undefined).doc() = "The data type of a CUDA memory buffer";
		;

	nb::class_<CUDAMemoryDesc> cudaMemoryDesc(m, "CudaMemoryDesc");
	cudaMemoryDesc.def(nb::init<>())
		.def_rw("shape", &CUDAMemoryDesc::shape)
		.def_rw("component_size", &CUDAMemoryDesc::componentSize)
		.def_rw("data_type", &CUDAMemoryDesc::dataType)
		.def_rw("strides", &CUDAMemoryDesc::strides)
		//.def("__repr__", [](CUDAMemoryShape& self)
		//	{
		//		return "CudaMemoryShape(" + std::to_string(self.width) + ", " + std::to_string(self.height)
		//			+ ", " + std::to_string(self.numComponents) + ", " + std::to_string(self.componentSize)
		//			+ ", " + cudaDataTypeToString(self.dataType) + ")";
		//	})
	;

	nb::class_<CUDAMemory> cudaMemory(m, "CudaMemory");
	cudaMemory.doc() = "A pointer to CUDA memory and its attributes";
	cudaMemory.def(nb::init<>())
		.def_ro("size", &CUDAMemory::size, nb::rv_policy::reference_internal)
		.def_rw("desc", &CUDAMemory::desc, nb::rv_policy::reference_internal)
		.def_prop_ro("ptr", [](CUDAMemory& self) -> uintptr_t
			{ return reinterpret_cast<uintptr_t>(self.ptr); }, nb::rv_policy::reference_internal);


	nb::class_<OutTopLink> outTopLink(m, "OutTopLink");
	outTopLink.doc() = "An OutTOP in a TouchDesigner component";
	outTopLink.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("cuda_memory", &OutTopLink::cudaMemory)
		.def("set_cuda_flags", [](OutTopLink& self, CudaFlagBits flags) { self.setCudaFlags(flags); })
		.def("as_dlpack", &arrayFromCudaMem<>, nb::rv_policy::reference_internal)
		.def("as_tensor", &arrayFromCudaMem<nb::pytorch>, nb::rv_policy::reference_internal)
		;

	nb::class_<OutTopLinks> outTopLinks(m, "OutTopLinks");
	outTopLinks.doc() = "A collection of OutTOP links in a TouchDesigner component";
	outTopLinks.def(nb::init<>())
		.def_prop_ro("count", [](OutTopLinks& self) { return self.size(); })
		.def_prop_ro("names", [](OutTopLinks& self) { return self.getLinkNames(); }, nb::rv_policy::reference_internal)
		.def("__getitem__", [](OutTopLinks& self, const std::string& name) { return self.getLinkByName(name); }, nb::rv_policy::reference_internal)
		.def("__getitem__", [](OutTopLinks& self, size_t index) { return self.getLinkByIndex(index); }, nb::rv_policy::reference_internal)
		;

	nb::class_<InTopLink> inTopLink(m, "InTopLink");
	inTopLink.doc() = "An InTOP in a TouchDesigner component";
	inTopLink.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>());

	//inTopLink
	//	.def("from_dlpack",
	//		[](InTopLink& self, nb::ndarray<arrayShape4, nb::device::cuda> array)
	//		{ copyArrayToCudaMemory<nb::ndarray<arrayShape4, nb::device::cuda>>(self, array); })
	//	.def("from_dlpack",
	//		[](InTopLink& self, nb::ndarray<arrayShape3, nb::device::cuda> array)
	//		{ copyArrayToCudaMemory<nb::ndarray<arrayShape3, nb::device::cuda>>(self, array); })
	//	.def("from_dlpack",
	//		[](InTopLink& self, nb::ndarray<arrayShape2, nb::device::cuda> array)
	//		{ copyArrayToCudaMemory<nb::ndarray<arrayShape2, nb::device::cuda>>(self, array); })
	//	.def("from_dlpack",
	//		[](InTopLink& self, nb::ndarray<arrayShape1, nb::device::cuda> array)
	//		{ copyArrayToCudaMemory<nb::ndarray<arrayShape1, nb::device::cuda>>(self, array); })
	//	;


	// TODO: set CHW and HWC flags based on array shape... 
	inTopLink
		.def("from_tensor",
			[](InTopLink& self, nb::ndarray<nb::pytorch, arrayShapeCHW4, nb::device::cuda> array, CudaFlagBits flags)
			{	flags |= CudaFlagBits::CHW; flags &= ~CudaFlagBits::HWC;
				copyArrayToCudaMemory<nb::ndarray<nb::pytorch, arrayShapeCHW4, nb::device::cuda>>(self, array, flags); },
			"array"_a, "flags"_a = CudaFlagBits::None)
		.def("from_tensor",
			[](InTopLink& self, nb::ndarray<nb::pytorch, arrayShapeCHW3, nb::device::cuda> array, CudaFlagBits flags)
			{	flags |= CudaFlagBits::CHW; flags &= ~CudaFlagBits::HWC;
				copyArrayToCudaMemory<nb::ndarray<nb::pytorch, arrayShapeCHW3, nb::device::cuda>>(self, array, flags); },
			"array"_a, "flags"_a = CudaFlagBits::None)
		.def("from_tensor",
			[](InTopLink& self, nb::ndarray<nb::pytorch, arrayShapeCHW2, nb::device::cuda> array, CudaFlagBits flags)
			{	flags |= CudaFlagBits::CHW; flags &= ~CudaFlagBits::HWC;
				copyArrayToCudaMemory<nb::ndarray<nb::pytorch, arrayShapeCHW2, nb::device::cuda>>(self, array, flags); },
			"array"_a, "flags"_a = CudaFlagBits::None)
		.def("from_tensor",
			[](InTopLink& self, nb::ndarray<nb::pytorch, arrayShapeCHW1, nb::device::cuda> array, CudaFlagBits flags)
			{	flags |= CudaFlagBits::CHW; flags &= ~CudaFlagBits::HWC;
				copyArrayToCudaMemory<nb::ndarray<nb::pytorch, arrayShapeCHW1, nb::device::cuda>>(self, array, flags); },
			"array"_a, "flags"_a = CudaFlagBits::None)
		.def("from_tensor",
			[](InTopLink& self, nb::ndarray < nb::pytorch, arrayShapeHWC4, nb::device::cuda> array, CudaFlagBits flags)
			{	flags |= CudaFlagBits::HWC; flags &= ~CudaFlagBits::CHW;
				copyArrayToCudaMemory<nb::ndarray<nb::pytorch, arrayShapeHWC4, nb::device::cuda>>(self, array, flags); },
			"array"_a, "flags"_a = CudaFlagBits::None)
		.def("from_tensor",
			[](InTopLink& self, nb::ndarray<nb::pytorch, arrayShapeHWC3, nb::device::cuda> array, CudaFlagBits flags)
			{	flags |= CudaFlagBits::HWC; flags &= ~CudaFlagBits::CHW;
				copyArrayToCudaMemory<nb::ndarray<nb::pytorch, arrayShapeHWC3, nb::device::cuda>>(self, array, flags); },
			"array"_a, "flags"_a = CudaFlagBits::None)
		.def("from_tensor",
			[](InTopLink& self, nb::ndarray<nb::pytorch, arrayShapeHWC2, nb::device::cuda> array, CudaFlagBits flags)
			{	flags |= CudaFlagBits::HWC; flags &= ~CudaFlagBits::CHW;
				copyArrayToCudaMemory<nb::ndarray<nb::pytorch, arrayShapeHWC2, nb::device::cuda>>(self, array, flags); },
			"array"_a, "flags"_a = CudaFlagBits::None)
		.def("from_tensor",
			[](InTopLink& self, nb::ndarray<nb::pytorch, arrayShapeHWC1, nb::device::cuda> array, CudaFlagBits flags)
			{	flags |= CudaFlagBits::HWC; flags &= ~CudaFlagBits::CHW;
				copyArrayToCudaMemory<nb::ndarray<nb::pytorch, arrayShapeHWC1, nb::device::cuda>>(self, array, flags); },
			"array"_a, "flags"_a = CudaFlagBits::None)

		;

	inTopLink.def("copy_cuda_memory", [](InTopLink& self, const CUDAMemory& memory)
		{
			self.copyCudaMemory(memory, nullptr);
		}, "cuda_mem"_a);
	 
	//inTopLink.def("copy_cuda_memory", [](InTopLink& self, const CUDAMemory& memory, CudaFlagBits flags)
	//	{
	//		self.copyCudaMemory(memory, nullptr);
	//	}, "cuda_mem"_a, "flags"_a = CudaFlagBits::None);

	nb::class_<InTopLinks> inTopLinks(m, "InTopLinks");
	inTopLinks.doc() = "A collection of InTOP links in a TouchDesigner component";
	inTopLinks.def(nb::init<>())
		.def_prop_ro("count", [](InTopLinks& self) { return self.size(); })
		.def_prop_ro("names", [](InTopLinks& self) { return self.getLinkNames(); }, nb::rv_policy::reference_internal)
		.def("__getitem__", [](InTopLinks& self, const std::string& name) { return self.getLinkByName(name); }, nb::rv_policy::reference_internal)
		.def("__getitem__", [](InTopLinks& self, size_t index) { return self.getLinkByIndex(index); }, nb::rv_policy::reference_internal)
		;


}