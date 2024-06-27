#include "comppy.h"
#include "comp.h"

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/operators.h>

namespace nb = nanobind;
using namespace nb::literals;

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
		.doc() = CompFlagsDoc
		;

	nb::class_ <Comp::Time> time(m, "Time");
	time.doc() = TimeDoc;
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
	comp.doc() = CompDoc;
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

		.def("load", [](Comp& self, std::string path, int fps) { self.load(path, fps); } , "tox_path"_a, "fps"_a = 60, loadDoc)
		.def("unload",                 &Comp::unload, unloadDoc, nb::rv_policy::reference_internal)
		.def("start",                  &Comp::start, startDoc, nb::rv_policy::reference_internal)
		.def("loaded", &Comp::loaded, loadedDoc, nb::rv_policy::reference_internal)
		.def("stop",                   &Comp::stop, stopDoc, nb::rv_policy::reference_internal)
		.def("frame_did_finish", &Comp::frameDidFinish, frame_did_finishDoc, nb::rv_policy::reference_internal)
		.def("apply_value_changes",    &Comp::applyValueChanges, apply_value_changesDoc, nb::rv_policy::reference_internal)
		.def("start_next_frame",       &Comp::startNextFrame, "time_value"_a = 0, "time_scale"_a = 0, start_next_frameDoc, nb::rv_policy::reference_internal)
		.def("time",                   &Comp::time, timeDoc, nb::rv_policy::reference_internal)
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
		.def_prop_ro("rate", [](Comp& self) -> float { return self.frameRate(); }, frame_rateDoc)
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
		"callback"_a, "info"_a, set_on_unloaded_callbackDoc);

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