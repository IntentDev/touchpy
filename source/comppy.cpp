#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>

#include "comp.h"

namespace nb = nanobind;
using namespace nb::literals;

const char* load_toxDoc = 
R"(Loads a .tox file, creates and initializes a TouchEngine Instance
Args:
	path: the path to the .tox file

Returns:
	True if the .tox file was loaded successfully, False otherwise
)";



void initCompBindings(nb::module_& m)
{
	nb::class_<Comp> comp(m, "Comp");
	comp.doc() = "A TouchDesigner component loaded in a TouchEngine instance.";
	comp.def(nb::init<>())
		.def(nb::init<const std::string&, bool>(), "tox_path"_a, "free_running"_a = false, nb::rv_policy::reference_internal)

		.def("load_tox", &Comp::loadTox, "path"_a, nb::rv_policy::reference_internal, load_toxDoc)
		.def("loaded", &Comp::loaded, nb::rv_policy::reference_internal)
		.def("update", &Comp::update, "start_next_frame"_a = false, nb::rv_policy::reference_internal)
		.def("start_next_frame", &Comp::startNextFrame, nb::rv_policy::reference_internal)
		.def("start", &Comp::runUpdateLoop, "update_starts_next_frame"_a = false, nb::rv_policy::reference_internal)
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

}