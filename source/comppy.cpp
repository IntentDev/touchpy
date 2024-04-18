#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/operators.h>

#include "comp.h"

namespace nb = nanobind;
using namespace nb::literals;

static const char* load_toxDoc = 
R"(Loads a .tox file, creates and initializes a TouchEngine Instance.
Args:
	path (str) : the path to the .tox file

Returns:
	True if the .tox file was loaded successfully, False otherwise
)";

static const char* unloadDoc =
R"(Unloads TouchEngine instance.

Returns:
	True if the toxfile has unloaded, False if the toxfile is still loaded 
)";

static const char* startDoc =
R"(Starts the TouchEngine instance.

Returns:
	None
)";

static const char* stopDoc =
R"(Stops the TouchEngine instance.

Returns:
	None
)";

static const char* frame_did_finishDoc =
R"(Stops the TouchEngine instance.

Returns:
	None
)";

static const char* apply_value_changesDoc =
R"(Stops the TouchEngine instance.

Returns:
	None
)";

static const char* call_on_frame_callbackDoc =
R"(calls the method set using: set_on_frame_callback()

Returns:
	None
)";

static const char* start_next_frameDoc =
R"(Starts the next frame.

Returns:
	True if frame started succesfully, False otherwise
)";

static const char* loadedDoc =
R"(True if the .tox file was loaded successfully, False otherwise.
)";

static const char* frame_did_finishDoc =
R"(True if the frame has finished, False otherwise.
)";

static const char* in_topsDoc =
R"(The input TOPs.
)";

static const char* in_topsDoc =
R"(The In TOPs of the currently loaded tox.
)";

static const char* out_topsDoc =
R"(The Out TOPs of the currently loaded tox.
)";

static const char* in_chopsDoc =
R"(The In CHOPs of the currently loaded tox.
)";

static const char* out_chopsDoc =
R"(The Out CHOPs of the currently loaded tox.
)";
static const char* in_datsDoc =
R"(The In DATs of the currently loaded tox.
)";

static const char* out_datsDoc =
R"(The Out DATs of the currently loaded tox.
)";

static const char* out_datsDoc =
R"(The parameters of the currently loaded tox.
)";

static const char* clear_on_frame_callbackDoc =
R"(Unsets any callback method set using set_on_frame_callback().

Returns:
	None
)";

static const char* set_on_frame_callbackDoc =
R"(Sets the Python method to be called everytime a frame ends.

Args:
	callback (callable)	: a callable Python method
	user_data (obj)		: a Python object for any userdata to be passed to the callback method

Returns:
	None
)";

static const char* clear_on_layout_change_callbackDoc =
R"(Unsets any callback method set using: on_layout_change_callback().

Returns:
	None
)";

static const char* set_on_layout_change_callbackDoc =
R"(Sets the Python method to be called everytime the tox layout changes.

Args:
	callback (callable)	: a callable Python method
	user_data (obj)		: a Python object for any userdata to be passed to the callback method

Returns:
	None
)";

static const char* cuda_streamDoc =
R"(Returns pointer to the CUDA stream handle used by TouchEngine.
)";





