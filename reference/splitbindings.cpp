


// File comppy.cpp
#include "commonpy.h"

void initCompBindings(nb::module_& m)
{
	nb::class_<Comp> comp(m, "Comp");
	comp.doc() = "Represents a TouchDesigner component";
	comp.def(nb::init<>());
	comp.def(nb::init<const std::string&>());
	comp.def("loadTox", &Comp::loadTox);
	comp.def("unload", &Comp::unload);
	comp.def("loaded", &Comp::loaded);
	comp.def("ready", &Comp::ready);
	comp.def("update", &Comp::update);
	comp.def("setOnFrameStartCallback", &Comp::setOnFrameStartCallback);
	comp.def("runUpdateLoop", &Comp::runUpdateLoop);
	comp.def("stopUpdateLoop", &Comp::stopUpdateLoop);
	comp.def("inputTopLinks", &Comp::inputTopLinks);
	comp.def("outputTopLinks", &Comp::outputTopLinks);
	comp.def("inChopLinks", &Comp::inChopLinks);
	comp.def("outChopLinks", &Comp::outChopLinks);
	comp.def("inDatLinks", &Comp::inDatLinks);
	comp.def("outDatLinks", &Comp::outDatLinks);
	comp.def("parLinks", &Comp::parLinks);

	//...
}


// File toplinkpy.cpp
#include "commonpy.h"

void initTopLinkBindings(nb::module_& m) 
{
	nb::class_<OutTopLink> outTopLink(m, "OutTopLink");
	outTopLink.doc() = "Represents an OutTOP in a TouchDesigner component";
	outTopLink.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>());
	// ...

	nb::class_<InTopLink> inTopLink(m, "InTopLink");
	// ...

	nb::class_<OutTopLinks> outTopLinks(m, "OutTopLinks");
	// ...
}


// File touchpy.cpp
#include "commonpy.h"

extern void initCompBindings(nb::module_& m);
extern void initTopLinkBindings(nb::module_& m);
// ...

PYBIND11_MODULE(touch, m)
{
	initCompBindings(m);
	initTopLinkBindings(m);
	// ...
}



