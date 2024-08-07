#pragma once


constexpr static char CompFlagsDoc[] =
R"(Enumeration of component flags.

Values:
	INTERNAL_TIME: Sets the loaded component to run in internal time mode.
	EXTERNAL_TIME: Sets the loaded component to run in external time mode.
	AUTO_UPDATE: The Comp will run a loop that updates the component.
	ASYNC_UPDATE: The Comp will run an asynchronous loop that updates the component.
	INTERNAL_TIME_AUTO: Shortcut for setting INTERNAL_TIME | AUTO_UPDATE.
	INTERNAL_TIME_ASYNC: Shortcut for setting INTERNAL_TIME | ASYNC_UPDATE.
	CUDA_STREAM_DEFAULT: Sets the CUDA stream to the default stream.
	CUDA_STREAM_INTERNAL: Sets the CUDA stream to an internal stream specific to the Comp.
	CUDA_DISABLE: Disables CUDA processing.
)";

constexpr static char TimeDoc[] =
R"(A struct containing the time information of the currently loaded component.
)";

constexpr static char CompDoc[] = 
R"(Representsa TouchDesigner component loaded in a TouchEngine instance.

Args:
	flags (CompFlags): The flags for the component (default: CompFlags.INTERNAL_TIME_AUTO | CompFlags.CUDA_STREAM_DEFAULT).
	device (int): The hardware device index to run the component on (default: 0).
	td_path (str): The path to the TouchDesigner installation if no path is set (default: "") then either:
					- The most recent TD install will be used.
					- The path set in TOUCHENGINE_APP_PATH environment variable if it exists.
					- The path set in shortcut or symlink located in the same directory as the component to be loaded.

Args:
	tox_path (str): The path to the .tox file.
	flags (CompFlags): The flags for the component.
	device (int): The hardware device to run the component on.
	fps (int): The frames per second of the TouchEngine instance.
	td_path (str): The path to the TouchDesigner installation.

Attributes:
	in_tops (List[InTOP]): The In TOPs of the currently loaded component.
	out_tops (List[OutTOP]): The Out TOPs of the currently loaded component.
	in_chops (List[InCHOP]): The In CHOPs of the currently loaded component.
	out_chops (List[OutCHOP]): The Out CHOPs of the currently loaded component.
	in_dats (List[InDAT]): The In DATs of the currently loaded component.
	out_dats (List[OutDAT]): The Out DATs of the currently loaded component.
	par (List[Par]): The parameters of the currently loaded component.
	rate (float): The frame rate of the currently loaded component.
	cuda_stream (int): The CUDA stream handle used by the CUDA device .
)";

constexpr static char loadDoc[] = 
R"(Loads a .tox file, creates and initializes a TouchEngine Instance.

Args:
	tox_path (str): The path to the .tox file.
	fps (int): The frames per second of the TouchEngine instance.

Returns:
	bool: True if the .tox file was loaded successfully, False otherwise.
)";

constexpr static char unloadDoc[] =
R"(Unloads TouchEngine instance.

Returns:
	None
)";

constexpr static char loadedDoc[] =
R"(Returns:
	bool: True if the .tox file was loaded successfully, False otherwise.
)";

constexpr static char startDoc[] =
R"(Starts the TouchEngine instance and the update loop (if AutoUpdate or AutoUpdate flags are set).
)";

constexpr static char stopDoc[] =
R"(Stops the TouchEngine instance and the update loop (if AutoUpdate or AutoUpdate flags are set).
)";

constexpr static char frame_did_finishDoc[] =
R"(Checks if the loaded component's frame has finished.

This should only be called when running the update loop manually.

Returns:
	bool: True if the frame has finished, False otherwise.
)";

constexpr static char apply_value_changesDoc[] =
R"(Applies value changes to the Comp if there are changes in the loaded component.

This should only be called when running the update loop manually, after 
frame_did_finish() returns True and before start_next_frame() is called.

Returns:
	None
)";

constexpr static char start_next_frameDoc[] =
R"(Starts the next frame.

