from collections.abc import Callable, Sequence
import enum
from typing import Annotated, overload

import numpy
from numpy.typing import ArrayLike


class CUDADataType(enum.Enum):
    """The data type of a CUDA memory buffer"""

    UInt8 = 0

    Float16 = 2

    Float32 = 3

    Undefined = 255

class ChopChannels:
    """
    Represents an array of floating point values.

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
    """

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, array: Annotated[ArrayLike, dict(dtype='float32', shape=(None, None), device='cpu')], rate: float = -1.0, is_time_dependent: bool = False, start_time: int = 0, end_time: int = 0, channel_names: Sequence[str] = []) -> None: ...

    @overload
    def __init__(self, num_samples: int, rate: float = -1.0, is_time_dependent: bool = False, start_time: int = 0, end_time: int = 0, channel_names: Sequence[str] = []) -> None: ...

    def __repr__(self) -> str: ...

    @overload
    def __getitem__(self, name: str) -> Annotated[ArrayLike, dict(dtype='float32', shape=(None), device='cpu')]:
        """Returns the channel by name."""

    @overload
    def __getitem__(self, index: int) -> Annotated[ArrayLike, dict(dtype='float32', shape=(None), device='cpu')]:
        """Returns the channel by index."""

    @property
    def num_chans(self) -> int:
        """(get) The number of channels in the CHOP."""

    @property
    def num_samples(self) -> int:
        """(get) The number of samples in the CHOP."""

    @property
    def names(self) -> list[str]:
        """(get) List of the  channel names in this CHOP."""

    @property
    def rate(self) -> float:
        """(set, get) The sample rate of the CHOP."""

    @rate.setter
    def rate(self, arg: float, /) -> None: ...

    @property
    def is_time_dependent(self) -> bool:
        """
        (set, get) Refers to whether the corresponding CHOP is time dependent or not. i.e. audio chops are time dependent.
        """

    @is_time_dependent.setter
    def is_time_dependent(self, arg: bool, /) -> None: ...

    @property
    def start_time(self) -> int:
        """(set, get) The start time of the CHOP, in frame number * chop rate."""

    @start_time.setter
    def start_time(self, arg: int, /) -> None: ...

    @property
    def end_time(self) -> int:
        """(set, get) The end time of the CHOP, in frame number * chop rate."""

    @end_time.setter
    def end_time(self, arg: int, /) -> None: ...

    @overload
    def set_values(self, chan_index: int, values: Sequence[float], start_index: int = 0) -> None:
        """Sets the values of a channel in the CHOP."""

    @overload
    def set_values(self, chan_name: str, values: Sequence[float], start_index: int = 0) -> None:
        """Sets the values of a channel in the CHOP."""

    @overload
    def set_value(self, chan_index: int, sample_index: int, value: float) -> None:
        """Sets the value of a channel at a specific sample index."""

    @overload
    def set_value(self, chan_name: str, sample_index: int, value: float) -> None:
        """Sets the value of a channel at a specific sample index."""

    def set_name(self, index: int, name: str) -> None:
        """Sets the name of a channel."""

    def append_channel(self, name: str = '', values: Sequence[float] = []) -> None:
        """Appends a channel to the CHOP."""

    def insert_channel(self, index: int, name: str = '', values: Sequence[float] = []) -> None:
        """Inserts a channel at the specified index."""

    @overload
    def remove_channel(self, index: int) -> None:
        """Removes a channel by name or at the specified index."""

    @overload
    def remove_channel(self, name: str) -> None:
        """Removes a channel by name or at the specified index."""

    def clear(self) -> None:
        """Remove all the channels and their data."""

    def as_numpy(self) -> Annotated[ArrayLike, dict(dtype='float32', writable=False, shape=(None, None))]:
        """
        Returns all of the channels in this CHOP as 2D NumPy array with a width equal to the channel length (the number of samples) and a height equal to the number of channels.
        """

    def as_numpy_ref(self) -> Annotated[ArrayLike, dict(dtype='float32', writable=False, shape=(None, None))]:
        """
        Returns a reference to a 2D NumPy array, with a width equal to the channel length (the number of samples) and a height equal to the number of channels. The data contained in this array is read-only must explicitly be copied if values need to be manipulated. For very large arrays this will be faster than as_numpy().
        """

    def from_numpy(self, arg0: Annotated[ArrayLike, dict(dtype='float32', shape=(None, None), device='cpu')], arg1: float, arg2: bool, arg3: int, arg4: int, arg5: Sequence[str], /) -> None: ...

class Color:
    def __init__(self, r: float = 1.0, g: float = 1.0, b: float = 1.0, a: float = 1.0) -> None: ...

    @property
    def r(self) -> float: ...

    @r.setter
    def r(self, arg: float, /) -> None: ...

    @property
    def g(self) -> float: ...

    @g.setter
    def g(self, arg: float, /) -> None: ...

    @property
    def b(self) -> float: ...

    @b.setter
    def b(self, arg: float, /) -> None: ...

    @property
    def a(self) -> float: ...

    @a.setter
    def a(self, arg: float, /) -> None: ...

    def __repr__(self) -> str: ...

class ColorPar:
    """A color parameter in a TouchDesigner component"""

    def __init__(self, arg0: "TouchObject<TEInstance_>", arg1: "TouchObject<TELinkInfo>", /) -> None: ...

    @overload
    def set(self, arg: bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color, /) -> None: ...

    @overload
    def set(self, r: float = 1.0, g: float = 1.0, b: float = 1.0, a: float = 1.0) -> None: ...

    @overload
    def set(self, arg: list, /) -> None: ...

    @overload
    def set(self, arg: tuple, /) -> None: ...

    def get(self) -> bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color: ...

    @property
    def val(self) -> bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color: ...

    @val.setter
    def val(self, arg: bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color, /) -> None: ...

    @property
    def r(self) -> float: ...

    @r.setter
    def r(self, arg: float, /) -> None: ...

    @property
    def g(self) -> float: ...

    @g.setter
    def g(self, arg: float, /) -> None: ...

    @property
    def b(self) -> float: ...

    @b.setter
    def b(self, arg: float, /) -> None: ...

    @property
    def a(self) -> float: ...

    @a.setter
    def a(self, arg: float, /) -> None: ...

