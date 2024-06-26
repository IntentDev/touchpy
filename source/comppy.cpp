#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/operators.h>

#include "comp.h"

namespace nb = nanobind;
using namespace nb::literals;

static const char* Comp_Doc = R"(
A TouchDesigner component loaded in a TouchEngine instance.

Args:
	flags (CompFlags) : the flags for the component
	device (int) : the hardware device to run the component on 
	td_path (str) : the path to the TouchDesigner installation

Args:
	tox_path (str) : the path to the .tox file
	flags (CompFlags) : the flags for the component
	device (int) : the hardware device to run the component on
	fps (int) : the frames per second of the TouchEngine instance
	td_path (str) : the path to the TouchDesigner installation

Attributes:
	in_tops (List[InTOP]) : the In TOPs of the currently loaded tox
	out_tops (List[OutTOP]) : the Out TOPs of the currently loaded tox
	in_chops (List[InCHOP]) : the In CHOPs of the currently loaded tox
	out_chops (List[OutCHOP]) : the Out CHOPs of the currently loaded tox
	in_dats (List[InDAT]) : the In DATs of the currently loaded tox
	out_dats (List[OutDAT]) : the Out DATs of the currently loaded tox
	par (List[Par]) : the parameters of the currently loaded tox
	rate (float) : the frame rate of the currently loaded tox
	cuda_stream (int) : the CUDA stream handle used by the CUDA device 
)";

static const char* load_Doc = 
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

static const char* file_pathDoc =
R"(The path to the loaded .tox file.
)";

static const char* td_pathDoc =
R"(The path to the TouchDesigner installation TouchEngine is using.
)";

static const char* cuda_deviceDoc =
R"(The CUDA device index used by TouchEngine.
)";

static const char* flagsDoc =
R"(The flags of the currently loaded tox.
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

static const char* set_on_loaded_callbackDoc =
R"(Sets the Python method to be called once the component is loaded.

Args:
	callback (Callable)	: a callable Python method
	user_data (object)	: a Python object for any userdata to be passed to the callback method

Returns:
	None
)";

static const char* set_on_start_callbackDoc =
R"(Sets the Python method to be called once the component starts.

Args:
	callback (Callable)	: a callable Python method
	user_data (object)	: a Python object for any userdata to be passed to the callback method

Returns:
	None
)";

static const char* set_on_stop_callbackDoc =
R"(Sets the Python method to be called once the component stops.

Args:
	callback (Callable)	: a callable Python method
	user_data (object)	: a Python object for any userdata to be passed to the callback method

Returns:
	None
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


inline void 
doCallback(Comp& self, nb::callable pythonCallback, CallbackData data)
{
	auto& dataPyObj = *std::static_pointer_cast<nb::object>(data);
	if (!self.asyncRunning())
	{
		pythonCallback(dataPyObj);
	}
	else
	{
		nb::gil_scoped_acquire acquire;
		pythonCallback(dataPyObj);
	}
};

inline void
doCallbackAsync(nb::callable pythonCallback, CallbackData data)
{
	auto& dataPyObj = *std::static_pointer_cast<nb::object>(data);
	nb::gil_scoped_acquire acquire;
	pythonCallback(dataPyObj);
};

