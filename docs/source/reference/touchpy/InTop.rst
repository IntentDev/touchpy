touchpy.InTop
=============

.. py:class:: touchpy.InTop(arg0: TouchObject<TEInstance_>, arg1: TouchObject<TELinkInfo>, /)

   An interface for an InTOP in a loaded TouchDesigner component

   
   


   
   
   .. rubric:: Methods
   
   .. py:method:: copy_cuda_memory(cuda_mem: CudaMemory) -> None


   .. py:method:: from_dlpack(array: Annotated[numpy.typing.ArrayLike, dict(shape=(4, None, None), device='cuda')], flags: CudaFlags = 0) -> None
                  from_dlpack(array: Annotated[numpy.typing.ArrayLike, dict(shape=(3, None, None), device='cuda')], flags: CudaFlags = 0) -> None
                  from_dlpack(array: Annotated[numpy.typing.ArrayLike, dict(shape=(2, None, None), device='cuda')], flags: CudaFlags = 0) -> None
                  from_dlpack(array: Annotated[numpy.typing.ArrayLike, dict(shape=(1, None, None), device='cuda')], flags: CudaFlags = 0) -> None
                  from_dlpack(array: Annotated[numpy.typing.ArrayLike, dict(shape=(None, None, 4), device='cuda')], flags: CudaFlags = 0) -> None
                  from_dlpack(array: Annotated[numpy.typing.ArrayLike, dict(shape=(None, None, 3), device='cuda')], flags: CudaFlags = 0) -> None
                  from_dlpack(array: Annotated[numpy.typing.ArrayLike, dict(shape=(None, None, 2), device='cuda')], flags: CudaFlags = 0) -> None
                  from_dlpack(array: Annotated[numpy.typing.ArrayLike, dict(shape=(None, None, 1), device='cuda')], flags: CudaFlags = 0) -> None
                  from_dlpack(array: Annotated[numpy.typing.ArrayLike, dict(shape=(4, None, None), device='cuda')], stream: int, flags: CudaFlags = 0) -> None
                  from_dlpack(array: Annotated[numpy.typing.ArrayLike, dict(shape=(3, None, None), device='cuda')], stream: int, flags: CudaFlags = 0) -> None
                  from_dlpack(array: Annotated[numpy.typing.ArrayLike, dict(shape=(2, None, None), device='cuda')], stream: int, flags: CudaFlags = 0) -> None
                  from_dlpack(array: Annotated[numpy.typing.ArrayLike, dict(shape=(1, None, None), device='cuda')], stream: int, flags: CudaFlags = 0) -> None
                  from_dlpack(array: Annotated[numpy.typing.ArrayLike, dict(shape=(None, None, 4), device='cuda')], stream: int, flags: CudaFlags = 0) -> None
                  from_dlpack(array: Annotated[numpy.typing.ArrayLike, dict(shape=(None, None, 3), device='cuda')], stream: int, flags: CudaFlags = 0) -> None
                  from_dlpack(array: Annotated[numpy.typing.ArrayLike, dict(shape=(None, None, 2), device='cuda')], stream: int, flags: CudaFlags = 0) -> None
                  from_dlpack(array: Annotated[numpy.typing.ArrayLike, dict(shape=(None, None, 1), device='cuda')], stream: int, flags: CudaFlags = 0) -> None


   .. py:method:: from_tensor(array: Annotated[numpy.typing.ArrayLike, dict(shape=(4, None, None), device='cuda')], flags: CudaFlags = 0) -> None
                  from_tensor(array: Annotated[numpy.typing.ArrayLike, dict(shape=(3, None, None), device='cuda')], flags: CudaFlags = 0) -> None
                  from_tensor(array: Annotated[numpy.typing.ArrayLike, dict(shape=(2, None, None), device='cuda')], flags: CudaFlags = 0) -> None
                  from_tensor(array: Annotated[numpy.typing.ArrayLike, dict(shape=(1, None, None), device='cuda')], flags: CudaFlags = 0) -> None
                  from_tensor(array: Annotated[numpy.typing.ArrayLike, dict(shape=(None, None, 4), device='cuda')], flags: CudaFlags = 0) -> None
                  from_tensor(array: Annotated[numpy.typing.ArrayLike, dict(shape=(None, None, 3), device='cuda')], flags: CudaFlags = 0) -> None
                  from_tensor(array: Annotated[numpy.typing.ArrayLike, dict(shape=(None, None, 2), device='cuda')], flags: CudaFlags = 0) -> None
                  from_tensor(array: Annotated[numpy.typing.ArrayLike, dict(shape=(None, None, 1), device='cuda')], flags: CudaFlags = 0) -> None
                  from_tensor(array: Annotated[numpy.typing.ArrayLike, dict(shape=(4, None, None), device='cuda')], stream: int, flags: CudaFlags = 0) -> None
                  from_tensor(array: Annotated[numpy.typing.ArrayLike, dict(shape=(3, None, None), device='cuda')], stream: int, flags: CudaFlags = 0) -> None
                  from_tensor(array: Annotated[numpy.typing.ArrayLike, dict(shape=(2, None, None), device='cuda')], stream: int, flags: CudaFlags = 0) -> None
                  from_tensor(array: Annotated[numpy.typing.ArrayLike, dict(shape=(1, None, None), device='cuda')], stream: int, flags: CudaFlags = 0) -> None
                  from_tensor(array: Annotated[numpy.typing.ArrayLike, dict(shape=(None, None, 4), device='cuda')], stream: int, flags: CudaFlags = 0) -> None
                  from_tensor(array: Annotated[numpy.typing.ArrayLike, dict(shape=(None, None, 3), device='cuda')], stream: int, flags: CudaFlags = 0) -> None
                  from_tensor(array: Annotated[numpy.typing.ArrayLike, dict(shape=(None, None, 2), device='cuda')], stream: int, flags: CudaFlags = 0) -> None
                  from_tensor(array: Annotated[numpy.typing.ArrayLike, dict(shape=(None, None, 1), device='cuda')], stream: int, flags: CudaFlags = 0) -> None




