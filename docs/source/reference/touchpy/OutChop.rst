touchpy.OutChop
===============

.. py:class:: touchpy.OutChop(arg0: TouchObject<TEInstance_>, arg1: TouchObject<TELinkInfo>, /)

   Access data in an OutCHOP in a loaded TouchDesigner component

   
   

   .. rubric:: Attributes

   .. py:property:: chan_names
      :type: list[str]

      (get) List of the  channel names in this CHOP.


   
   
   .. rubric:: Methods
   
   .. py:method:: as_numpy() -> numpy.ndarray

      Returns all of the channels in this CHOP as 2D NumPy array with a width equal to the channel length (the number of samples) and a height equal to the number of channels.


   .. py:method:: as_numpy_ref() -> numpy.ndarray

      Returns a reference to a 2D NumPy array, with a width equal to the channel length (the number of samples) and a height equal to the number of channels. The data contained in this array is read-only must explicitly be copied if values need to be manipulated. For very large arrays this will be faster than as_numpy().


   .. py:method:: chans() -> ChopChannels

      (get) The ChopChannels member.




