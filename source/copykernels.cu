#include "copykernels.cuh"

//----------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------
template<typename ColType, typename CompType, int R, int G, int B, int A> cudaError_t
memCopySurfaceToPlanar(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream)
{
	dim3 blockSize(16, 16, 1);
	dim3 gridSize(divUp(width, blockSize.x), divUp(height, blockSize.y), 1);
	fromSurfaceToPlanar<ColType, CompType, R, G, B, A> <<<gridSize, blockSize, 0, stream>>> (dst, width, height, src);

	CHECK_CUDA_ERROR_AND_RETURN_STATUS(cudaDeviceSynchronize());
}

template cudaError_t // BGRA -> BGRA (uint8_t)
memCopySurfaceToPlanar<uchar4, uint8_t, 0, 1, 2, 3>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // BGRA -> RGBA (uint8_t)
memCopySurfaceToPlanar<uchar4, uint8_t, 2, 1, 0, 3>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RGBA -> RGBA (short)
memCopySurfaceToPlanar<ushort4, short, 0, 1, 2, 3>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RGBA -> BGRA (short)
memCopySurfaceToPlanar<ushort4, short, 2, 1, 0, 3>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RGBA -> RGBA (float32)
memCopySurfaceToPlanar<float4, float, 0, 1, 2, 3>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RGBA -> BGRA (float32)
memCopySurfaceToPlanar<float4, float, 2, 1, 0, 3>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RGBA -> RGBA (float16)
memCopySurfaceToPlanar<Half4, half, 0, 1, 2, 3>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RGBA -> BGRA (float16)
memCopySurfaceToPlanar<Half4, half, 2, 1, 0, 3>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

//----------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------
template<typename ColType, typename CompType, int R, int G, int B> cudaError_t
memCopySurfaceToPlanar(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream)
{
	dim3 blockSize(16, 16, 1);
	dim3 gridSize(divUp(width, blockSize.x), divUp(height, blockSize.y), 1);
	fromSurfaceToPlanar<ColType, CompType, R, G, B> <<<gridSize, blockSize, 0, stream>>> (dst, width, height, src);

	CHECK_CUDA_ERROR_AND_RETURN_STATUS(cudaDeviceSynchronize());
}

template cudaError_t // BGRA -> BGR (uint8_t)
memCopySurfaceToPlanar<uchar4, uint8_t, 0, 1, 2>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // BGRA -> RGB (uint8_t)
memCopySurfaceToPlanar<uchar4, uint8_t, 2, 1, 0>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RGBA -> RGB (short)
memCopySurfaceToPlanar<ushort4, short, 0, 1, 2>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RGBA -> BGR (short)
memCopySurfaceToPlanar<ushort4, short, 2, 1, 0>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RGBA -> RGB (float32)
memCopySurfaceToPlanar<float4, float, 0, 1, 2>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RGBA -> BGR (float32)
memCopySurfaceToPlanar<float4, float, 2, 1, 0>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RGBA -> RGB (float16)
memCopySurfaceToPlanar<Half4, half, 0, 1, 2>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RGBA -> BGR (float16)
memCopySurfaceToPlanar<Half4, half, 2, 1, 0>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

//----------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------
template<typename ColType, typename CompType, int R, int G> cudaError_t
memCopySurfaceToPlanar(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream)
{
	dim3 blockSize(16, 16, 1);
	dim3 gridSize(divUp(width, blockSize.x), divUp(height, blockSize.y), 1);
	fromSurfaceToPlanar<ColType, CompType, R, G> <<<gridSize, blockSize, 0, stream>>> (dst, width, height, src);

	CHECK_CUDA_ERROR_AND_RETURN_STATUS(cudaDeviceSynchronize());
}

template cudaError_t // RG -> RG (uint8_t)
memCopySurfaceToPlanar<uchar2, uint8_t, 0, 1>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RG -> RG (short)
memCopySurfaceToPlanar<ushort2, short, 0, 1>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RG -> RG (float32)
memCopySurfaceToPlanar<float2, float, 0, 1>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RG -> RG (float16)
memCopySurfaceToPlanar<half2, half, 0, 1>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);


