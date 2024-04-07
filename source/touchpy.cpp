#include <nanobind/nanobind.h>
#include "logging.h"

namespace nb = nanobind;


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
	//nb::set_leak_warnings(false);
    init_logging();

	initCompBindings(m);
	initTopLinkBindings(m);
	initChopLinkBindings(m);
	initDatLinkBindings(m);
	initParLinkBindings(m);
}