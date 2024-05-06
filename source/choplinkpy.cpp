#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include "choplink.h"
#include "chopchannels.h"

#include <iostream>

namespace nb = nanobind;
using namespace nb::literals;

static const char* num_chansDoc =
R"((get) The number of channels in the CHOP.
)";

static const char* num_samplesDoc =
R"((get) The number of samples in the CHOP.
)";

static const char* chan_namesDoc =
R"((get) List of the  channel names in this CHOP.
)";

static const char* rateDoc =
R"((set, get) The sample rate of the CHOP.
)";

static const char* is_time_dependentDoc =
R"((set, get) Refers to whether the corresponding CHOP is time dependent or not. i.e. audio chops are time dependent. 
)";

static const char* start_timeDoc =
R"((set, get) The start time of the CHOP, in frame number * chop rate.
)";

static const char* end_timeDoc =
R"((set, get) The end time of the CHOP, in frame number * chop rate.
)";

static const char* set_valuesDoc =
R"(Sets the values of a channel in the CHOP.
)";

static const char* set_valueDoc =
R"(Sets the value of a channel at a specific sample index.
)";

static const char* set_nameDoc =
R"(Sets the name of a channel.
)";

static const char* append_channelDoc =
R"(Appends a channel to the CHOP.
)";

static const char* insert_channelDoc =
R"(Inserts a channel at the specified index.
)";

static const char* remove_channelDoc =
R"(Removes a channel by name or at the specified index.
)";

static const char* clearDoc =
R"(Remove all the channels and their data.
)";

static const char* as_numpyDoc =
R"(Returns all of the channels in this CHOP as 2D NumPy array with a width equal to the channel length (the number of samples) and a height equal to the number of channels.
)";

static const char* chansDoc =
R"((get) The ChopChannels member.
)";

static const char* as_numpy_refDoc =
R"(Returns a reference to a 2D NumPy array, with a width equal to the channel length (the number of samples) and a height equal to the number of channels. The data contained in this array is read-only must explicitly be copied if values need to be manipulated. For very large arrays this will be faster than as_numpy().
)";

static const char* countDocOutChop =
R"(Returns the number of Out CHOPS in the loaded tox.
)";

static const char* namesDocOutChop =
R"(Returns a list of names of all Out CHOPs in the loaded tox.
)";

static const char* from_numpyDoc =
R"(Sets the data in this CHOP from a 2D NumPy array.

Args:
	array (numpy.ndarray) : the 2D NumPy array to set the data from
	names (list) : the names of the channels in the array (optional)
)";


static const char* countDocInChop =
R"(Returns:
	int: number of In CHOPs in the loaded tox.
)";	

static const char* namesDocInChop =
R"(Names of all In CHOPs.

Returns: 
	list: names of all In CHOPs in the loaded tox.
)";

void fromNumpyToChopLink(
	InChopLink& inChopLink,
	nb::ndarray<float, nb::ndim<2>, nb::device::cpu> array,
	const std::vector<std::string>& channelNames)
{
	auto view = array.view();

	int32_t channelCount = static_cast<int32_t>(view.shape(0));
	uint32_t valueCount = static_cast<uint32_t>(view.shape(1));
	std::vector<const float*> channels(channelCount);

	if (channelNames.size() != channelCount)
	{
		for (int32_t i = 0; i < channelCount; ++i)
			channels[i] = view.data() + i * valueCount;

		ChopChannelsView channels(std::move(channels), channelCount, valueCount, valueCount, -1.0, false);
		inChopLink.set(std::move(channels));
	}
	else
	{
		std::vector<const char*> names(channelCount);
		for (int32_t i = 0; i < channelCount; ++i)
		{
			channels[i] = view.data() + i * valueCount;
			names[i] = channelNames[i].c_str();
		}
		ChopChannelsView channels(std::move(channels), channelCount, valueCount, valueCount, -1.0, false, std::move(names));
		inChopLink.set(std::move(channels));
	}
}

nb::ndarray<nb::numpy, const float, nb::ndim<2>> asNumpy(ChopChannels& chopChannels)
{
	size_t shape[2] = {
		static_cast<size_t>(chopChannels.channelCount()),
		static_cast<size_t>(chopChannels.valueCount())
	};
	return nb::ndarray<nb::numpy, const float, nb::ndim<2>>(chopChannels.valuesArray(), 2, shape);
}