class Comp:
    """
    Representsa TouchDesigner component loaded in a TouchEngine instance.

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
    """

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, flags: CompFlags = 37, fps: int = 60, device: int = 0, td_path: str = '') -> None: ...

    @overload
    def __init__(self, tox_path: str, flags: CompFlags = 37, fps: int = 60, device: int = 0, td_path: str = '') -> None: ...

    def load(self, tox_path: str, fps: int = 60) -> None:
        """
        Loads a .tox file, creates and initializes a TouchEngine Instance.

        Args:
        	tox_path (str): The path to the .tox file.
        	fps (int): The frames per second of the TouchEngine instance.

        Returns:
        	bool: True if the .tox file was loaded successfully, False otherwise.
        """

    def unload(self) -> None:
        """
        Unloads TouchEngine instance.

        Returns:
        	None
        """

    def start(self) -> None:
        """
        Starts the TouchEngine instance and the update loop (if AutoUpdate or AutoUpdate flags are set).
        """

    def loaded(self) -> bool:
        """
        Returns:
        	bool: True if the .tox file was loaded successfully, False otherwise.
        """

    def stop(self) -> None:
        """
        Stops the TouchEngine instance and the update loop (if AutoUpdate or AutoUpdate flags are set).
        """

    def frame_did_finish(self) -> bool:
        """
        Checks if the loaded component's frame has finished.

        This should only be called when running the update loop manually.

        Returns:
        	bool: True if the frame has finished, False otherwise.
        """

    def apply_value_changes(self) -> None:
        """
        Applies value changes to the Comp if there are changes in the loaded component.

        This should only be called when running the update loop manually, after 
        frame_did_finish() returns True and before start_next_frame() is called.

        Returns:
        	None
        """

    def start_next_frame(self, time_value: int = 0, time_scale: int = 0) -> bool:
        """
        Starts the next frame.

        When the component is running in INTERNAL_TIME this will instruct the TE instance to 
        start the process of transfering data. When the component is running in EXTERNAL_TIME
        this will instruct the component to start processing the set data and rendering the frame.	

        When running the update loop manually, this must be called after apply_value_changes().
        Otherwise, if the on_frame() callback is set must also be called. Call after reading data
        from the component, then do work on the data and set values in the component after
        start_next_frame() is called to gain performance with the cost of 1 frame of latency.

        Returns:
        	bool: True if frame started succesfully, False otherwise
        """

    def time(self) -> Time:
        """
        This method is best called at most once per frame, as it i an asynchronous call to the TouchEngine instance.

        Usage:
        	time_info[] = comp.time()

        Returns:
        	Time: a struct containing the time information of the currently loaded component.
        """

    @property
    def file_path(self) -> str:
        """(get) The path to the loaded .tox file."""

    @property
    def td_path(self) -> str:
        """(get) The path to the TouchDesigner installation TouchEngine is using."""

    @property
    def cuda_device(self) -> int:
        """(get) The CUDA device index used by TouchEngine."""

    @property
    def flags(self) -> int:
        """(get) The flags (as an int) of the currently loaded tox."""

    @property
    def in_tops(self) -> InTops:
        """(get) List of InTOPs of the currently loaded tox."""

    @property
    def out_tops(self) -> OutTops:
        """(get) List of OutTOPs of the currently loaded tox."""

    @property
    def in_chops(self) -> InChops:
        """(get) List InCHOPs of the currently loaded tox."""

    @property
    def out_chops(self) -> OutChops:
        """(get) List OutCHOPs of the currently loaded tox."""

    @property
    def in_dats(self) -> InDats:
        """(get) List InDATs of the currently loaded tox."""

    @property
    def out_dats(self) -> OutDats:
        """(get) List OutDATs of the currently loaded tox."""

    @property
    def par(self) -> ParCollection:
        """(get) ParCollection of the currently loaded tox."""

    @property
    def rate(self) -> float:
        """(get) The frame rate of the currently loaded tox."""

    def cuda_stream(self) -> int:
        """The CUDA stream handle used by TouchEngine."""

    def clear_on_frame_callback(self) -> None:
        """
        Unsets any callback method set using:py:meth:`set_on_frame_callback`.

        Returns:
        	None
        """

    def clear_on_layout_change_callback(self) -> None:
        """
        Unsets any callback method set using::py:meth:`set_on_layout_change_callback`.
        """

    def set_on_loaded_callback(self, callback: Callable, info: object) -> None:
        """
        Sets the Python method to be called once the component is loaded.

        Args:
        	callback (Callable)	: a callable Python method
        	user_data (object)	: a Python object for any userdata to be passed to the callback method

        Returns:
        	None
        """

    def set_on_unloaded_callback(self, callback: Callable, info: object) -> None:
        """
        Sets the Python method to be called once the component is unloaded.

        Args:
        	callback (Callable)	: a callable Python method
        	user_data (object)	: a Python object for any userdata to be passed to the callback method

        Returns:
        	None
        """

    def set_on_start_callback(self, callback: Callable, info: object) -> None:
        """
        Sets the Python method to be called once the component starts.

        Args:
        	callback (Callable)	: a callable Python method
        	user_data (object)	: a Python object for any userdata to be passed to the callback method

        Returns:
        	None
        """

    def set_on_stop_callback(self, callback: Callable, info: object) -> None:
        """
        Sets the Python method to be called once the component stops.

        Args:
        	callback (Callable)	: a callable Python method
        	user_data (object)	: a Python object for any userdata to be passed to the callback method

        Returns:
        	None
        """

    def set_on_frame_callback(self, callback: Callable, info: object) -> None:
        """
        Sets the Python method to be called everytime a frame ends.

        Args:
        	callback (Callable)	: a callable Python method
        	user_data (object)	: a Python object for any userdata to be passed to the callback method

        Returns:
        	None
        """

    def set_on_layout_change_callback(self, callback: Callable, info: object) -> None:
        """
        Sets the Python method to be called everytime the tox layout changes.

        Args:
        	callback (Callable)	: a callable Python method
        	user_data (object)	: a Python object for any userdata to be passed to the callback method

        Returns:
        	None
        """

class CompFlags(enum.IntFlag):
    """
    Enumeration of component flags.

    Values:
    	INTERNAL_TIME: Sets the loaded component to run in internal time mode.
    	EXTERNAL_TIME: Sets the loaded component to run in external time mode.
    	AUTO_UPDATE: The Comp will run a loop that updates the component.
    	ASYNC_UPDATE: The Comp will run an asynchronous loop that updates the component.
    	REALTIME: If INTERNAL_TIME is set the loaded component will run in real-time mode.
    	INTERNAL_TIME_AUTO: Shortcut for setting INTERNAL_TIME | AUTO_UPDATE.
    	INTERNAL_TIME_ASYNC: Shortcut for setting INTERNAL_TIME | ASYNC_UPDATE.
    	CUDA_STREAM_DEFAULT: Sets the CUDA stream to the default stream.
    	CUDA_STREAM_INTERNAL: Sets the CUDA stream to an internal stream specific to the Comp.
    	CUDA_DISABLE: Disables CUDA processing.
    """

    __str__ = __repr__

    def __repr__(self, /):
        """Return repr(self)."""

    INTERNAL_TIME = 1

    EXTERNAL_TIME = 2

    AUTO_UPDATE = 4

    ASYNC_UPDATE = 8

    REALTIME = 16

    INTERNAL_TIME_AUTO = 5

    INTERNAL_TIME_ASYNC = 9

    CUDA_STREAM_DEFAULT = 32

    CUDA_STREAM_INTERNAL = 64

    CUDA_DISABLE = 128

