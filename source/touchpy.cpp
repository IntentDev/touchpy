#include <nanobind/nanobind.h>
#include "logging.h"

namespace nb = nanobind;
using namespace nb::literals;


void printInfo(const std::string& info)
{
    nb::gil_scoped_acquire acquire;
    nb::print(info.c_str());
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
		;

	//nb::set_leak_warnings(false);
	initLogging(spdlog::level::info);

	m.def("set_log_level", &setLogLevel, "level"_a = spdlog::level::warn);

	initCompBindings(m);
	initTopLinkBindings(m);
	initChopLinkBindings(m);
	initDatLinkBindings(m);
	initParLinkBindings(m);
}