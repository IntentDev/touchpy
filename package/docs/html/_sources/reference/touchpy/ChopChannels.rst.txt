touchpy.ChopChannels
====================

.. py:class:: touchpy.ChopChannels
              ChopChannels(array: Annotated[numpy.typing.ArrayLike, dict(dtype='float32', shape=(None, None), device='cpu')], rate: float = -1.0, is_time_dependent: bool = False, start_time: int = 0, end_time: int = 0, channel_names: collections.abc.Sequence[str] = [])
              ChopChannels(num_samples: int, rate: float = -1.0, is_time_dependent: bool = False, start_time: int = 0, end_time: int = 0, channel_names: collections.abc.Sequence[str] = [])

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

   
   

   .. rubric:: Attributes

   .. py:property:: end_time
      :type: int

      (set, get) The end time of the CHOP, in frame number * chop rate.

   .. py:property:: is_time_dependent
      :type: bool

      (set, get) Refers to whether the corresponding CHOP is time dependent or not. i.e. audio chops are time dependent.

   .. py:property:: names
      :type: list[str]

      (get) List of the  channel names in this CHOP.

   .. py:property:: num_chans
      :type: int

      (get) The number of channels in the CHOP.

   .. py:property:: num_samples
      :type: int

      (get) The number of samples in the CHOP.

   .. py:property:: rate
      :type: float

      (set, get) The sample rate of the CHOP.

   .. py:property:: start_time
      :type: int

      (set, get) The start time of the CHOP, in frame number * chop rate.


   
   
   .. rubric:: Methods
   
   .. py:method:: append_channel(name: str = '', values: collections.abc.Sequence[float] = []) -> None

      Appends a channel to the CHOP.


   .. py:method:: as_numpy() -> Annotated[numpy.typing.ArrayLike, dict(dtype='float32', writable=False, shape=(None, None))]

      Returns all of the channels in this CHOP as 2D NumPy array with a width equal to the channel length (the number of samples) and a height equal to the number of channels.


   .. py:method:: as_numpy_ref() -> Annotated[numpy.typing.ArrayLike, dict(dtype='float32', writable=False, shape=(None, None))]

      Returns a reference to a 2D NumPy array, with a width equal to the channel length (the number of samples) and a height equal to the number of channels. The data contained in this array is read-only must explicitly be copied if values need to be manipulated. For very large arrays this will be faster than as_numpy().


   .. py:method:: clear() -> None

      Remove all the channels and their data.


   .. py:method:: from_numpy(arg0: Annotated[numpy.typing.ArrayLike, dict(dtype='float32', shape=(None, None), device='cpu')], arg1: float, arg2: bool, arg3: int, arg4: int, arg5: collections.abc.Sequence[str], /) -> None


   .. py:method:: insert_channel(index: int, name: str = '', values: collections.abc.Sequence[float] = []) -> None

      Inserts a channel at the specified index.


   .. py:method:: remove_channel(index: int) -> None
                  remove_channel(name: str) -> None

      Removes a channel by name or at the specified index.


   .. py:method:: set_name(index: int, name: str) -> None

      Sets the name of a channel.


   .. py:method:: set_value(chan_index: int, sample_index: int, value: float) -> None
                  set_value(chan_name: str, sample_index: int, value: float) -> None

      Sets the value of a channel at a specific sample index.


   .. py:method:: set_values(chan_index: int, values: collections.abc.Sequence[float], start_index: int = 0) -> None
                  set_values(chan_name: str, values: collections.abc.Sequence[float], start_index: int = 0) -> None

      Sets the values of a channel in the CHOP.