void initCompBindings(nb::module_& m)
{
	//Comp::setPrintInfoFunc(printInfo);

	nb::enum_<CompFlagBits>(m, "CompFlags", nb::flag_enum())
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
	comp.doc() = Comp_Doc;
	comp.def(nb::init<>(), nb::rv_policy::take_ownership)

		.def(nb::init<CompFlagBits, uint8_t, const std::string&>(),
			"flags"_a = static_cast<CompFlags::IntType>(DEFAULT_COMP_FLAG_BITS),
			"device"_a = 0u, 
			"td_path"_a = "",
			nb::rv_policy::take_ownership)

		.def(nb::init<const std::string&, CompFlagBits, int64_t, uint8_t, const std::string&>(),
			"tox_path"_a, 
			"flags"_a = static_cast<CompFlags::IntType>(DEFAULT_COMP_FLAG_BITS),
			"fps"_a = 60, 
			"device"_a = 0u,
			"td_path"_a = "",
			nb::rv_policy::take_ownership)

		.def("load", [](Comp& self, std::string path, int fps) { self.load(path, fps); } , "tox_path"_a, "fps"_a = 60, load_Doc)
		.def("unload",                 &Comp::unload, unloadDoc, nb::rv_policy::reference_internal)
		.def("start",                  &Comp::start, startDoc, nb::rv_policy::reference_internal)
		.def("stop",                   &Comp::stop, stopDoc, nb::rv_policy::reference_internal)
		.def("apply_value_changes",    &Comp::applyValueChanges, apply_value_changesDoc, nb::rv_policy::reference_internal)
		.def("start_next_frame",       &Comp::startNextFrame, "time_value"_a = 0, "time_scale"_a = 0, start_next_frameDoc, nb::rv_policy::reference_internal)
		.def("loaded",				   &Comp::loaded, loadedDoc, nb::rv_policy::reference_internal)
		.def("frame_did_finish",       &Comp::frameDidFinish, frame_did_finishDoc, nb::rv_policy::reference_internal)
		.def("time",                   &Comp::time, time_doc, nb::rv_policy::reference_internal)
		.def_prop_ro("file_path",      &Comp::filePath, file_pathDoc, nb::rv_policy::reference_internal)
		.def_prop_ro("td_path",        &Comp::configuredEnginePath, td_pathDoc, nb::rv_policy::reference_internal)
		.def_prop_ro("cuda_device",    &Comp::cudaDeviceIndex, cuda_deviceDoc, nb::rv_policy::reference_internal)
		.def_prop_ro("flags",		   [](Comp& self) { return self.flags()(); }, flagsDoc, nb::rv_policy::reference_internal) // return the flags as an int
		.def_prop_ro("in_tops",        &Comp::inputTopLinks, in_topsDoc, nb::rv_policy::reference_internal)
		.def_prop_ro("out_tops",       &Comp::outputTopLinks,out_topsDoc, nb::rv_policy::reference_internal)
		.def_prop_ro("in_chops",       &Comp::inChopLinks, in_chopsDoc, nb::rv_policy::reference_internal)
		.def_prop_ro("out_chops",      &Comp::outChopLinks, out_chopsDoc, nb::rv_policy::reference_internal)
		.def_prop_ro("in_dats",        &Comp::inDatLinks, in_datsDoc, nb::rv_policy::reference_internal)
		.def_prop_ro("out_dats",       &Comp::outDatLinks, out_datsDoc, nb::rv_policy::reference_internal)
		.def_prop_ro("par",            &Comp::parLinks, parDoc, nb::rv_policy::reference_internal)
		.def_prop_ro("rate", [](Comp& self) -> float { return self.frameRate(); })
		.def("cuda_stream", [](Comp& self) -> uintptr_t { return reinterpret_cast<uintptr_t>(self.cudaStream()); }, cuda_streamDoc, nb::rv_policy::reference_internal)
		.def("clear_on_frame_callback", &Comp::clearOnFrameCallback, clear_on_frame_callbackDoc, nb::rv_policy::reference_internal)
		.def("clear_on_layout_change_callback", &Comp::clearOnLayoutChangeCallback, clear_on_layout_change_callbackDoc, nb::rv_policy::reference_internal)
		;

	comp.def("set_on_loaded_callback", [](Comp& self, nb::callable callback, nb::object data)
		{
			auto dataPtr = std::make_shared<nb::object>(data); 
			self.setOnLoadedCallback([callback](CallbackData data) { doCallbackAsync(callback, data); }, dataPtr);
		},
		"callback"_a, "info"_a, set_on_loaded_callbackDoc);

	comp.def("set_on_unloaded_callback", [](Comp& self, nb::callable callback, nb::object data)
		{
			auto dataPtr = std::make_shared<nb::object>(data);
			self.setOnUnloadedCallback([callback](CallbackData data) { doCallbackAsync(callback, data); }, dataPtr);
		},
		"callback"_a, "info"_a, set_on_loaded_callbackDoc);

	comp.def("set_on_start_callback", [](Comp& self, nb::callable callback, nb::object data)
		{
			auto dataPtr = std::make_shared<nb::object>(data);
			self.setOnStartCallback([&self, callback](CallbackData data) { doCallback(self, callback, data); }, dataPtr);
		},
		"callback"_a, "info"_a, set_on_start_callbackDoc);

	comp.def("set_on_stop_callback", [](Comp& self, nb::callable callback, nb::object data)
		{
			auto dataPtr = std::make_shared<nb::object>(data);
			self.setOnStopCallback([&self, callback](CallbackData data) { doCallback(self, callback, data); }, dataPtr);
		},
		"callback"_a, "info"_a, set_on_stop_callbackDoc);
		
	comp.def("set_on_frame_callback", [](Comp& self, nb::callable callback, nb::object data)
		{
			auto dataPtr = std::make_shared<nb::object>(data);
			self.setOnFrameCallback([&self, callback](CallbackData data) { doCallback(self, callback, data); }, dataPtr);
		},
		"callback"_a, "info"_a, set_on_frame_callbackDoc);

	comp.def("set_on_layout_change_callback", [](Comp& self, nb::callable callback, nb::object data)
		{
			auto dataPtr = std::make_shared<nb::object>(data);
			self.setOnLayoutChangeCallback([&self, callback](CallbackData data) { doCallback(self, callback, data); }, dataPtr);
		},
		"callback"_a, "info"_a, set_on_layout_change_callbackDoc);


	

}