void initChopLinkBindings(nb::module_& m)
{
	nb::class_<ChopChannels> chopChannels(m, "ChopChannels");
	chopChannels.doc() = "A container of CHOP channels";
	chopChannels.def(nb::init<>())
		.def("__init__", [](
			ChopChannels* chopChannels, 
			nb::ndarray<float, nb::ndim<2>, nb::device::cpu> array, 
			double rate = -1.0, 
			bool isTimeDependent = false, 
			int64_t startTime = 0, 
			int64_t endTime = 0, 
			const std::vector<std::string>& channelNames = {})
			{
				auto view = array.view();
				int32_t channelCount = static_cast<int32_t>(view.shape(0));
				uint32_t valueCount = static_cast<uint32_t>(view.shape(1));
				new (chopChannels) ChopChannels(view.data(), channelCount, valueCount, rate, isTimeDependent, startTime, endTime, channelNames);
			}, "array"_a, "rate"_a = -1.0, "is_time_dependent"_a = false, "start_time"_a = 0, "end_time"_a = 0, "channel_names"_a = nb::list()
		)

		.def("__init__", [](
			ChopChannels* chopChannels,
			uint32_t numSamples,
			double rate = -1.0,
			bool isTimeDependent = false,
			int64_t startTime = 0,
			int64_t endTime = 0,
			const std::vector<std::string>& channelNames = {})
			{
				new (chopChannels) ChopChannels(numSamples, rate, isTimeDependent, startTime, endTime, channelNames);
			}, "num_samples"_a, "rate"_a = -1.0, "is_time_dependent"_a = false, "start_time"_a = 0, "end_time"_a = 0, "channel_names"_a = nb::list())

		.def("__repr__", [](ChopChannels& self)
			{
				std::string repr =
					"ChopChannels(channels: " + std::to_string(self.channelCount())
					+ " samples: " + std::to_string(self.valueCount())
					+ " rate: " + std::to_string(self.rate())
					+ " time dependent: " + std::to_string(self.isTimeDependent())
					+ " start time: " + std::to_string(self.startTime())
					+ " end time: " + std::to_string(self.endTime())
					+ ")";

				return repr;
			})

		.def("__getitem__", [](ChopChannels& self, int32_t index)
			{
				auto array = nb::ndarray<nb::numpy, float, nb::ndim<1>, nb::device::cpu>(
					static_cast<void*>(self.mutableChan(index)),
					{ self.valueCount() },
					nb::handle(),
					{ 1 });

				return array;
			}, "index"_a, nb::rv_policy::reference_internal)

		.def("__getitem__", [](ChopChannels& self, const std::string name)
			{
				auto array = nb::ndarray<nb::numpy, float, nb::ndim<1>, nb::device::cpu>(
					static_cast<void*>(self.mutableChan(name.c_str())),
					{ self.valueCount() },
					nb::handle(),
					{ 1 });
				return array;
			}, "name"_a, nb::rv_policy::reference_internal)
		
		.def_prop_ro("num_chans", &ChopChannels::channelCount, num_chansDoc)
		.def_prop_ro("num_samples", &ChopChannels::valueCount, num_samplesDoc)
		.def_prop_ro("chan_names", &ChopChannels::namesBuffer, chan_namesDoc, nb::rv_policy::reference_internal)

		.def_prop_rw("rate", 
			[](ChopChannels& self) { return self.rate(); },
			[](ChopChannels& self, double value) { self.setRate(value); }, rateDoc)

		.def_prop_rw("is_time_dependent", 
			[](ChopChannels& self) { return self.isTimeDependent(); }, 
			[](ChopChannels& self, bool value) { self.setIsTimeDependent(value); }, is_time_dependentDoc)

		.def_prop_rw("start_time", 
			[](ChopChannels& self) { return self.startTime(); }, 
			[](ChopChannels& self, int64_t value) { self.setStartTime(value); }, start_timeDoc)

		.def_prop_rw("end_time", 
			[](ChopChannels& self) { return self.endTime(); },
			[](ChopChannels& self, int64_t value) { self.setEndTime(value); }, end_timeDoc)

		.def("set_values", nb::overload_cast<int32_t, const std::vector<float>&, uint32_t>(&ChopChannels::setChannelValues), "chan_index"_a, "values"_a, "start_index"_a = 0, set_valuesDoc)
		.def("set_values", nb::overload_cast<const std::string&, const std::vector<float>&, uint32_t>(&ChopChannels::setChannelValues), "chan_name"_a, "values"_a, "start_index"_a = 0, set_valuesDoc)

		.def("set_value", nb::overload_cast<int32_t, uint32_t, float>(&ChopChannels::setChannelValue), "chan_index"_a, "sample_index"_a, "value"_a, set_valueDoc)
		.def("set_value", nb::overload_cast<const std::string&, uint32_t, float>(&ChopChannels::setChannelValue), "chan_name"_a, "sample_index"_a, "value"_a, set_valueDoc)
		.def("set_name", &ChopChannels::setChannelName, "index"_a, "name"_a, set_nameDoc)

		.def("append_channel", nb::overload_cast<const std::string&, const std::vector<float>&>(&ChopChannels::appendChannel), "name"_a = "", "values"_a = nb::list(), append_channelDoc)
		.def("insert_channel", nb::overload_cast<uint32_t, const std::string&, const std::vector<float>&>(&ChopChannels::insertChannel), "index"_a, "name"_a = "", "values"_a = nb::list(), insert_channelDoc)
		.def("remove_channel", nb::overload_cast<uint32_t>(&ChopChannels::removeChannel), "index"_a, remove_channelDoc)
		.def("remove_channel", nb::overload_cast<const std::string&>(&ChopChannels::removeChannel), "name"_a, remove_channelDoc)

		.def("clear", &ChopChannels::clear, clearDoc)

		.def("as_numpy", [](ChopChannels& self) { return asNumpy(self); }, as_numpyDoc, nb::rv_policy::automatic)
		.def("as_numpy_ref", [](ChopChannels& self) { return asNumpy(self); }, as_numpy_refDoc, nb::rv_policy::reference_internal)

		.def("from_numpy",
			[](ChopChannels& self, nb::ndarray<float, nb::ndim<2>,
				nb::device::cpu> array,
				double rate,
				bool isTimeDependent,
				int64_t startTime,
				int64_t endTime,
				const std::vector<std::string>& channelNames)
			{
				auto view = array.view();
				int32_t channelCount = static_cast<int32_t>(view.shape(0));
				uint32_t valueCount = static_cast<uint32_t>(view.shape(1));
				self.setChannels(view.data(), channelCount, valueCount, rate, isTimeDependent, startTime, endTime, channelNames);

			})

		;

	nb::class_<OutChopLink> outChop(m, "OutChop");
	outChop.doc() = "An interface for an OutCHOP in a loaded TouchDesigner component";
	outChop.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>());
	outChop.def_prop_ro("chan_names", &OutChopLink::channelNames, chan_namesDoc, nb::rv_policy::reference_internal);
	outChop.def("chans", &OutChopLink::chopChannels, chansDoc, nb::rv_policy::reference_internal);

	outChop.def("as_numpy", [](OutChopLink& self)
		{
			auto& chans = self.chopChannels();
			size_t shape[2] = { 
				static_cast<size_t>(chans.channelCount()),
				static_cast<size_t>(chans.valueCount())
			};
			return nb::ndarray<nb::numpy, const float, nb::ndim<2>>(self.valuesArray(), 2, shape);
		},
		nb::sig("def as_numpy()-> numpy.ndarray"),
		as_numpyDoc, nb::rv_policy::automatic);

	outChop.def("as_numpy_ref", [](OutChopLink& self)
		{
			auto& chans = self.chopChannels();
			size_t shape[2] = {
				static_cast<size_t>(chans.channelCount()),
				static_cast<size_t>(chans.valueCount())
			};
			return nb::ndarray<nb::numpy, const float, nb::ndim<2>>(self.valuesArray(), 2, shape);
		},
		nb::sig("def as_numpy_ref()-> numpy.ndarray"),
		as_numpy_refDoc, nb::rv_policy::reference_internal);


	nb::class_<OutChopLinks> outChops(m, "OutChops");
	outChops.doc() = "A container of OutChop objects.";
	outChops.def(nb::init<>())
		.def_prop_ro("count", [](OutChopLinks& self) { return self.size(); }, countDocOutChop)
		.def_prop_ro("names", [](OutChopLinks& self) { return self.getLinkNames(); }, namesDocOutChop, nb::rv_policy::reference_internal)
		.def("__getitem__", [](OutChopLinks& self, const std::string& name) { return self.getLinkByName(name); }, nb::rv_policy::reference_internal)
		.def("__getitem__", [](OutChopLinks& self, size_t index) { return self.getLinkByIndex(index); }, nb::rv_policy::reference_internal)
		;

	nb::class_<InChopLink> inChop(m, "InChop");
	inChop.doc() = "An interface for an InCHOP in a loaded TouchDesigner component";

	inChop.def("from_channels", [](InChopLink& self, ChopChannels& channels)
		{
			self.set(ChopChannelsView(channels));
		},
		nb::sig("def from_channels(self, channels: ChopChannels)->None"),
		"channels"_a, "The CHOP channels object.");

	inChop.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("from_numpy", &fromNumpyToChopLink, "array"_a, "names"_a = nb::list(), 
			nb::sig("def from_numpy(self, array: numpy.ndarray, names: list)->None"), from_numpyDoc);

	nb::class_<InChopLinks> inChops(m, "InChops", nb::dynamic_attr());
	inChops.doc() = "A container of InChop objects.";
	inChops.def(nb::init<>())
		.def_prop_ro("count", [](InChopLinks& self) { return self.size(); }, countDocInChop)
		.def_prop_ro("names", [](InChopLinks& self) { return self.getLinkNames(); }, namesDocInChop, nb::rv_policy::reference_internal)
		.def("__getitem__", [](InChopLinks& self, const std::string& name) { return self.getLinkByName(name); }, nb::rv_policy::reference_internal)
		.def("__getitem__", [](InChopLinks& self, size_t index) { return self.getLinkByIndex(index); }, nb::rv_policy::reference_internal)
		;

}
