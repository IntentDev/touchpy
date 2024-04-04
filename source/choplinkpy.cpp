#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include "choplink.h"
#include "chopchannels.h"

#include <iostream>

namespace nb = nanobind;
using namespace nb::literals;

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
		.def("num_channels", &ChopChannels::channelCount)
		.def("num_samples", &ChopChannels::valueCount)
		.def("rate", &ChopChannels::rate)
		.def("is_time_dependent", &ChopChannels::isTimeDependent)
		.def("channel_names", &ChopChannels::channelNames, nb::rv_policy::reference_internal);

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
	outChop.def("chan_names", &OutChopLink::channelNames, nb::rv_policy::reference_internal);

	outChop.def("as_numpy", [](OutChopLink& self)
		{
			auto& chans = self.chopChannels();
			size_t shape[2] = { 
				static_cast<size_t>(chans.channelCount()),
				static_cast<size_t>(chans.valueCount())
			};
			return nb::ndarray<nb::numpy, const float, nb::ndim<2>>(self.data(), 2, shape);
		},
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
		nb::rv_policy::reference_internal);


	nb::class_<OutChopLinks> outChops(m, "OutChops");
	outChops.doc() = "A container of OutChop objects.";
	outChops.def(nb::init<>())
		.def_prop_ro("count", [](OutChopLinks& self) { return self.size(); })
		.def_prop_ro("names", [](OutChopLinks& self) { return self.getLinkNames(); }, nb::rv_policy::reference_internal)
		.def("__getitem__", [](OutChopLinks& self, const std::string& name) { return self.getLinkByName(name); }, nb::rv_policy::reference_internal)
		.def("__getitem__", [](OutChopLinks& self, size_t index) { return self.getLinkByIndex(index); }, nb::rv_policy::reference_internal)
		;

	nb::class_<InChopLink> inChop(m, "InChop");
	inChop.doc() = "An interface for an InCHOP in a loaded TouchDesigner component";
	inChop.def(nb::init<TouchObject<TEInstance>, TouchObject<TELinkInfo>>())
		.def("from_numpy", &fromNumpyToChopLink, "array"_a, "names"_a = nb::list());

	nb::class_<InChopLinks> inChops(m, "InChops");
	inChops.doc() = "A container of InChop objects.";
	inChops.def(nb::init<>())
		.def_prop_ro("count", [](InChopLinks& self) { return self.size(); })
		.def_prop_ro("names", [](InChopLinks& self) { return self.getLinkNames(); }, nb::rv_policy::reference_internal)
		.def("__getitem__", [](InChopLinks& self, const std::string& name) { return self.getLinkByName(name); }, nb::rv_policy::reference_internal)
		.def("__getitem__", [](InChopLinks& self, size_t index) { return self.getLinkByIndex(index); }, nb::rv_policy::reference_internal)
		;

}