class CudaFlags(enum.IntFlag):
    __str__ = __repr__

    def __repr__(self, /):
        """Return repr(self)."""

    NONE = 0

    RGBA = 1

    RGB = 2

    RG = 4

    R = 8

    BGRA = 16

    BGR = 32

    CHW = 64

    HWC = 128

    VEC4 = 256

    VEC3 = 512

    VEC2 = 1024

class CudaMemory:
    """A pointer to CUDA memory and its attributes"""

    def __init__(self) -> None: ...

    @property
    def size(self) -> int: ...

    @property
    def desc(self) -> CudaMemoryDesc: ...

    @desc.setter
    def desc(self, arg: CudaMemoryDesc, /) -> None: ...

    @property
    def ptr(self) -> int: ...

class CudaMemoryDesc:
    def __init__(self) -> None: ...

    @property
    def shape(self) -> list[int]: ...

    @shape.setter
    def shape(self, arg: Sequence[int], /) -> None: ...

    @property
    def component_size(self) -> int: ...

    @component_size.setter
    def component_size(self, arg: int, /) -> None: ...

    @property
    def data_type(self) -> CUDADataType: ...

    @data_type.setter
    def data_type(self, arg: CUDADataType, /) -> None: ...

    @property
    def strides(self) -> list[int]: ...

    @strides.setter
    def strides(self, arg: Sequence[int], /) -> None: ...

