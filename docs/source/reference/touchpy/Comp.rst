touchpy.Comp
============

.. py:class:: touchpy.Comp
              Comp(flags: CompFlags = 37, fps: int = 60, device: int = 0, td_path: str = '')
              Comp(tox_path: str, flags: CompFlags = 37, fps: int = 60, device: int = 0, td_path: str = '')

   Representsa TouchDesigner component loaded in a TouchEngine instance.

   :param flags: The flags for the component (default: CompFlags.INTERNAL_TIME_AUTO | CompFlags.CUDA_STREAM_DEFAULT).
   :type flags: :py:class:`CompFlags`
   :param device: The hardware device index to run the component on (default: 0).
   :type device: :py:class:`int`
   :param td_path: The path to the TouchDesigner installation if no path is set (default: "") then either:
                   - The most recent TD install will be used.
                   - The path set in TOUCHENGINE_APP_PATH environment variable if it exists.
                   - The path set in shortcut or symlink located in the same directory as the component to be loaded.
   :type td_path: :py:class:`str`

   :param tox_path: The path to the .tox file.
   :type tox_path: :py:class:`str`
   :param flags: The flags for the component.
   :type flags: :py:class:`CompFlags`
   :param device: The hardware device to run the component on.
   :type device: :py:class:`int`
   :param fps: The frames per second of the TouchEngine instance.
   :type fps: :py:class:`int`
   :param td_path: The path to the TouchDesigner installation.
   :type td_path: :py:class:`str`

   .. attribute:: in_tops

      The In TOPs of the currently loaded component.

      :type: :py:class:`List[InTOP]`

   .. attribute:: out_tops

      The Out TOPs of the currently loaded component.

      :type: :py:class:`List[OutTOP]`

   .. attribute:: in_chops

      The In CHOPs of the currently loaded component.

      :type: :py:class:`List[InCHOP]`

   .. attribute:: out_chops

      The Out CHOPs of the currently loaded component.

      :type: :py:class:`List[OutCHOP]`

   .. attribute:: in_dats

      The In DATs of the currently loaded component.

      :type: :py:class:`List[InDAT]`

   .. attribute:: out_dats

      The Out DATs of the currently loaded component.

      :type: :py:class:`List[OutDAT]`

   .. attribute:: par

      The parameters of the currently loaded component.

      :type: :py:class:`List[Par]`

   .. attribute:: rate

      The frame rate of the currently loaded component.

      :type: :py:class:`float`

   .. attribute:: cuda_stream

      The CUDA stream handle used by the CUDA device .

      :type: :py:class:`int`

   
   

   .. rubric:: Attributes

   .. py:property:: cuda_device
      :type: int

      (get) The CUDA device index used by TouchEngine.

   .. py:property:: file_path
      :type: str

      (get) The path to the loaded .tox file.

   .. py:property:: flags
      :type: int

      (get) The flags (as an int) of the currently loaded tox.

   .. py:property:: in_chops
      :type: InChops

      (get) List InCHOPs of the currently loaded tox.

   .. py:property:: in_dats
      :type: InDats

      (get) List InDATs of the currently loaded tox.

   .. py:property:: in_tops
      :type: InTops

      (get) List of InTOPs of the currently loaded tox.

   .. py:property:: out_chops
      :type: OutChops

      (get) List OutCHOPs of the currently loaded tox.

   .. py:property:: out_dats
      :type: OutDats

      (get) List OutDATs of the currently loaded tox.

   .. py:property:: out_tops
      :type: OutTops

      (get) List of OutTOPs of the currently loaded tox.

   .. py:property:: par
      :type: ParCollection

      (get) ParCollection of the currently loaded tox.

   .. py:property:: rate
      :type: float

      (get) The frame rate of the currently loaded tox.

   .. py:property:: td_path
      :type: str

      (get) The path to the TouchDesigner installation TouchEngine is using.


   
   
   .. rubric:: Methods
   
   .. py:method:: apply_value_changes() -> None

      Applies value changes to the Comp if there are changes in the loaded component.

      This should only be called when running the update loop manually, after
      frame_did_finish() returns True and before start_next_frame() is called.

      :returns: None


   .. py:method:: clear_on_frame_callback() -> None

      Unsets any callback method set using:py:meth:`set_on_frame_callback`.

      :returns: None


   .. py:method:: clear_on_layout_change_callback() -> None

      Unsets any callback method set using::py:meth:`set_on_layout_change_callback`.


   .. py:method:: cuda_stream() -> int

      The CUDA stream handle used by TouchEngine.


   .. py:method:: frame_did_finish() -> bool

      Checks if the loaded component's frame has finished.

      This should only be called when running the update loop manually.

      :returns: True if the frame has finished, False otherwise.
      :rtype: bool


   .. py:method:: load(tox_path: str, fps: int = 60) -> None

      Loads a .tox file, creates and initializes a TouchEngine Instance.

      :param tox_path: The path to the .tox file.
      :type tox_path: :py:class:`str`
      :param fps: The frames per second of the TouchEngine instance.
      :type fps: :py:class:`int`

      :returns: True if the .tox file was loaded successfully, False otherwise.
      :rtype: bool


   .. py:method:: loaded() -> bool

      :returns: True if the .tox file was loaded successfully, False otherwise.
      :rtype: bool


   .. py:method:: set_on_frame_callback(callback: collections.abc.Callable, info: object) -> None

      Sets the Python method to be called everytime a frame ends.

      :param callback: a callable Python method
      :type callback: collections.abc.Callable
      :param user_data: a Python object for any userdata to be passed to the callback method
      :type user_data: :py:class:`object`

      :returns: None


   .. py:method:: set_on_layout_change_callback(callback: collections.abc.Callable, info: object) -> None

      Sets the Python method to be called everytime the tox layout changes.

      :param callback: a callable Python method
      :type callback: collections.abc.Callable
      :param user_data: a Python object for any userdata to be passed to the callback method
      :type user_data: :py:class:`object`

      :returns: None


   .. py:method:: set_on_loaded_callback(callback: collections.abc.Callable, info: object) -> None

      Sets the Python method to be called once the component is loaded.

      :param callback: a callable Python method
      :type callback: collections.abc.Callable
      :param user_data: a Python object for any userdata to be passed to the callback method
      :type user_data: :py:class:`object`

      :returns: None


   .. py:method:: set_on_start_callback(callback: collections.abc.Callable, info: object) -> None

      Sets the Python method to be called once the component starts.

      :param callback: a callable Python method
      :type callback: collections.abc.Callable
      :param user_data: a Python object for any userdata to be passed to the callback method
      :type user_data: :py:class:`object`

      :returns: None


   .. py:method:: set_on_stop_callback(callback: collections.abc.Callable, info: object) -> None

      Sets the Python method to be called once the component stops.

      :param callback: a callable Python method
      :type callback: collections.abc.Callable
      :param user_data: a Python object for any userdata to be passed to the callback method
      :type user_data: :py:class:`object`

      :returns: None


   .. py:method:: set_on_unloaded_callback(callback: collections.abc.Callable, info: object) -> None

      Sets the Python method to be called once the component is unloaded.

      :param callback: a callable Python method
      :type callback: collections.abc.Callable
      :param user_data: a Python object for any userdata to be passed to the callback method
      :type user_data: :py:class:`object`

      :returns: None


   .. py:method:: start() -> None

      Starts the TouchEngine instance and the update loop (if AutoUpdate or AutoUpdate flags are set).


   .. py:method:: start_next_frame(time_value: int = 0, time_scale: int = 0) -> bool

      Starts the next frame.

      When the component is running in INTERNAL_TIME this will instruct the TE instance to
      start the process of transfering data. When the component is running in EXTERNAL_TIME
      this will instruct the component to start processing the set data and rendering the frame.

      When running the update loop manually, this must be called after apply_value_changes().
      Otherwise, if the on_frame() callback is set must also be called. Call after reading data
      from the component, then do work on the data and set values in the component after
      start_next_frame() is called to gain performance with the cost of 1 frame of latency.

      :returns: True if frame started succesfully, False otherwise
      :rtype: bool


   .. py:method:: stop() -> None

      Stops the TouchEngine instance and the update loop (if AutoUpdate or AutoUpdate flags are set).


   .. py:method:: time() -> Time

      This method is best called at most once per frame, as it i an asynchronous call to the TouchEngine instance.

      Usage:
              time_info[] = comp.time()

      :returns: a struct containing the time information of the currently loaded component.
      :rtype: Time


   .. py:method:: unload() -> None

      Unloads TouchEngine instance.

      :returns: None




