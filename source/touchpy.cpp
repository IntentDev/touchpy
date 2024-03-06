#include <nanobind/nanobind.h>
#include <nanobind/stl/unique_ptr.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/function.h>

#include "comp.h"
#include "texturelink.h"
#include "choplink.h"
#include "datlink.h"
#include "parlink.h"

#include <memory>

namespace nb = nanobind;
using namespace nb::literals;

int add(int a, int b) { return a + b; }

NB_MODULE(touchpy, m) {
    m.def("add", &add);

    nb::class_<Comp> comp(m, "Comp");
    comp.def(nb::init<>())
        .def(nb::init<const std::string&>(), nb::rv_policy::reference_internal)
        .def("load_tox", &Comp::loadTox, nb::rv_policy::reference_internal)
        .def("loaded", &Comp::loaded, nb::rv_policy::reference_internal)
        .def("update", &Comp::update, nb::rv_policy::reference_internal)
        .def("start", &Comp::runUpdateLoop, nb::rv_policy::reference_internal)
        .def("stop", &Comp::stopUpdateLoop, nb::rv_policy::reference_internal)
        .def_prop_ro("input_textures", &Comp::inputTextureLinks, nb::rv_policy::reference_internal)
        .def_prop_ro("output_textures", &Comp::outputTextureLinks, nb::rv_policy::reference_internal)
        .def_prop_ro("input_chops", &Comp::inputChopLinks, nb::rv_policy::reference_internal)
        .def_prop_ro("output_chops", &Comp::outputChopLinks, nb::rv_policy::reference_internal)
        .def_prop_ro("input_dats", &Comp::inputDatLinks, nb::rv_policy::reference_internal)
        .def_prop_ro("output_dats", &Comp::outputDatLinks, nb::rv_policy::reference_internal)
        //.def("par_links", &Comp::parLinks, nb::rv_policy::reference)
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
        });

    nb::class_<DatLink> datLink(m, "DatLink");
    datLink.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
        .def("on_output_value_change", &DatLink::onOuputValueChange)
        .def("get_table", &DatLink::getTable)
        .def("get_string", &DatLink::getString)
        .def("set_table", nb::overload_cast<const DatLink::Table&>(&DatLink::set))
        .def("set_string", nb::overload_cast<const std::string&>(&DatLink::set))
        .def("get_type", &DatLink::type)
        .def("get_type_description", &DatLink::getTypeDescription)
        ;

    nb::class_<DatLinks> datLinks(m, "DatLinks");
    datLinks.def(nb::init<>())
        .def("num_links", &DatLinks::size)
        .def("link_names", &DatLinks::getLinkNames)
        .def("__getitem__", [](DatLinks& self, const std::string& name) { return self.getLinkByName(name); }, nb::rv_policy::reference)
        .def("__getitem__", [](DatLinks& self, size_t index) { return self.getLinkByIndex(index); }, nb::rv_policy::reference)
        ;


}