//----------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------
template<typename ColType, typename CompType, int R, int G, int B, int A> cudaError_t
memCopySurface(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream)
{
	dim3 blockSize(16, 16, 1);
	dim3 gridSize(divUp(width, blockSize.x), divUp(height, blockSize.y), 1);
	fromSurface<ColType, CompType, R, G, B, A> <<<gridSize, blockSize, 0, stream>>> (dst, width, height, src);

	CHECK_CUDA_ERROR_AND_RETURN_STATUS(cudaDeviceSynchronize());
}

template cudaError_t // BGRA -> RGBA (uint8_t)
memCopySurface<uchar4, uint8_t, 2, 1, 0, 3>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RGBA -> BGRA (short)
memCopySurface<ushort4, short, 2, 1, 0, 3>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RGBA -> BGRA (float32)
memCopySurface<float4, float, 2, 1, 0, 3>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RGBA -> BGRA (float16)
memCopySurface<Half4, half, 2, 1, 0, 3>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);


//----------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------
template<typename ColType, typename CompType, int R, int G, int B> cudaError_t
memCopySurface(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream)
{
	dim3 blockSize(16, 16, 1);
	dim3 gridSize(divUp(width, blockSize.x), divUp(height, blockSize.y), 1);
	fromSurface<ColType, CompType, R, G, B> <<<gridSize, blockSize, 0, stream>>> (dst, width, height, src);

	CHECK_CUDA_ERROR_AND_RETURN_STATUS(cudaDeviceSynchronize());
}

template cudaError_t // BGRA -> BGR (uint8_t)
memCopySurface<uchar4, uint8_t, 0, 1, 2>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // BGRA -> RGB (uint8_t)
memCopySurface<uchar4, uint8_t, 2, 1, 0>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // BGRA -> BGR (short)
memCopySurface<ushort4, short, 0, 1, 2>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // BGRA -> RGB (short)
memCopySurface<ushort4, short, 2, 1, 0>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RGBA -> RGB (float32)
memCopySurface<float4, float, 0, 1, 2>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RGBA -> BGR (float32)
memCopySurface<float4, float, 2, 1, 0>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RGBA -> RGB (float16)
memCopySurface<Half4, half, 0, 1, 2>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RGBA -> BGR (float16)
memCopySurface<Half4, half, 2, 1, 0>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);


//----------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------
template<typename T> cudaError_t
memCopySurface(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream)
{
	dim3 blockSize(16, 16, 1);
	dim3 gridSize(divUp(width, blockSize.x), divUp(height, blockSize.y), 1);
	fromSurface<T> <<<gridSize, blockSize, 0, stream>>> (dst, width, height, src);

	CHECK_CUDA_ERROR_AND_RETURN_STATUS(cudaDeviceSynchronize());
}

template cudaError_t // BGRA -> BGRA (uint8_t) (to interleaved)
memCopySurface<uchar4>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RG -> RG (uint8_t) (to interleaved)
memCopySurface<uchar2>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // R -> R (uint8_t)
memCopySurface<uint8_t>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RGBA -> RGBA (short) (to interleaved)
memCopySurface<ushort4>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RG -> RG (short) (to interleaved)
memCopySurface<ushort2>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // R -> R (short)
memCopySurface<short>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RGBA -> RGBA (float32) (to interleaved)
memCopySurface<float4>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RG -> RG (float32) (to interleaved)
memCopySurface<float2>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // R -> R (float32)
memCopySurface<float>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RGBA -> RGBA (float16) (to interleaved)
memCopySurface<Half4>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // RG -> RG (float16) (to interleaved)
memCopySurface<half2>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);

template cudaError_t // R -> R (float16)
memCopySurface<half>(void* dst, int width, int height, cudaSurfaceObject_t src, cudaStream_t stream);