void initCompBindings(nb::module_& m)
{
	//Comp::setPrintInfoFunc(printInfo);

	nb::enum_<CompFlagBits>(m, "CompFlags")
		.value("INTERNAL_TIME", CompFlagBits::InternalTime)
		.value("EXTERNAL_TIME", CompFlagBits::ExternalTime)
		.value("AUTO_UPDATE", CompFlagBits::AutoUpdate)
		.value("ASYNC_UPDATE", CompFlagBits::AsyncUpdate)
		.value("REALTIME", CompFlagBits::Realtime)
		.value("INTERNAL_TIME_AUTO", CompFlagBits::InternalTimeAuto)
		.value("INTERNAL_TIME_ASYNC", CompFlagBits::InternalTimeAsync)
		.value("CUDA_STREAM_DEFAULT", CompFlagBits::CudaStreamDefault)
		.value("CUDA_STREAM_INTERNAL", CompFlagBits::CudaStreamInternal)
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

	nb::class_<Comp> comp(m, "Comp");
	comp.doc() = "A TouchDesigner component loaded in a TouchEngine instance.";
	comp.def(nb::init<>())
		.def(nb::init<const std::string&, CompFlagBits, int64_t>(),
			"tox_path"_a, "flags"_a = CompFlagBits::InternalTimeAuto, "fps"_a = 60, nb::rv_policy::take_ownership)
		.def("load_tox", [](Comp& self, std::string path, int fps) { self.loadTox(path, fps); } , "path"_a, "fps"_a = 60, load_toxDoc)
		.def("unload",                 &Comp::unload, nb::rv_policy::reference_internal)
		.def("start",                  &Comp::start, nb::rv_policy::reference_internal)
		.def("stop",                   &Comp::stop, nb::rv_policy::reference_internal)
		.def("apply_value_changes",    &Comp::applyValueChanges, nb::rv_policy::reference_internal)
		.def("call_on_frame_callback", &Comp::callOnFrameCallback, nb::rv_policy::reference_internal)
		.def("start_next_frame",       &Comp::startNextFrame, "time_value"_a = 0, "time_scale"_a = 0, nb::rv_policy::reference_internal)
		.def_ro("loaded",              &Comp::loaded, nb::rv_policy::reference_internal)
		.def_ro("frame_did_finish",    &Comp::frameDidFinish, nb::rv_policy::reference_internal)
		.def_prop_ro("in_tops",        &Comp::inputTopLinks, nb::rv_policy::reference_internal)
		.def_prop_ro("out_tops",       &Comp::outputTopLinks, nb::rv_policy::reference_internal)
		.def_prop_ro("in_chops",       &Comp::inChopLinks, nb::rv_policy::reference_internal)
		.def_prop_ro("out_chops",      &Comp::outChopLinks, nb::rv_policy::reference_internal)
		.def_prop_ro("in_dats",        &Comp::inDatLinks, nb::rv_policy::reference_internal)
		.def_prop_ro("out_dats",       &Comp::outDatLinks, nb::rv_policy::reference_internal)
		.def_prop_ro("par",            &Comp::parLinks, nb::rv_policy::reference_internal)
		;

	comp.def("clear_on_frame_callback", &Comp::clearOnFrameCallback, nb::rv_policy::reference_internal);
	comp.def("set_on_frame_callback", [](Comp& self, nb::callable pythonCallback, nb::object userData)
		{
			auto userDataPtr = std::make_shared<nb::object>(userData);
			self.setOnFrameCallback([pythonCallback](Comp& comp, std::shared_ptr<void> userData)
				{
					auto& userDataPyObj = *std::static_pointer_cast<nb::object>(userData);
					if (!comp.asyncRunning())
					{
						pythonCallback(nb::cast(comp, nb::rv_policy::reference_internal), userDataPyObj);
					}

					else
					{
						nb::gil_scoped_acquire acquire;
						pythonCallback(nb::cast(comp, nb::rv_policy::reference_internal), userDataPyObj);
					}
				},
				userDataPtr);
		}
	);

	comp.def("clear_on_layout_change_callback", &Comp::clearOnLayoutChangeCallback, nb::rv_policy::reference_internal);
	comp.def("set_on_layout_change_callback", [](Comp& self, nb::callable pythonCallback, nb::object userData)
		{
			auto userDataPtr = std::make_shared<nb::object>(userData);
			self.setOnLayoutChangeCallback([pythonCallback](Comp& comp, std::shared_ptr<void> userData)
				{
					auto& userDataPyObj = *std::static_pointer_cast<nb::object>(userData);
					if (!comp.asyncRunning())
					{
						pythonCallback(nb::cast(comp, nb::rv_policy::reference_internal), userDataPyObj);
					}

					else
					{
						nb::gil_scoped_acquire acquire;
						pythonCallback(nb::cast(comp, nb::rv_policy::reference_internal), userDataPyObj);
					}
				},
				userDataPtr);
		}
	);

	comp.def("cuda_stream", [](Comp& self) -> uintptr_t
		{ 
			return reinterpret_cast<uintptr_t>(self.cudaStream());
		}
	, nb::rv_policy::reference_internal);

	

}