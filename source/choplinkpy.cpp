#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include "choplink.h"
#include "chopchannels.h"

#include <iostream>

namespace nb = nanobind;
using namespace nb::literals;

static const char* num_channelsDoc =
R"(The number of channels in the CHOP.
)";

static const char* num_samplesDoc =
R"(The number of samples in the CHOP.
)";

static const char* rateDoc =
R"(The sample rate of the CHOP.
)";

static const char* is_time_dependentDoc =
R"(////todokeith ?
)";

static const char* as_numpyDoc =
R"(Returns all of the channels in this CHOP as 2D NumPy array with a width equal to the channel length (the number of samples) and a height equal to the number of channels.
)";

static const char* chan_namesDoc =
R"(Returns a list of the  channel names in this CHOP.
)";

static const char* as_numpy_refDoc =
R"(////todokeith ?Returns a reference to a 2D NumPy array, with a width equal to the channel length (the number of samples) and a height equal to the number of channels.
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
	array (ndarray) : the 2D NumPy array to set the data from
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

		ChopChannelsReference channels(std::move(channels), channelCount, valueCount, valueCount, -1.0, false);
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
		ChopChannelsReference channels(std::move(channels), channelCount, valueCount, valueCount, -1.0, false, std::move(names));
		inChopLink.set(std::move(channels));
	}
}

void initChopLinkBindings(nb::module_& m)
{
	nb::class_<ChopChannels> chopChannels(m, "ChopChannels");
	chopChannels.doc() = "A container of CHOP channels";
	chopChannels.def(nb::init<>())
		.def_prop_ro("num_channels", &ChopChannels::channelCount, num_channelsDoc)
		.def_prop_ro("num_samples", &ChopChannels::valueCount, num_samplesDoc)
		.def_prop_ro("rate", &ChopChannels::rate, rateDoc)
		.def_prop_ro("is_time_dependent", &ChopChannels::isTimeDependent, is_time_dependentDoc)
		.def_prop_ro("channel_names", &ChopChannels::channelNames, chan_namesDoc, nb::rv_policy::reference_internal);

	chopChannels.def("as_numpy", [](ChopChannels& self)
		{
			size_t shape[2] = {
				static_cast<size_t>(self.channelCount()),
				static_cast<size_t>(self.valueCount())
			};
			return nb::ndarray<nb::numpy, const float, nb::ndim<2>>(self.data(), 2, shape);
		},
		nb::rv_policy::automatic);

	nb::class_<OutChopLink> outChop(m, "OutChop");
	outChop.doc() = "An interface for an OutCHOP in a loaded TouchDesigner component";
	outChop.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>());
	outChop.def_prop_ro("chan_names", &OutChopLink::channelNames, chan_namesDoc, nb::rv_policy::reference_internal);

	outChop.def("as_numpy", [](OutChopLink& self)
		{
			auto& chans = self.chopChannels();
			size_t shape[2] = { 
				static_cast<size_t>(chans.channelCount()),
				static_cast<size_t>(chans.valueCount())
			};
			return nb::ndarray<nb::numpy, const float, nb::ndim<2>>(self.data(), 2, shape);
		},
		nb::sig("def as_numpy(self) -> array_like"),
		nb::rv_policy::automatic);

	outChop.def("as_numpy_ref", [](OutChopLink& self)
		{
			auto& chans = self.chopChannels();
			size_t shape[2] = {
				static_cast<size_t>(chans.channelCount()),
				static_cast<size_t>(chans.valueCount())
			};
			return nb::ndarray<nb::numpy, const float, nb::ndim<2>>(self.data(), 2, shape);
		},
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
	inChop.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("from_numpy", &fromNumpyToChopLink, "array"_a, "names"_a = nb::list(), from_numpyDoc);

	nb::class_<InChopLinks> inChops(m, "InChops", nb::dynamic_attr());
	inChops.doc() = "A container of InChop objects.";
	inChops.def(nb::init<>())
		.def_prop_ro("count", [](InChopLinks& self) { return self.size(); }, countDocInChop)
		.def_prop_ro("names", [](InChopLinks& self) { return self.getLinkNames(); }, namesDocInChop, nb::rv_policy::reference_internal)
		.def("__getitem__", [](InChopLinks& self, const std::string& name) { return self.getLinkByName(name); }, nb::rv_policy::reference_internal)
		.def("__getitem__", [](InChopLinks& self, size_t index) { return self.getLinkByIndex(index); }, nb::rv_policy::reference_internal)
		;

}