class DatTable:
    """
    Represents a 2D table of strings.

    Constructor 1:
    	Default constructor, creates an empty DatTable object.

    Constructor 2:
    	Args:
    		values (list[str]): A single list fill the table with (length must be == num_rows * num_cols).
    		num_rows (int): The number of rows in the table.
    		num_cols (int): The number of columns in the table.

    Constructor 3:
    	Args:
    		values (list[list[str]]): A list of lists to fill the table with.

    Properties:
    	num_rows (int): (set, get) The number of rows in the table.
    	num_cols (int): (set, get) The number of columns in the table.

    Methods:
    	row(int) -> list[str]: Returns a list of values from the row matching the index.
    	col(int) -> list[str]: Returns a list of values from the column matching the index.
    	cell(int, int) -> str: Returns the value at the row and column index.
    	resize(int, int): Resizes the table to the specified number of rows and columns.
    	set_num_rows(int): Sets the number of rows in the table.
    	set_num_cols(int): Sets the number of columns in the table.
    	set_cell(int, int, str): Sets the value at the row and column index.
    	set_row(int, list[str]): Sets the values of the row at the index.
    	set_col(int, list[str]): Sets the values of the column at the index.
    	append_row(list[str]): Appends a row to the table.
    	append_col(list[str]): Appends a column to the table.
    	insert_row(int, list[str]): Inserts a row at the index.
    	insert_col(int, list[str]): Inserts a column at the index.
    	remove_row(int): Removes the row at the index.
    	remove_col(int): Removes the column at the index.
    	clear(): Removes all rows and columns from the table.
    	as_list() -> list[list[str]]: Returns the table as a list of lists.
    	from_list(list[list[str]], bool) -> DatTable: Fills the table from a list of lists.
    	as_table() -> DatTable: Returns the Out DAT as touchpy.DatTable object
    	as_string() -> str: Returns the Out DAT in string format.
    """

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, num_rows: int, num_cols: int) -> None:
        """
        Represents a 2D table of strings.

        Constructor 1:
        	Default constructor, creates an empty DatTable object.

        Constructor 2:
        	Args:
        		values (list[str]): A single list fill the table with (length must be == num_rows * num_cols).
        		num_rows (int): The number of rows in the table.
        		num_cols (int): The number of columns in the table.

        Constructor 3:
        	Args:
        		values (list[list[str]]): A list of lists to fill the table with.

        Properties:
        	num_rows (int): (set, get) The number of rows in the table.
        	num_cols (int): (set, get) The number of columns in the table.

        Methods:
        	row(int) -> list[str]: Returns a list of values from the row matching the index.
        	col(int) -> list[str]: Returns a list of values from the column matching the index.
        	cell(int, int) -> str: Returns the value at the row and column index.
        	resize(int, int): Resizes the table to the specified number of rows and columns.
        	set_num_rows(int): Sets the number of rows in the table.
        	set_num_cols(int): Sets the number of columns in the table.
        	set_cell(int, int, str): Sets the value at the row and column index.
        	set_row(int, list[str]): Sets the values of the row at the index.
        	set_col(int, list[str]): Sets the values of the column at the index.
        	append_row(list[str]): Appends a row to the table.
        	append_col(list[str]): Appends a column to the table.
        	insert_row(int, list[str]): Inserts a row at the index.
        	insert_col(int, list[str]): Inserts a column at the index.
        	remove_row(int): Removes the row at the index.
        	remove_col(int): Removes the column at the index.
        	clear(): Removes all rows and columns from the table.
        	as_list() -> list[list[str]]: Returns the table as a list of lists.
        	from_list(list[list[str]], bool) -> DatTable: Fills the table from a list of lists.
        	as_table() -> DatTable: Returns the Out DAT as touchpy.DatTable object
        	as_string() -> str: Returns the Out DAT in string format.
        """

    @overload
    def __init__(self, values: Sequence[str], num_rows: int, num_cols: int) -> None:
        """
        Represents a 2D table of strings.

        Constructor 1:
        	Default constructor, creates an empty DatTable object.

        Constructor 2:
        	Args:
        		values (list[str]): A single list fill the table with (length must be == num_rows * num_cols).
        		num_rows (int): The number of rows in the table.
        		num_cols (int): The number of columns in the table.

        Constructor 3:
        	Args:
        		values (list[list[str]]): A list of lists to fill the table with.

        Properties:
        	num_rows (int): (set, get) The number of rows in the table.
        	num_cols (int): (set, get) The number of columns in the table.

        Methods:
        	row(int) -> list[str]: Returns a list of values from the row matching the index.
        	col(int) -> list[str]: Returns a list of values from the column matching the index.
        	cell(int, int) -> str: Returns the value at the row and column index.
        	resize(int, int): Resizes the table to the specified number of rows and columns.
        	set_num_rows(int): Sets the number of rows in the table.
        	set_num_cols(int): Sets the number of columns in the table.
        	set_cell(int, int, str): Sets the value at the row and column index.
        	set_row(int, list[str]): Sets the values of the row at the index.
        	set_col(int, list[str]): Sets the values of the column at the index.
        	append_row(list[str]): Appends a row to the table.
        	append_col(list[str]): Appends a column to the table.
        	insert_row(int, list[str]): Inserts a row at the index.
        	insert_col(int, list[str]): Inserts a column at the index.
        	remove_row(int): Removes the row at the index.
        	remove_col(int): Removes the column at the index.
        	clear(): Removes all rows and columns from the table.
        	as_list() -> list[list[str]]: Returns the table as a list of lists.
        	from_list(list[list[str]], bool) -> DatTable: Fills the table from a list of lists.
        	as_table() -> DatTable: Returns the Out DAT as touchpy.DatTable object
        	as_string() -> str: Returns the Out DAT in string format.
        """

    @overload
    def __init__(self, values: Sequence[Sequence[str]]) -> None:
        """
        Represents a 2D table of strings.

        Constructor 1:
        	Default constructor, creates an empty DatTable object.

        Constructor 2:
        	Args:
        		values (list[str]): A single list fill the table with (length must be == num_rows * num_cols).
        		num_rows (int): The number of rows in the table.
        		num_cols (int): The number of columns in the table.

        Constructor 3:
        	Args:
        		values (list[list[str]]): A list of lists to fill the table with.

        Properties:
        	num_rows (int): (set, get) The number of rows in the table.
        	num_cols (int): (set, get) The number of columns in the table.

        Methods:
        	row(int) -> list[str]: Returns a list of values from the row matching the index.
        	col(int) -> list[str]: Returns a list of values from the column matching the index.
        	cell(int, int) -> str: Returns the value at the row and column index.
        	resize(int, int): Resizes the table to the specified number of rows and columns.
        	set_num_rows(int): Sets the number of rows in the table.
        	set_num_cols(int): Sets the number of columns in the table.
        	set_cell(int, int, str): Sets the value at the row and column index.
        	set_row(int, list[str]): Sets the values of the row at the index.
        	set_col(int, list[str]): Sets the values of the column at the index.
        	append_row(list[str]): Appends a row to the table.
        	append_col(list[str]): Appends a column to the table.
        	insert_row(int, list[str]): Inserts a row at the index.
        	insert_col(int, list[str]): Inserts a column at the index.
        	remove_row(int): Removes the row at the index.
        	remove_col(int): Removes the column at the index.
        	clear(): Removes all rows and columns from the table.
        	as_list() -> list[list[str]]: Returns the table as a list of lists.
        	from_list(list[list[str]], bool) -> DatTable: Fills the table from a list of lists.
        	as_table() -> DatTable: Returns the Out DAT as touchpy.DatTable object
        	as_string() -> str: Returns the Out DAT in string format.
        """

    @overload
    def __getitem__(self, arg: tuple[int, int], /) -> str:
        """
        Returns the value at the row and column index.

        Args:
        	row (int) : the index of the row
        	col (int) : the index of the column
        """

    @overload
    def __getitem__(self, arg: tuple[str, int], /) -> str:
        """
        Returns the value at the row and column index.

        Args:
        	row (int) : the index of the row
        	col (int) : the index of the column
        """

    @overload
    def __getitem__(self, arg: tuple[int, str], /) -> str:
        """
        Returns the value at the row and column index.

        Args:
        	row (int) : the index of the row
        	col (int) : the index of the column
        """

    @overload
    def __getitem__(self, arg: tuple[str, str], /) -> str:
        """
        Returns the value at the row and column index.

        Args:
        	row (int) : the index of the row
        	col (int) : the index of the column
        """

    @overload
    def __setitem__(self, arg0: tuple[int, int], arg1: str, /) -> None:
        """
        Sets the value at the row and column index.

        Args:
        	row (int) : the index of the row
        	col (int) : the index of the column
        	value (str) : the value to set
        """

    @overload
    def __setitem__(self, arg0: tuple[str, int], arg1: str, /) -> None:
        """
        Sets the value at the row and column index.

        Args:
        	row (int) : the index of the row
        	col (int) : the index of the column
        	value (str) : the value to set
        """

    @overload
    def __setitem__(self, arg0: tuple[int, str], arg1: str, /) -> None:
        """
        Sets the value at the row and column index.

        Args:
        	row (int) : the index of the row
        	col (int) : the index of the column
        	value (str) : the value to set
        """

    @overload
    def __setitem__(self, arg0: tuple[str, str], arg1: str, /) -> None:
        """
        Sets the value at the row and column index.

        Args:
        	row (int) : the index of the row
        	col (int) : the index of the column
        	value (str) : the value to set
        """

    @property
    def num_rows(self) -> int:
        """(set, get) The number of rows in the table."""

    @num_rows.setter
    def num_rows(self, arg: int, /) -> None: ...

    @property
    def num_cols(self) -> int:
        """(set, get) The number of columns in the table."""

    @num_cols.setter
    def num_cols(self, arg: int, /) -> None: ...

    @overload
    def row(self, index: int) -> list[str]:
        """
        Returns a list of values from the row matching the index.

        Args:
        	index (int) : the index of the row to return
        """

    @overload
    def row(self, name: str) -> list[str]:
        """
        Returns a list of values from the row matching the index.

        Args:
        	index (int) : the index of the row to return
        """

    @overload
    def col(self, index: int) -> list[str]:
        """
        Returns a list of values from the column matching the index.

        Args:
        	index (int) : the index of the column to return
        """

    @overload
    def col(self, name: str) -> list[str]:
        """
        Returns a list of values from the column matching the index.

        Args:
        	index (int) : the index of the column to return
        """

    @overload
    def cell(self, row: int, col: int) -> str:
        """
        Returns the value at the row and column index.

        Args:
        	row (int) : the index of the row
        	col (int) : the index of the column
        """

    @overload
    def cell(self, row_name: str, col: int) -> str:
        """
        Returns the value at the row and column index.

        Args:
        	row (int) : the index of the row
        	col (int) : the index of the column
        """

    @overload
    def cell(self, row: int, col_name: str) -> str:
        """
        Returns the value at the row and column index.

        Args:
        	row (int) : the index of the row
        	col (int) : the index of the column
        """

    @overload
    def cell(self, row_name: str, col_name: str) -> str:
        """
        Returns the value at the row and column index.

        Args:
        	row (int) : the index of the row
        	col (int) : the index of the column
        """

    def resize(self, num_rows: int, num_cols: int) -> None:
        """
        Resizes the table to the specified number of rows and columns.

        Args:
        	numRows (int) : the number of rows
        	numCols (int) : the number of columns
        """

    def set_num_rows(self, num_rows: int) -> None:
        """
        Sets the number of rows in the table.

        Args:
        	numRows (int) : the number of rows
        """

    def set_num_cols(self, num_cols: int) -> None:
        """
        Sets the number of columns in the table.

        Args:
        	numCols (int) : the number of columns
        """

    @overload
    def set_cell(self, i: int, j: int, value: str) -> None:
        """
        Sets the value at the row and column index.

        Args:
        	row (int) : the index of the row
        	col (int) : the index of the column
        	value (str) : the value to set
        """

    @overload
    def set_cell(self, rowName: str, j: int, value: str) -> None:
        """
        Sets the value at the row and column index.

        Args:
        	row (int) : the index of the row
        	col (int) : the index of the column
        	value (str) : the value to set
        """

    @overload
    def set_cell(self, i: int, colName: str, value: str) -> None:
        """
        Sets the value at the row and column index.

        Args:
        	row (int) : the index of the row
        	col (int) : the index of the column
        	value (str) : the value to set
        """

    @overload
    def set_cell(self, rowName: str, colName: str, value: str) -> None:
        """
        Sets the value at the row and column index.

        Args:
        	row (int) : the index of the row
        	col (int) : the index of the column
        	value (str) : the value to set
        """

    @overload
    def set_row(self, i: int, row: Sequence[str]) -> None:
        """
        Sets the values of the row at the index.

        Lists smaller than the number of columns will be padded with empty strings.
        Lists larger than the number of columns will be truncated.

        Args:
        	i (int) : the index of the row
        	row (list) : the values to set
        """

    @overload
    def set_row(self, name: str, row: Sequence[str]) -> None:
        """
        Sets the values of the row at the index.

        Lists smaller than the number of columns will be padded with empty strings.
        Lists larger than the number of columns will be truncated.

        Args:
        	i (int) : the index of the row
        	row (list) : the values to set
        """

    @overload
    def set_col(self, j: int, col: Sequence[str]) -> None:
        """
        Sets the values of the column at the index.

        Lists smaller than the number of rows will be padded with empty strings.
        Lists larger than the number of rows will be truncated.

        Args:
        	i (int) : the index of the column
        	col (list) : the values to set
        """

    @overload
    def set_col(self, name: str, col: Sequence[str]) -> None:
        """
        Sets the values of the column at the index.

        Lists smaller than the number of rows will be padded with empty strings.
        Lists larger than the number of rows will be truncated.

        Args:
        	i (int) : the index of the column
        	col (list) : the values to set
        """

    def append_row(self, row: Sequence[str] = []) -> None:
        """
        Appends a row to the table.

        Lists smaller or larger than the number of columns will be padded or truncated respectively.

        Args:
        	row (list) : the values to append (optional)
        """

    def append_col(self, col: Sequence[str] = []) -> None:
        """
        Appends a column to the table.

        Lists smaller or larger than the number of rows will be padded or truncated respectively.

        Args:
        	col (list) : the values to append (optional)
        """

    def insert_row(self, i: int, row: Sequence[str] = []) -> None:
        """
        Inserts a row at the index.

        Lists smaller or larger than the number of columns will be padded or truncated respectively.

        Args:
        	i (int) : the index to insert the row
        	row (list) : the values to insert (optional)
        """

    def insert_col(self, j: int, col: Sequence[str] = []) -> None:
        """
        Inserts a column at the index.

        Lists smaller or larger than the number of rows will be padded or truncated respectively.

        Args:
        	i (int) : the index to insert the column
        	col (list) : the values to insert (optional)
        """

    @overload
    def remove_row(self, i: int) -> None:
        """
        Removes the row at the index.

        Args:
        	i (int) : the index of the row to remove
        """

    @overload
    def remove_row(self, name: str) -> None:
        """
        Removes the row at the index.

        Args:
        	i (int) : the index of the row to remove
        """

    @overload
    def remove_col(self, j: int) -> None:
        """
        Removes the column at the index.

        Args:
        	i (int) : the index of the column to remove
        """

    @overload
    def remove_col(self, name: str) -> None:
        """
        Removes the column at the index.

        Args:
        	i (int) : the index of the column to remove
        """

    def clear(self) -> None:
        """Removes all rows and columns from the table."""

    def as_list(self) -> list[list[str]]:
        """Returns the table as a list of lists."""

    def as_string(self) -> str:
        """Returns the Out DAT in string format."""

    def from_list(self, list: list, cast: bool = False) -> None:
        """
        Fills the table from a list of lists.

        Args:
        	list (list) : the list of lists to fill the table from
        	cast (bool) : if True, casts the values to strings (optional)
        """

