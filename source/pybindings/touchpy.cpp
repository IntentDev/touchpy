#include "touchpy.h"
#include "logging.h"

#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/string.h>

namespace nb = nanobind;
using namespace nb::literals;

void printInfo(const std::string& info)
{
    nb::gil_scoped_acquire acquire;
    nb::print(info.c_str());
}

std::string dtype_codeAsStr(uint8_t code)
{
	auto code_enum = static_cast<nb::dlpack::dtype_code>(code);
	switch (code_enum)
	{
	case nb::dlpack::dtype_code::Int: return "Int";
	case nb::dlpack::dtype_code::UInt: return "UInt";
	case nb::dlpack::dtype_code::Float: return "Float";
	case nb::dlpack::dtype_code::Bfloat: return "Bfloat";
	case nb::dlpack::dtype_code::Complex: return "Complex";
	case nb::dlpack::dtype_code::Bool: return "Bool";
	default : return "Unknown";
	}
}

nb::dict getDLPackCapsuleInfo(nb::ndarray<> array)
{
	nb::gil_scoped_acquire acquire;
	nb::dict info;

	// see nanobind/ndarray.h for names of device types - could break if nanobind changes...
	const char* device_names[] = {
	"none", "cpu", "cuda", "cuda_host", "opencl", "vulkan", "metal", "rocm", "rocm_host", "cuda_managed", "oneapi" };

	info["device_type"] = device_names[array.device_type()];
	info["device_id"] = array.device_id();

	auto dtype = array.dtype();
	nb::dict info_dtype;
	info_dtype["code"] = dtype.code;
	info_dtype["code_str"] = dtype_codeAsStr(dtype.code);
	info_dtype["bits"] = dtype.bits;
	info_dtype["lanes"] = dtype.lanes;
	info["dtype"] = info_dtype;


	info["ndim"] = array.ndim();
	info["nbytes"] = array.nbytes();
	info["size"] = array.size();
	info["itemsize"] = array.itemsize();

	std::vector<int64_t> shape(array.ndim());
	std::copy(array.shape_ptr(), array.shape_ptr() + array.ndim(), shape.begin());
	info["shape"] = shape;

	std::vector<int64_t> stride(array.ndim());
	std::copy(array.stride_ptr(), array.stride_ptr() + array.ndim(), stride.begin());
	info["stride"] = stride;

	return info;
}

extern void initCompBindings(nb::module_& m);
extern void initTopLinkBindings(nb::module_& m);
extern void initChopLinkBindings(nb::module_& m);
extern void initDatLinkBindings(nb::module_& m);
extern void initParLinkBindings(nb::module_& m);

NB_MODULE(touchpy, m)
{
	nb::enum_<spdlog::level::level_enum> (m, "LogLevel")
		.value("TRACE", spdlog::level::trace)
		.value("DEBUG", spdlog::level::debug)
		.value("INFO", spdlog::level::info)
		.value("WARN", spdlog::level::warn)
		.value("ERROR", spdlog::level::err)
		.value("CRITICAL", spdlog::level::critical)
		.value("OFF", spdlog::level::off)
		.doc() = LogLevelDoc
		;

	//nb::set_leak_warnings(false);
	initLogging(spdlog::level::debug, true, false);

	m.def("init_logging", &initLogging, "level"_a = spdlog::level::debug, "console"_a = true, "file"_a = false, init_loggingDoc);
	m.def("set_log_level", &setLogLevel, "level"_a = spdlog::level::warn, set_log_levelDoc);

	m.def("get_dlpack_capsule_info", &getDLPackCapsuleInfo, "array"_a, get_dlpack_capsule_infoDoc);


	initCompBindings(m);
	initTopLinkBindings(m);
	initChopLinkBindings(m);
	initDatLinkBindings(m);
	initParLinkBindings(m);
}
