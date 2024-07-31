touchpy.InChop
==============

.. py:class:: touchpy.InChop(arg0: TouchObject<TEInstance_>, arg1: TouchObject<TELinkInfo>, /)

   Set data on an InCHOP in a loaded TouchDesigner component

   
   


   
   
   .. rubric:: Methods
   
   .. py:method:: from_channels(channels: ChopChannels) -> None

      The CHOP channels object.


   .. py:method:: from_numpy(array: numpy.ndarray, names: list) -> None

      Sets the data in this CHOP from a 2D NumPy array.

      :param array: The 2D NumPy array to set the data from.
      :type array: :py:class:`numpy.ndarray`
      :param names: The names of the channels in the array (optional).
      :type names: :py:class:`list`