When the component is running in INTERNAL_TIME this will instruct the TE instance to 
start the process of transfering data. When the component is running in EXTERNAL_TIME
this will instruct the component to start processing the set data and rendering the frame.	

When running the update loop manually, this must be called after apply_value_changes().
Otherwise, if the on_frame() callback is set must also be called. Call after reading data
from the component, then do work on the data and set values in the component after
start_next_frame() is called to gain performance with the cost of 1 frame of latency.

Returns:
	bool: True if frame started succesfully, False otherwise
)";

constexpr static char timeDoc[] =
R"(This method is best called at most once per frame, as it i an asynchronous call to the TouchEngine instance.

Usage:
	time_info[] = comp.time()

Returns:
	Time: a struct containing the time information of the currently loaded component.

)";

constexpr static char file_pathDoc[] =
R"((get) The path to the loaded .tox file.
)";

constexpr static char td_pathDoc[] =
R"((get) The path to the TouchDesigner installation TouchEngine is using.
)";

constexpr static char cuda_deviceDoc[] =
R"((get) The CUDA device index used by TouchEngine.
)";

constexpr static char flagsDoc[] =
R"((get) The flags (as an int) of the currently loaded tox.
)";

constexpr static char in_topsDoc[] =
R"((get) List of InTOPs of the currently loaded tox.
)";

constexpr static char out_topsDoc[] =
R"((get) List of OutTOPs of the currently loaded tox.
)";

constexpr static char in_chopsDoc[] =
R"((get) List InCHOPs of the currently loaded tox.
)";

constexpr static char out_chopsDoc[] =
R"((get) List OutCHOPs of the currently loaded tox.
)";

constexpr static char in_datsDoc[] =
R"((get) List InDATs of the currently loaded tox.
)";

constexpr static char out_datsDoc[] =
R"((get) List OutDATs of the currently loaded tox.
)";

constexpr static char parDoc[] =
R"((get) ParCollection of the currently loaded tox.
)";

constexpr static char frame_rateDoc[] =
R"((get) The frame rate of the currently loaded tox.
)";

constexpr static char cuda_streamDoc[] =
R"(The CUDA stream handle used by TouchEngine.
)";

constexpr static char set_on_loaded_callbackDoc[] =
R"(Sets the Python method to be called once the component is loaded.

Args:
	callback (Callable)	: a callable Python method
	user_data (object)	: a Python object for any userdata to be passed to the callback method

Returns:
	None
)";

constexpr static char set_on_unloaded_callbackDoc[] =
R"(Sets the Python method to be called once the component is unloaded.

Args:
	callback (Callable)	: a callable Python method
	user_data (object)	: a Python object for any userdata to be passed to the callback method

Returns:
	None
)";

constexpr static char set_on_start_callbackDoc[] =
R"(Sets the Python method to be called once the component starts.

Args:
	callback (Callable)	: a callable Python method
	user_data (object)	: a Python object for any userdata to be passed to the callback method

Returns:
	None
)";

constexpr static char set_on_stop_callbackDoc[] =
R"(Sets the Python method to be called once the component stops.

Args:
	callback (Callable)	: a callable Python method
	user_data (object)	: a Python object for any userdata to be passed to the callback method

Returns:
	None
)";

constexpr static char clear_on_frame_callbackDoc[] =
R"(Unsets any callback method set using:py:meth:`set_on_frame_callback`.

Returns:
	None
)";

constexpr static char set_on_frame_callbackDoc[] =
R"(Sets the Python method to be called everytime a frame ends.

Args:
	callback (Callable)	: a callable Python method
	user_data (object)	: a Python object for any userdata to be passed to the callback method

Returns:
	None
)";

constexpr static char clear_on_layout_change_callbackDoc[] =
R"(Unsets any callback method set using::py:meth:`set_on_layout_change_callback`.
)";

constexpr static char set_on_layout_change_callbackDoc[] =
R"(Sets the Python method to be called everytime the tox layout changes.

Args:
	callback (Callable)	: a callable Python method
	user_data (object)	: a Python object for any userdata to be passed to the callback method

Returns:
	None
)";