class Double2Par:
    """A float2 parameter in a TouchDesigner component"""

    def __init__(self, arg0: "TouchObject<TEInstance_>", arg1: "TouchObject<TELinkInfo>", /) -> None: ...

    @overload
    def set(self, arg: bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color, /) -> None: ...

    @overload
    def set(self, x: float = 0.0, y: float = 0.0) -> None: ...

    @overload
    def set(self, arg: list, /) -> None: ...

    @overload
    def set(self, arg: tuple, /) -> None: ...

    def get(self) -> bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color: ...

    @property
    def val(self) -> bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color: ...

    @val.setter
    def val(self, arg: bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color, /) -> None: ...

    @property
    def x(self) -> float: ...

    @x.setter
    def x(self, arg: float, /) -> None: ...

    @property
    def y(self) -> float: ...

    @y.setter
    def y(self, arg: float, /) -> None: ...

class Double3Par:
    """A float3 parameter in a TouchDesigner component"""

    def __init__(self, arg0: "TouchObject<TEInstance_>", arg1: "TouchObject<TELinkInfo>", /) -> None: ...

    @overload
    def set(self, arg: bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color, /) -> None: ...

    @overload
    def set(self, x: float = 0.0, y: float = 0.0, z: float = 0.0) -> None: ...

    @overload
    def set(self, values: list) -> None: ...

    @overload
    def set(self, values: tuple) -> None: ...

    def get(self) -> bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color: ...

    @property
    def val(self) -> bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color: ...

    @val.setter
    def val(self, arg: bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color, /) -> None: ...

    @property
    def x(self) -> float: ...

    @x.setter
    def x(self, arg: float, /) -> None: ...

    @property
    def y(self) -> float: ...

    @y.setter
    def y(self, arg: float, /) -> None: ...

    @property
    def z(self) -> float: ...

    @z.setter
    def z(self, arg: float, /) -> None: ...

class Double4Par:
    """A float4 parameter in a TouchDesigner component"""

    def __init__(self, arg0: "TouchObject<TEInstance_>", arg1: "TouchObject<TELinkInfo>", /) -> None: ...

    @overload
    def set(self, arg: bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color, /) -> None: ...

    @overload
    def set(self, x: float = 0.0, y: float = 0.0, z: float = 0.0, w: float = 0.0) -> None: ...

    @overload
    def set(self, arg: list, /) -> None: ...

    @overload
    def set(self, arg: tuple, /) -> None: ...

    def get(self) -> bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color: ...

    @property
    def val(self) -> bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color: ...

    @val.setter
    def val(self, arg: bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color, /) -> None: ...

    @property
    def x(self) -> float: ...

    @x.setter
    def x(self, arg: float, /) -> None: ...

    @property
    def y(self) -> float: ...

    @y.setter
    def y(self, arg: float, /) -> None: ...

    @property
    def z(self) -> float: ...

    @z.setter
    def z(self, arg: float, /) -> None: ...

    @property
    def w(self) -> float: ...

    @w.setter
    def w(self, arg: float, /) -> None: ...

