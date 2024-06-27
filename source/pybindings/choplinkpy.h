#pragma once


constexpr static char ChopChannelsDoc[] =
R"(Represents an array of floating point values.

A ChopChannels object is an array of floating point values that has similar 
structure as 2D Numpy array but can operated on in a similar fashion to a CHOP
in TD. The data is stored in a single contiguous block of memory but is still
accessble by channel (name or index) and sample index.

Constructor 1:
	Default constructor, creates an empty ChopChannels object.

Constructor 2:
	Args:
		array (numpy.ndarray): A 2D NumPy array to set the data from.
		rate (float): The sample rate of the CHOP (optional, default is -1.0 which means the rate is not set).
		is_time_dependent (bool): Whether the CHOP is time dependent or not (optional, default is False).
		start_time (float): The start time of the CHOP (optional, default is 0.0).
		end_time (float): The end time of the CHOP (optional, default is 0.0).
		channel_names (list): The names of the channels in the array (optional, default is []).

Constructor 3:
	Args:
		num_samples (int): The number of samples in the CHOP.
		rate (float): The sample rate of the CHOP (optional, default is -1.0 which means the rate is not set).
		is_time_dependent (bool): Whether the CHOP is time dependent or not (optional, default is False).
		start_time (float): The start time of the CHOP (optional, default is 0.0).
		end_time (float): The end time of the CHOP (optional, default is 0.0).
		channel_names (list): The names of the channels in the array (optional, default is []).

Properties:
	num_chans (int): (get) The number of channels in the CHOP.
	num_samples (int): (get) The number of samples in the CHOP.
	chan_names (list): (get) List of the channel names in this CHOP.
	rate (float): (get, set) The sample rate of the CHOP.
	is_time_dependent (bool): (get, set) Refers to whether the corresponding CHOP is time dependent or not. i.e. audio chops are time dependent.
	start_time (float): (get, set) The start time of the CHOP, in frame number * chop rate.
	end_time (float): (get, set) The end time of the CHOP, in frame number * chop rate.

)";

constexpr static char num_chansDoc[] =
R"((get) The number of channels in the CHOP.
)";

constexpr static char num_samplesDoc[] =
R"((get) The number of samples in the CHOP.
)";

constexpr static char chan_namesDoc[] =
R"((get) List of the  channel names in this CHOP.
)";

constexpr static char rateDoc[] =
R"((set, get) The sample rate of the CHOP.
)";

constexpr static char is_time_dependentDoc[] =
R"((set, get) Refers to whether the corresponding CHOP is time dependent or not. i.e. audio chops are time dependent. 
)";

constexpr static char start_timeDoc[] =
R"((set, get) The start time of the CHOP, in frame number * chop rate.
)";

constexpr static char end_timeDoc[] =
R"((set, get) The end time of the CHOP, in frame number * chop rate.
)";

constexpr static char set_valuesDoc[] =
R"(Sets the values of a channel in the CHOP.
)";

constexpr static char set_valueDoc[] =
R"(Sets the value of a channel at a specific sample index.
)";

constexpr static char set_nameDoc[] =
R"(Sets the name of a channel.
)";

constexpr static char append_channelDoc[] =
R"(Appends a channel to the CHOP.
)";

constexpr static char insert_channelDoc[] =
R"(Inserts a channel at the specified index.
)";

constexpr static char remove_channelDoc[] =
R"(Removes a channel by name or at the specified index.
)";

constexpr static char clearDoc[] =
R"(Remove all the channels and their data.
)";

constexpr static char as_numpyDoc[] =
R"(Returns all of the channels in this CHOP as 2D NumPy array with a width equal to the channel length (the number of samples) and a height equal to the number of channels.
)";

constexpr static char chansDoc[] =
R"((get) The ChopChannels member.
)";

constexpr static char as_numpy_refDoc[] =
R"(Returns a reference to a 2D NumPy array, with a width equal to the channel length (the number of samples) and a height equal to the number of channels. The data contained in this array is read-only must explicitly be copied if values need to be manipulated. For very large arrays this will be faster than as_numpy().
)";

constexpr static char countDocOutChop[] =
R"(Returns the number of Out CHOPS in the loaded tox.
)";

constexpr static char namesDocOutChop[] =
R"(Returns a list of names of all Out CHOPs in the loaded tox.
)";

constexpr static char from_numpyDoc[] =
R"(Sets the data in this CHOP from a 2D NumPy array.

Args:
	array (numpy.ndarray): The 2D NumPy array to set the data from.
	names (list): The names of the channels in the array (optional).
)";


constexpr static char countDocInChop[] =
R"(Returns:
	int: The number of InCHOPs in the loaded tox.
)";	

constexpr static char namesDocInChop[] =
R"(Names of all In CHOPs.

Returns: 
	list: The names of all In CHOPs in the loaded tox.
)";

constexpr static char Chans__getitem__byNameDoc[] =
R"(Returns the channel by name.
)";

constexpr static char Chans__getitem__byIndexDoc[] =
R"(Returns the channel by index.
)";

constexpr static char OutChops__getitem__byNameDoc[] =
R"(Returns the Out CHOP by name.
)";

constexpr static char OutChops__getitem__byIndexDoc[] =
R"(Returns the Out CHOP by index.
)";

constexpr static char InChops__getitem__byNameDoc[] =
R"(Returns the In CHOP by name.
)";

constexpr static char InChops__getitem__byIndexDoc[] =
R"(Returns the In CHOP by index.
)";



