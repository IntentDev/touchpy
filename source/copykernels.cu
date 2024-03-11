
#include "copykernels.cuh"

cudaError_t
memCopyBRGA8USurfaceToRGBA8U(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream)
{
	dim3 blockSize(16, 16, 1);
	dim3 gridSize(divUp(width, blockSize.x), divUp(height, blockSize.y), 1);
	fromSurfaceBRGA8UToRGBA8U <<<gridSize, blockSize, 0, stream>>> (dst, width, height, src);

	CHECK_CUDA_ERROR_AND_RETURN_STATUS(cudaDeviceSynchronize());
}

template<typename T> cudaError_t
memCopyFromSurface(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream)
{
	dim3 blockSize(16, 16, 1);
	dim3 gridSize(divUp(width, blockSize.x), divUp(height, blockSize.y), 1);
	fromSurface<T> <<<gridSize, blockSize, 0, stream>>> (dst, width, height, src);

	CHECK_CUDA_ERROR_AND_RETURN_STATUS(cudaDeviceSynchronize());
}

// instantiate the template for the types we need, so the compiler can generate the code.
// must ensure that the template definitions are visible to any translation unit that instantiates those templates
// (i.e. they must be in a header file, or included in the translation unit before the instantiation but since this is
// cuda code, we can't include the .cu or .cuh file in the .h file)
template cudaError_t memCopyFromSurface<float4>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);
template cudaError_t memCopyFromSurface<float2>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);
template cudaError_t memCopyFromSurface<float>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);
template cudaError_t memCopyFromSurface<uchar4>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

cudaError_t
memCopyRGBA8UToBGRA8USurface(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream)
{
	dim3 blockSize(16, 16, 1);
	dim3 gridSize(divUp(width, blockSize.x), divUp(height, blockSize.y), 1);
	toSurfaceBRGA8UFromRGBA8U <<<gridSize, blockSize, 0, stream>>> (dst, width, height, src);

	CHECK_CUDA_ERROR_AND_RETURN_STATUS(cudaDeviceSynchronize());
}

template<typename T> cudaError_t
memCopyToSurface(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream)
{
	dim3 blockSize(16, 16, 1);
	dim3 gridSize(divUp(width, blockSize.x), divUp(height, blockSize.y), 1);
	toSurface<T> <<<gridSize, blockSize, 0, stream>>> (dst, width, height, src);

	CHECK_CUDA_ERROR_AND_RETURN_STATUS(cudaDeviceSynchronize());
}

// instantiate the template for the types we need, so the compiler can generate the code
template cudaError_t memCopyToSurface<float4>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);
template cudaError_t memCopyToSurface<float2>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);
template cudaError_t memCopyToSurface<float>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);
template cudaError_t memCopyToSurface<uchar4>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);