class Float2:
    def __init__(self, arg0: float, arg1: float, /) -> None: ...

    @property
    def x(self) -> float: ...

    @x.setter
    def x(self, arg: float, /) -> None: ...

    @property
    def y(self) -> float: ...

    @y.setter
    def y(self, arg: float, /) -> None: ...

    def __repr__(self) -> str: ...

class Float3:
    def __init__(self, arg0: float, arg1: float, arg2: float, /) -> None: ...

    @property
    def x(self) -> float: ...

    @x.setter
    def x(self, arg: float, /) -> None: ...

    @property
    def y(self) -> float: ...

    @y.setter
    def y(self, arg: float, /) -> None: ...

    @property
    def z(self) -> float: ...

    @z.setter
    def z(self, arg: float, /) -> None: ...

    def __repr__(self) -> str: ...

class Float4:
    def __init__(self, arg0: float, arg1: float, arg2: float, arg3: float, /) -> None: ...

    @property
    def x(self) -> float: ...

    @x.setter
    def x(self, arg: float, /) -> None: ...

    @property
    def y(self) -> float: ...

    @y.setter
    def y(self, arg: float, /) -> None: ...

    @property
    def z(self) -> float: ...

    @z.setter
    def z(self, arg: float, /) -> None: ...

    @property
    def w(self) -> float: ...

    @w.setter
    def w(self, arg: float, /) -> None: ...

    def __repr__(self) -> str: ...

class InChop:
    """Set data on an InCHOP in a loaded TouchDesigner component"""

    def __init__(self, arg0: "TouchObject<TEInstance_>", arg1: "TouchObject<TELinkInfo>", /) -> None: ...

    def from_channels(self, channels: ChopChannels)->None:
        """The CHOP channels object."""

    def from_numpy(self, array: numpy.ndarray, names: list)->None:
        """
        Sets the data in this CHOP from a 2D NumPy array.

        Args:
        	array (numpy.ndarray): The 2D NumPy array to set the data from.
        	names (list): The names of the channels in the array (optional).
        """

class InChops:
    """A container of InChop objects."""

    def __init__(self) -> None: ...

    @property
    def count(self) -> int:
        """
        Returns:
        	int: The number of InCHOPs in the loaded tox.
        """

    @property
    def names(self) -> list[str]:
        """
        Names of all In CHOPs.

        Returns: 
        	list: The names of all In CHOPs in the loaded tox.
        """

    @overload
    def __getitem__(self, arg: str, /) -> InChop:
        """Returns the In CHOP by name."""

    @overload
    def __getitem__(self, arg: int, /) -> InChop:
        """Returns the In CHOP by index."""

class InDat:
    """An interface for an InDAT in a loaded TouchDesigner component"""

    def __init__(self, arg0: "TouchObject<TEInstance_>", arg1: "TouchObject<TELinkInfo>", /) -> None: ...

    def from_table(self, arg: DatTable, /) -> None:
        """
        Fills the In DAT from a touchpy.DatTable object and sets it to Table DAT mode.
        """

    def from_string(self, arg: str, /) -> None:
        """Fills the In DAT from a string and sets it to Text DAT mode."""

    def from_list(self, list: list, cast: bool = False) -> None:
        """
        Fills the table from a list of lists.

        Args:
        	list (list) : the list of lists to fill the table from
        	cast (bool) : if True, casts the values to strings (optional)
        """

class InDats:
    """A container of InDat objects."""

    def __init__(self) -> None: ...

    @property
    def count(self) -> int:
        """Returns the number of In DATs in the loaded tox."""

    @property
    def names(self) -> list[str]:
        """Returns a list of names of all In DATs in the loaded tox."""

    @overload
    def __getitem__(self, arg: str, /) -> InDat: ...

    @overload
    def __getitem__(self, arg: int, /) -> InDat: ...