//----------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------
template<typename ColType, typename CompType, int R, int G, int B, int A> cudaError_t
memCopyPlanarToSurface(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream)
{
	dim3 blockSize(16, 16, 1);
	dim3 gridSize(divUp(width, blockSize.x), divUp(height, blockSize.y), 1);
	planarToSurface<ColType, CompType, R, G, B, A> <<<gridSize, blockSize, 0, stream>>> (dst, width, height, src);

	CHECK_CUDA_ERROR_AND_RETURN_STATUS(cudaDeviceSynchronize());
}

template cudaError_t // BGRA -> BGRA (uint8_t)
memCopyPlanarToSurface<uchar4, uint8_t, 0, 1, 2, 3>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // RGBA -> BGRA (uint8_t)
memCopyPlanarToSurface<uchar4, uint8_t, 2, 1, 0, 3>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // RGBA -> RGBA (short)
memCopyPlanarToSurface<ushort4, short, 0, 1, 2, 3>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // BGRA -> RGBA (short)
memCopyPlanarToSurface<ushort4, short, 2, 1, 0, 3>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // RGBA -> RGBA (float32)
memCopyPlanarToSurface<float4, float, 0, 1, 2, 3>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // BGRA -> RGBA (float32)
memCopyPlanarToSurface<float4, float, 2, 1, 0, 3>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // RGBA -> RGBA (float16)
memCopyPlanarToSurface<Half4, half, 0, 1, 2, 3>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // BGRA -> RGBA (float16)
memCopyPlanarToSurface<Half4, half, 2, 1, 0, 3>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);


//----------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------
template<typename ColType, typename CompType, int R, int G, int B> cudaError_t
memCopyPlanarToSurface(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream)
{
	dim3 blockSize(16, 16, 1);
	dim3 gridSize(divUp(width, blockSize.x), divUp(height, blockSize.y), 1);
	planarToSurface<ColType, CompType, R, G, B> <<<gridSize, blockSize, 0, stream>>> (dst, width, height, src);

	CHECK_CUDA_ERROR_AND_RETURN_STATUS(cudaDeviceSynchronize());
}

template cudaError_t // BGR -> BGRA (uint8_t)
memCopyPlanarToSurface<uchar4, uint8_t, 0, 1, 2>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // RGB -> BGRA (uint8_t)
memCopyPlanarToSurface<uchar4, uint8_t, 2, 1, 0>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // RGB -> RGBA (short)
memCopyPlanarToSurface<ushort4, short, 0, 1, 2>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // BGR -> RGBA (short)
memCopyPlanarToSurface<ushort4, short, 2, 1, 0>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // RGB -> RGBA (float32)
memCopyPlanarToSurface<float4, float, 0, 1, 2>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // BGR -> RGBA (float32)
memCopyPlanarToSurface<float4, float, 2, 1, 0>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // RGB -> RGBA (float16)
memCopyPlanarToSurface<Half4, half, 0, 1, 2>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // BGR -> RGBA (float16)
memCopyPlanarToSurface<Half4, half, 2, 1, 0>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);


//----------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------
template<typename ColType, typename CompType, int R, int G> cudaError_t
memCopyPlanarToSurface(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream)
{
	dim3 blockSize(16, 16, 1);
	dim3 gridSize(divUp(width, blockSize.x), divUp(height, blockSize.y), 1);
	planarToSurface<ColType, CompType, R, G> <<<gridSize, blockSize, 0, stream>>> (dst, width, height, src);

	CHECK_CUDA_ERROR_AND_RETURN_STATUS(cudaDeviceSynchronize());
}

template cudaError_t // RG -> RG (uint8_t)
memCopyPlanarToSurface<uchar2, uint8_t, 0, 1>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // RG -> RG (short)
memCopyPlanarToSurface<ushort2, short, 0, 1>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // RG -> RG (float32)
memCopyPlanarToSurface<float2, float, 0, 1>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // RG -> RG (float16)
memCopyPlanarToSurface<half2, half, 0, 1>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);


