
#include "comp.h"

#include <texturelink.h>
#include <choplink.h>
#include <datlink.h>
#include <parlink.h>


#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <memory>


namespace py = pybind11;



PYBIND11_MODULE(touchpy, m) 
{
    m.doc() = "Comp: Loads a TouchDesigner .tox file and runs in process."; 

    py::class_<Comp> comp(m, "Comp");
    comp.def(py::init<>())
		.def(py::init<std::string>())
		.def("load_tox", &Comp::loadTox)
		.def("loaded", &Comp::loaded)
		.def("update", &Comp::update)
        .def("start", &Comp::runUpdateLoop)
        .def("stop", &Comp::stopUpdateLoop)
        .def_property_readonly("input_textures", &Comp::inputTextureLinks, py::return_value_policy::reference)
        .def_property_readonly("output_textures", &Comp::outputTextureLinks, py::return_value_policy::reference)
        .def_property_readonly("input_chops", &Comp::inputChopLinks, py::return_value_policy::reference)
        .def_property_readonly("output_chops", &Comp::outputChopLinks, py::return_value_policy::reference)
        .def_property_readonly("input_dats", &Comp::inputDatLinks, py::return_value_policy::reference)
        .def_property_readonly("output_dats", &Comp::outputDatLinks, py::return_value_policy::reference)
  	    //.def("par_links", &Comp::parLinks, py::return_value_policy::reference)
        ;

    comp.def("set_on_frame_start_callback", [](
        Comp& self,
        py::function callback,
        py::object userData)
        {
            auto userDataPtr = std::make_shared<py::object>(userData);
            self.setOnFrameStartCallback(
                [callback](Comp& comp, std::shared_ptr<void> userData)
                {
                    auto userDataPyObj = *std::static_pointer_cast<py::object>(userData);
                    callback(comp, userDataPyObj);
                },
                userDataPtr);
        });

    py::class_<DatLink> datLink(m, "DatLink");
    datLink.def(py::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("on_output_value_change", &DatLink::onOuputValueChange)
		.def("get_table", &DatLink::getTable)
		.def("get_string", &DatLink::getString)
        .def("set_table", py::overload_cast<const DatLink::Table&>(&DatLink::set))
		.def("set_string",py::overload_cast<const std::string&>(&DatLink::set))
		.def("get_type", &DatLink::type)
		.def("get_type_description", &DatLink::getTypeDescription)
		;

    py::class_<DatLinks> datLinks(m, "DatLinks");
	datLinks.def(py::init<>())
        .def("num_links", &DatLinks::size)
        .def("link_names", &DatLinks::getLinkNames)
		.def("__getitem__", [](DatLinks& self, const std::string& name) { return self.getLinkByName(name); }, py::return_value_policy::reference)
        .def("__getitem__", [](DatLinks& self, size_t index) { return self.getLinkByIndex(index); }, py::return_value_policy::reference)
		;



}