class InTop:
    """An interface for an InTOP in a loaded TouchDesigner component"""

    def __init__(self, arg0: "TouchObject<TEInstance_>", arg1: "TouchObject<TELinkInfo>", /) -> None: ...

    @overload
    def from_dlpack(self, array: Annotated[ArrayLike, dict(shape=(4, None, None), device='cuda')], flags: CudaFlags = 0) -> None: ...

    @overload
    def from_dlpack(self, array: Annotated[ArrayLike, dict(shape=(3, None, None), device='cuda')], flags: CudaFlags = 0) -> None: ...

    @overload
    def from_dlpack(self, array: Annotated[ArrayLike, dict(shape=(2, None, None), device='cuda')], flags: CudaFlags = 0) -> None: ...

    @overload
    def from_dlpack(self, array: Annotated[ArrayLike, dict(shape=(1, None, None), device='cuda')], flags: CudaFlags = 0) -> None: ...

    @overload
    def from_dlpack(self, array: Annotated[ArrayLike, dict(shape=(None, None, 4), device='cuda')], flags: CudaFlags = 0) -> None: ...

    @overload
    def from_dlpack(self, array: Annotated[ArrayLike, dict(shape=(None, None, 3), device='cuda')], flags: CudaFlags = 0) -> None: ...

    @overload
    def from_dlpack(self, array: Annotated[ArrayLike, dict(shape=(None, None, 2), device='cuda')], flags: CudaFlags = 0) -> None: ...

    @overload
    def from_dlpack(self, array: Annotated[ArrayLike, dict(shape=(None, None, 1), device='cuda')], flags: CudaFlags = 0) -> None: ...

    @overload
    def from_dlpack(self, array: Annotated[ArrayLike, dict(shape=(4, None, None), device='cuda')], stream: int, flags: CudaFlags = 0) -> None: ...

    @overload
    def from_dlpack(self, array: Annotated[ArrayLike, dict(shape=(3, None, None), device='cuda')], stream: int, flags: CudaFlags = 0) -> None: ...

    @overload
    def from_dlpack(self, array: Annotated[ArrayLike, dict(shape=(2, None, None), device='cuda')], stream: int, flags: CudaFlags = 0) -> None: ...

    @overload
    def from_dlpack(self, array: Annotated[ArrayLike, dict(shape=(1, None, None), device='cuda')], stream: int, flags: CudaFlags = 0) -> None: ...

    @overload
    def from_dlpack(self, array: Annotated[ArrayLike, dict(shape=(None, None, 4), device='cuda')], stream: int, flags: CudaFlags = 0) -> None: ...

    @overload
    def from_dlpack(self, array: Annotated[ArrayLike, dict(shape=(None, None, 3), device='cuda')], stream: int, flags: CudaFlags = 0) -> None: ...

    @overload
    def from_dlpack(self, array: Annotated[ArrayLike, dict(shape=(None, None, 2), device='cuda')], stream: int, flags: CudaFlags = 0) -> None: ...

    @overload
    def from_dlpack(self, array: Annotated[ArrayLike, dict(shape=(None, None, 1), device='cuda')], stream: int, flags: CudaFlags = 0) -> None: ...

    @overload
    def from_tensor(self, array: Annotated[ArrayLike, dict(shape=(4, None, None), device='cuda')], flags: CudaFlags = 0) -> None: ...

    @overload
    def from_tensor(self, array: Annotated[ArrayLike, dict(shape=(3, None, None), device='cuda')], flags: CudaFlags = 0) -> None: ...

    @overload
    def from_tensor(self, array: Annotated[ArrayLike, dict(shape=(2, None, None), device='cuda')], flags: CudaFlags = 0) -> None: ...

    @overload
    def from_tensor(self, array: Annotated[ArrayLike, dict(shape=(1, None, None), device='cuda')], flags: CudaFlags = 0) -> None: ...

    @overload
    def from_tensor(self, array: Annotated[ArrayLike, dict(shape=(None, None, 4), device='cuda')], flags: CudaFlags = 0) -> None: ...

    @overload
    def from_tensor(self, array: Annotated[ArrayLike, dict(shape=(None, None, 3), device='cuda')], flags: CudaFlags = 0) -> None: ...

    @overload
    def from_tensor(self, array: Annotated[ArrayLike, dict(shape=(None, None, 2), device='cuda')], flags: CudaFlags = 0) -> None: ...

    @overload
    def from_tensor(self, array: Annotated[ArrayLike, dict(shape=(None, None, 1), device='cuda')], flags: CudaFlags = 0) -> None: ...

    @overload
    def from_tensor(self, array: Annotated[ArrayLike, dict(shape=(4, None, None), device='cuda')], stream: int, flags: CudaFlags = 0) -> None: ...

    @overload
    def from_tensor(self, array: Annotated[ArrayLike, dict(shape=(3, None, None), device='cuda')], stream: int, flags: CudaFlags = 0) -> None: ...

    @overload
    def from_tensor(self, array: Annotated[ArrayLike, dict(shape=(2, None, None), device='cuda')], stream: int, flags: CudaFlags = 0) -> None: ...

    @overload
    def from_tensor(self, array: Annotated[ArrayLike, dict(shape=(1, None, None), device='cuda')], stream: int, flags: CudaFlags = 0) -> None: ...

    @overload
    def from_tensor(self, array: Annotated[ArrayLike, dict(shape=(None, None, 4), device='cuda')], stream: int, flags: CudaFlags = 0) -> None: ...

    @overload
    def from_tensor(self, array: Annotated[ArrayLike, dict(shape=(None, None, 3), device='cuda')], stream: int, flags: CudaFlags = 0) -> None: ...

    @overload
    def from_tensor(self, array: Annotated[ArrayLike, dict(shape=(None, None, 2), device='cuda')], stream: int, flags: CudaFlags = 0) -> None: ...

    @overload
    def from_tensor(self, array: Annotated[ArrayLike, dict(shape=(None, None, 1), device='cuda')], stream: int, flags: CudaFlags = 0) -> None: ...

    def copy_cuda_memory(self, cuda_mem: CudaMemory) -> None: ...

class InTops:
    """A container of InTop objects."""

    def __init__(self) -> None: ...

    @property
    def count(self) -> int: ...

    @property
    def names(self) -> list[str]: ...

    @overload
    def __getitem__(self, arg: str, /) -> InTop: ...

    @overload
    def __getitem__(self, arg: int, /) -> InTop: ...

class Int2:
    def __init__(self, arg0: int, arg1: int, /) -> None: ...

    @property
    def x(self) -> int: ...

    @x.setter
    def x(self, arg: int, /) -> None: ...

    @property
    def y(self) -> int: ...

    @y.setter
    def y(self, arg: int, /) -> None: ...

    def __repr__(self) -> str: ...

class Int2Par:
    """An int2 parameter in a TouchDesigner component"""

    def __init__(self, arg0: "TouchObject<TEInstance_>", arg1: "TouchObject<TELinkInfo>", /) -> None: ...

    @overload
    def set(self, arg: bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color, /) -> None: ...

    @overload
    def set(self, x: int = 0, y: int = 0) -> None: ...

    @overload
    def set(self, arg: list, /) -> None: ...

    @overload
    def set(self, arg: tuple, /) -> None: ...

    def get(self) -> bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color: ...

    @property
    def val(self) -> bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color: ...

    @val.setter
    def val(self, arg: bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color, /) -> None: ...

    @property
    def x(self) -> int: ...

    @x.setter
    def x(self, arg: int, /) -> None: ...

    @property
    def y(self) -> int: ...

    @y.setter
    def y(self, arg: int, /) -> None: ...

class Int3:
    def __init__(self, arg0: int, arg1: int, /) -> None: ...

    @property
    def x(self) -> int: ...

    @x.setter
    def x(self, arg: int, /) -> None: ...

    @property
    def y(self) -> int: ...

    @y.setter
    def y(self, arg: int, /) -> None: ...

    @property
    def z(self) -> int: ...

    @z.setter
    def z(self, arg: int, /) -> None: ...

    def __repr__(self) -> str: ...

class Int3Par:
    """An int3 parameter in a TouchDesigner component"""

    def __init__(self, arg0: "TouchObject<TEInstance_>", arg1: "TouchObject<TELinkInfo>", /) -> None: ...

    @overload
    def set(self, arg: bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color, /) -> None: ...

    @overload
    def set(self, x: int = 0, y: int = 0, z: int = 0) -> None: ...

    @overload
    def set(self, arg: list, /) -> None: ...

    @overload
    def set(self, arg: tuple, /) -> None: ...

    def get(self) -> bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color: ...

    @property
    def val(self) -> bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color: ...

    @val.setter
    def val(self, arg: bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color, /) -> None: ...

    @property
    def x(self) -> int: ...

    @x.setter
    def x(self, arg: int, /) -> None: ...

    @property
    def y(self) -> int: ...

    @y.setter
    def y(self, arg: int, /) -> None: ...

    @property
    def z(self) -> int: ...

    @z.setter
    def z(self, arg: int, /) -> None: ...

class Int4:
    def __init__(self, arg0: int, arg1: int, /) -> None: ...

    @property
    def x(self) -> int: ...

    @x.setter
    def x(self, arg: int, /) -> None: ...

    @property
    def y(self) -> int: ...

    @y.setter
    def y(self, arg: int, /) -> None: ...

    @property
    def z(self) -> int: ...

    @z.setter
    def z(self, arg: int, /) -> None: ...

    @property
    def w(self) -> int: ...

    @w.setter
    def w(self, arg: int, /) -> None: ...

    def __repr__(self) -> str: ...