//----------------------------------------------------------------------------------------------------------------------------------
template<typename ColType, typename CompType, int R, int G, int B, int A> cudaError_t
memCopyToSurface(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream)
{
	dim3 blockSize(16, 16, 1);
	dim3 gridSize(divUp(width, blockSize.x), divUp(height, blockSize.y), 1);
	toSurface<ColType, CompType, R, G, B, A> <<<gridSize, blockSize, 0, stream>>> (dst, width, height, src);

	CHECK_CUDA_ERROR_AND_RETURN_STATUS(cudaDeviceSynchronize());
}

template cudaError_t // RGBA -> BGRA (uint8_t)
memCopyToSurface<uchar4, uint8_t, 2, 1, 0, 3>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // BGRA -> RGBA (short)
memCopyToSurface<ushort4, short, 2, 1, 0, 3>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // BGRA -> RGBA (float32)
memCopyToSurface<float4, float, 2, 1, 0, 3>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // BGRA -> RGBA (float16)
memCopyToSurface<Half4, half, 2, 1, 0, 3>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);


//----------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------
template<typename ColType, typename CompType, int R, int G, int B> cudaError_t
memCopyToSurface(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream)
{
	dim3 blockSize(16, 16, 1);
	dim3 gridSize(divUp(width, blockSize.x), divUp(height, blockSize.y), 1);
	toSurface<ColType, CompType, R, G, B> <<<gridSize, blockSize, 0, stream>>> (dst, width, height, src);

	CHECK_CUDA_ERROR_AND_RETURN_STATUS(cudaDeviceSynchronize());
}

template cudaError_t // BGR -> BGRA (uint8_t)
memCopyToSurface<uchar4, uint8_t, 0, 1, 2>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // RGB -> BGRA (uint8_t)
memCopyToSurface<uchar4, uint8_t, 2, 1, 0>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // RGB -> RGBA (short)
memCopyToSurface<ushort4, short, 0, 1, 2>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // BGR -> RGBA (short)
memCopyToSurface<ushort4, short, 2, 1, 0>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // RGB -> RGBA (float32)
memCopyToSurface<float4, float, 0, 1, 2>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // BGR -> RGBA (float32)
memCopyToSurface<float4, float, 2, 1, 0>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // RGB -> RGBA (float16)
memCopyToSurface<Half4, half, 0, 1, 2>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // BGR -> RGBA (float16)
memCopyToSurface<Half4, half, 2, 1, 0>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);


//----------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------
template<typename T> cudaError_t
memCopyToSurface(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream)
{
	dim3 blockSize(16, 16, 1);
	dim3 gridSize(divUp(width, blockSize.x), divUp(height, blockSize.y), 1);
	toSurface<T> <<<gridSize, blockSize, 0, stream>>> (dst, width, height, src);

	CHECK_CUDA_ERROR_AND_RETURN_STATUS(cudaDeviceSynchronize());
}

template cudaError_t // BGRA -> BGRA (uint8_t) (from interleaved)
memCopyToSurface<uchar4>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // RG -> RG (uint8_t) (from interleaved)
memCopyToSurface<uchar2>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // R -> R (uint8_t)
memCopyToSurface<uint8_t>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // RGBA -> RGBA (short) (from interleaved)
memCopyToSurface<ushort4>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // RG -> RG (short) (from interleaved)
memCopyToSurface<ushort2>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // R -> R (short)
memCopyToSurface<short>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // RGBA -> RGBA (float32) (from interleaved)
memCopyToSurface<float4>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // RG -> RG (float32) (from interleaved)
memCopyToSurface<float2>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // R -> R (float32)
memCopyToSurface<float>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // RGBA -> RGBA (float16) (from interleaved)
memCopyToSurface<Half4>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // RG -> RG (float16) (from interleaved)
memCopyToSurface<half2>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

template cudaError_t // R -> R (float16)
memCopyToSurface<half>(cudaSurfaceObject_t dst, int width, int height, const void* src, cudaStream_t stream);

