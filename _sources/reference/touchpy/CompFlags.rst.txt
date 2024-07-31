touchpy.CompFlags
=================

.. py:class:: touchpy.CompFlags



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

   
   

   .. rubric:: Attributes

   .. py:attribute:: ASYNC_UPDATE
      :value: '8'

      

   .. py:attribute:: AUTO_UPDATE
      :value: '4'

      

   .. py:attribute:: CUDA_DISABLE
      :value: '128'

      

   .. py:attribute:: CUDA_STREAM_DEFAULT
      :value: '32'

      

   .. py:attribute:: CUDA_STREAM_INTERNAL
      :value: '64'

      

   .. py:attribute:: EXTERNAL_TIME
      :value: '2'

      

   .. py:attribute:: INTERNAL_TIME
      :value: '1'

      

   .. py:attribute:: INTERNAL_TIME_ASYNC
      :value: '9'

      

   .. py:attribute:: INTERNAL_TIME_AUTO
      :value: '5'

      

   .. py:attribute:: REALTIME
      :value: '16'

      