class Int4Par:
    """An int4 parameter in a TouchDesigner component"""

    def __init__(self, arg0: "TouchObject<TEInstance_>", arg1: "TouchObject<TELinkInfo>", /) -> None: ...

    @overload
    def set(self, arg: bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color, /) -> None: ...

    @overload
    def set(self, x: int = 0, y: int = 0, z: int = 0, w: int = 0) -> None: ...

    @overload
    def set(self, arg: list, /) -> None: ...

    @overload
    def set(self, arg: tuple, /) -> None: ...

    def get(self) -> bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color: ...

    @property
    def val(self) -> bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color: ...

    @val.setter
    def val(self, arg: bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color, /) -> None: ...

    @property
    def x(self) -> int: ...

    @x.setter
    def x(self, arg: int, /) -> None: ...

    @property
    def y(self) -> int: ...

    @y.setter
    def y(self, arg: int, /) -> None: ...

    @property
    def z(self) -> int: ...

    @z.setter
    def z(self, arg: int, /) -> None: ...

    @property
    def w(self) -> int: ...

    @w.setter
    def w(self, arg: int, /) -> None: ...

class LogLevel(enum.Enum):
    """Enumeration of logging levels."""

    TRACE = 0

    DEBUG = 1

    INFO = 2

    WARN = 3

    ERROR = 4

    CRITICAL = 5

    OFF = 6

class OutChop:
    """Access data in an OutCHOP in a loaded TouchDesigner component"""

    def __init__(self, arg0: "TouchObject<TEInstance_>", arg1: "TouchObject<TELinkInfo>", /) -> None: ...

    @property
    def chan_names(self) -> list[str]:
        """(get) List of the  channel names in this CHOP."""

    def chans(self) -> ChopChannels:
        """(get) The ChopChannels member."""

    def as_numpy()-> numpy.ndarray:
        """
        Returns all of the channels in this CHOP as 2D NumPy array with a width equal to the channel length (the number of samples) and a height equal to the number of channels.
        """

    def as_numpy_ref()-> numpy.ndarray:
        """
        Returns a reference to a 2D NumPy array, with a width equal to the channel length (the number of samples) and a height equal to the number of channels. The data contained in this array is read-only must explicitly be copied if values need to be manipulated. For very large arrays this will be faster than as_numpy().
        """

class OutChops:
    """A container of OutChop objects."""

    def __init__(self) -> None: ...

    @property
    def count(self) -> int:
        """Returns the number of Out CHOPS in the loaded tox."""

    @property
    def names(self) -> list[str]:
        """Returns a list of names of all Out CHOPs in the loaded tox."""

    @overload
    def __getitem__(self, arg: str, /) -> OutChop:
        """Returns the Out CHOP by name."""

    @overload
    def __getitem__(self, arg: int, /) -> OutChop:
        """Returns the Out CHOP by index."""

class OutDat:
    """An interface for an OutDAT in a loaded TouchDesigner component"""

    def __init__(self, arg0: "TouchObject<TEInstance_>", arg1: "TouchObject<TELinkInfo>", /) -> None: ...

    def as_table(self) -> DatTable:
        """Returns the Out DAT as touchpy.DatTable object"""

    def as_string(self) -> str:
        """Returns the Out DAT in string format."""

class OutDats:
    """A container of OutDat objects."""

    def __init__(self) -> None: ...

    @property
    def count(self) -> int:
        """Returns the number of Out DATs in the loaded tox."""

    @property
    def names(self) -> list[str]:
        """Returns a list of names of all Out DATs in the loaded tox."""

    @overload
    def __getitem__(self, arg: str, /) -> OutDat: ...

    @overload
    def __getitem__(self, arg: int, /) -> OutDat: ...

class OutTop:
    """An interface for an OutTOP in a loaded TouchDesigner component"""

    def __init__(self, arg0: "TouchObject<TEInstance_>", arg1: "TouchObject<TELinkInfo>", /) -> None: ...

    def cuda_memory(self, sync_cuda_stream: bool = False) -> CudaMemory: ...

    def set_cuda_flags(self, flags: CudaFlags) -> None: ...

    def set_cuda_stream(self, stream: int) -> None: ...

    def as_dlpack(self, sync_cuda_stream: bool = False) -> ArrayLike: ...

    def as_tensor(self, sync_cuda_stream: bool = False) -> ArrayLike: ...

class OutTops:
    """A container of OutTop objects"""

    def __init__(self) -> None: ...

    @property
    def count(self) -> int: ...

    @property
    def names(self) -> list[str]: ...

    @overload
    def __getitem__(self, arg: str, /) -> OutTop: ...

    @overload
    def __getitem__(self, arg: int, /) -> OutTop: ...

class Par:
    """A parameter in a TouchDesigner component"""

    def __init__(self, arg0: "TouchObject<TEInstance_>", arg1: "TouchObject<TELinkInfo>", /) -> None: ...

    def set(self, arg: bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color, /) -> None: ...

    def get(self) -> bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color: ...

    @property
    def val(self) -> bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color: ...

    @val.setter
    def val(self, arg: bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color, /) -> None: ...

class ParCollection:
    """A collection of par Par objects"""

    def __init__(self) -> None: ...

    @property
    def count(self) -> int: ...

    @property
    def names(self) -> list[str]: ...

    def __getitem__(self, arg: str, /) -> Par: ...

class PulsePar:
    """A pulse parameter in a TouchDesigner component"""

    def __init__(self, arg0: "TouchObject<TEInstance_>", arg1: "TouchObject<TELinkInfo>", /) -> None: ...

    def set(self, arg: bool | str | int | Int2 | Int3 | Int4 | float | Float2 | Float3 | Float4 | Color, /) -> None: ...

    def pulse(self) -> None: ...

class Time:
    """
    A struct containing the time information of the currently loaded component.
    """

    def __repr__(self) -> str: ...

    @property
    def rate(self) -> float: ...

    @property
    def frame(self) -> int: ...

    @property
    def seconds(self) -> float: ...

    @property
    def value(self) -> int: ...

    @property
    def scale(self) -> int: ...

def get_dlpack_capsule_info(array: ArrayLike) -> dict:
    """
    Get information about a DLPack capsule.

    Args:
    	array (ndarray): The array to get information about.

    Returns:
    	dict: A dictionary containing information about the array.
    """

def init_logging(level: LogLevel = LogLevel.INFO, console: bool = True, file: bool = False) -> None:
    """
    Initialize the logging system.

    Args:
    	level (LogLevel): The logging level to use.
    	console (bool): Whether to log to the console.
    	file (bool): Whether to log to a file.

    Returns:
    	None
    """

def set_log_level(level: LogLevel = LogLevel.WARN) -> None:
    """
    Set the logging level.

    Args:
    	level (LogLevel): The logging level to use.

    Returns:
    	None
    """
