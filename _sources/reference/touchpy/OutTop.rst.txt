touchpy.OutTop
==============

.. py:class:: touchpy.OutTop(arg0: TouchObject<TEInstance_>, arg1: TouchObject<TELinkInfo>, /)

   An interface for an OutTOP in a loaded TouchDesigner component

   
   


   
   
   .. rubric:: Methods
   
   .. py:method:: as_dlpack(sync_cuda_stream: bool = False) -> numpy.typing.ArrayLike


   .. py:method:: as_tensor(sync_cuda_stream: bool = False) -> numpy.typing.ArrayLike


   .. py:method:: cuda_memory(sync_cuda_stream: bool = False) -> CudaMemory


   .. py:method:: set_cuda_flags(flags: CudaFlags) -> None


   .. py:method:: set_cuda_stream(stream: int) -> None




