#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/operators.h>

#include "comp.h"

namespace nb = nanobind;
using namespace nb::literals;

static const char* load_toxDoc = 
R"(Loads a .tox file, creates and initializes a TouchEngine Instance.

Args:
	tox_path (str) : the path to the .tox file
	fps (int) : the frames per second of the TouchEngine instance

Returns:
	bool: True if the .tox file was loaded successfully, False otherwise
)";

static const char* unloadDoc =
R"(Unloads TouchEngine instance.

Returns:
	True if the toxfile has unloaded, False if the toxfile is still loaded 
)";

static const char* startDoc =
R"(Starts the TouchEngine instance.
)";

static const char* stopDoc =
R"(Stops the TouchEngine instance.
)";

static const char* apply_value_changesDoc =
R"(Stops the TouchEngine instance.
)";

static const char* call_on_frame_callbackDoc =
R"(calls the method set using: set_on_frame_callback()

Returns:
	bool: True if the callback was succesfully called, False otherwise
)";

static const char* start_next_frameDoc =
R"(Starts the next frame.

Returns:
	bool: True if frame started succesfully, False otherwise
)";

static const char* loadedDoc =
R"(Returns:
	bool: True if the .tox file was loaded successfully, False otherwise.
)";

static const char* frame_did_finishDoc =
R"(Returns:
	bool: True if the frame has finished, False otherwise.
)";

static const char* time_doc =
R"(This method is best called at most once per frame, as it i an asynchronous call to the TouchEngine instance.

Usage:
	time_info = comp.time()

Returns:
	Time: a struct containing the time information of the currently loaded component.

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

static const char* parDoc =
R"(The parameters of the currently loaded tox.
)";

static const char* clear_on_frame_callbackDoc =
R"(Unsets any callback method set using :py:meth:`set_on_frame_callback`.

Returns:
	None
)";

static const char* set_on_frame_callbackDoc =
R"(Sets the Python method to be called everytime a frame ends.

Args:
	callback (Callable)	: a callable Python method
	user_data (object)	: a Python object for any userdata to be passed to the callback method

Returns:
	None
)";

static const char* clear_on_layout_change_callbackDoc =
R"(Unsets any callback method set using: :py:meth:`set_on_layout_change_callback`.
)";

static const char* set_on_layout_change_callbackDoc =
R"(Sets the Python method to be called everytime the tox layout changes.

Args:
	callback (Callable)	: a callable Python method
	user_data (object)	: a Python object for any userdata to be passed to the callback method

Returns:
	None
)";

static const char* cuda_streamDoc =
R"(Returns the CUDA stream handle used by TouchEngine.
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
		.value("CUDA_DISABLE", CompFlagBits::CudaDisable)
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

	nb::class_ <Comp::Time> time(m, "Time");
	time.doc() = "A struct containing the time information of the currently loaded component.";
	time.def("__repr__", [](const Comp::Time& self) -> std::string
		{
			return
				"Time{rate: "	+ std::to_string(self.rate) +
				", frame: "		+ std::to_string(self.frame) +
				", seconds: "	+ std::to_string(self.seconds) +
				", value: "		+ std::to_string(self.value) +
				", scale: "		+ std::to_string(self.scale) + "}";
		})
		.def_ro("rate", &Comp::Time::rate)
		.def_ro("frame", &Comp::Time::frame)
		.def_ro("seconds", &Comp::Time::seconds)
		.def_ro("value", &Comp::Time::value)
		.def_ro("scale", &Comp::Time::scale)
	;

	nb::class_<Comp> comp(m, "Comp");
	comp.doc() = "A TouchDesigner component loaded in a TouchEngine instance.";
	comp.def(nb::init<>(), nb::rv_policy::take_ownership)
		.def(nb::init<uint8_t>(), "device"_a = 0u, nb::rv_policy::take_ownership)
		.def(nb::init<const std::string&, CompFlagBits, int64_t, uint8_t>(),
			"tox_path"_a, "flags"_a = CompFlagBits::InternalTimeAuto, "fps"_a = 60, "device"_a = 0u, nb::rv_policy::take_ownership)
		.def("load_tox", [](Comp& self, std::string path, int fps) { self.loadTox(path, fps); } , "tox_path"_a, "fps"_a = 60, load_toxDoc)
		.def("unload",                 &Comp::unload, unloadDoc, nb::rv_policy::reference_internal)
		.def("start",                  &Comp::start, startDoc, nb::rv_policy::reference_internal)
		.def("stop",                   &Comp::stop, stopDoc, nb::rv_policy::reference_internal)
		.def("apply_value_changes",    &Comp::applyValueChanges, apply_value_changesDoc, nb::rv_policy::reference_internal)
		.def("call_on_frame_callback", &Comp::callOnFrameCallback, call_on_frame_callbackDoc, nb::rv_policy::reference_internal)
		.def("start_next_frame",       &Comp::startNextFrame, "time_value"_a = 0, "time_scale"_a = 0, start_next_frameDoc, nb::rv_policy::reference_internal)
		.def("loaded",				   &Comp::loaded, loadedDoc, nb::rv_policy::reference_internal)
		.def("frame_did_finish",       &Comp::frameDidFinish, frame_did_finishDoc, nb::rv_policy::reference_internal)
		.def("time",                   &Comp::time, time_doc, nb::rv_policy::reference_internal)
		.def_prop_ro("in_tops",        &Comp::inputTopLinks, in_topsDoc, nb::rv_policy::reference_internal)
		.def_prop_ro("out_tops",       &Comp::outputTopLinks,out_topsDoc, nb::rv_policy::reference_internal)
		.def_prop_ro("in_chops",       &Comp::inChopLinks, in_chopsDoc, nb::rv_policy::reference_internal)
		.def_prop_ro("out_chops",      &Comp::outChopLinks, out_chopsDoc, nb::rv_policy::reference_internal)
		.def_prop_ro("in_dats",        &Comp::inDatLinks, in_datsDoc, nb::rv_policy::reference_internal)
		.def_prop_ro("out_dats",       &Comp::outDatLinks, out_datsDoc, nb::rv_policy::reference_internal)
		.def_prop_ro("par",            &Comp::parLinks, parDoc, nb::rv_policy::reference_internal)
		;

	comp.def("clear_on_frame_callback", &Comp::clearOnFrameCallback, clear_on_frame_callbackDoc, nb::rv_policy::reference_internal);
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
		},
		set_on_frame_callbackDoc);

	comp.def("clear_on_layout_change_callback", &Comp::clearOnLayoutChangeCallback, clear_on_layout_change_callbackDoc, nb::rv_policy::reference_internal);
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
		},
		set_on_layout_change_callbackDoc);

	comp.def("cuda_stream", [](Comp& self) -> uintptr_t
		{ 
			return reinterpret_cast<uintptr_t>(self.cudaStream());
		},
		cuda_streamDoc, nb::rv_policy::reference_internal);

	comp.def_prop_ro("rate", [](Comp& self) -> float { return self.frameRate(); });

	

}