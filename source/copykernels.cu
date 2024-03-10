
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <algorithm>



#ifdef _DEBUG
#define CHECK_CUDA_ERROR_AND_RETURN_STATUS(call) do { \
    cudaError_t cudaStatus = (call); \
    if (cudaStatus != cudaSuccess) { \
        fprintf(stderr, "CUDA Error at %s:%d: %s\n", __FILE__, __LINE__, cudaGetErrorString(cudaStatus)); \
    } \
    return cudaStatus; \
} while(0)
#else
#define CHECK_CUDA_ERROR_AND_RETURN_STATUS(call) return cudaSuccess;
#endif


inline int divUp(int a, int b)
{
	return ((a % b) != 0) ? (a / b + 1) : (a / b);
}

__global__ void
copyBRGA8USurfaceToRGBA8U(void* dst, int width, int height, cudaSurfaceObject_t src)
{
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	uchar4 color;
	size_t size = sizeof(uchar4);
	surf2Dread(&color, src, x * size, y, cudaBoundaryModeZero);

	//uchar4* dstPtr = (uchar4*)((uint8_t*)dst + y * width * size);
	//dstPtr[x] = color;
	uint8_t* dstPtr = (uint8_t*)dst + x * size + y * width * size;
	dstPtr[0] = color.z;
	dstPtr[1] = color.y;
	dstPtr[2] = color.x;
	dstPtr[3] = color.w;

}

cudaError_t
memCopyBRGA8USurfaceToRGBA8U(
	void* dst,
	int width,
	int height,
	cudaSurfaceObject_t src,
	cudaStream_t stream)
{
	dim3 blockSize(16, 16, 1);
	dim3 gridSize(divUp(width, blockSize.x), divUp(height, blockSize.y), 1);
	copyBRGA8USurfaceToRGBA8U <<<gridSize, blockSize, 0, stream>>> (dst, width, height, src);

	CHECK_CUDA_ERROR_AND_RETURN_STATUS(cudaDeviceSynchronize());
}

__global__ void
copyBRG8USurfaceToRGB8U(void* dst, int width, int height, cudaSurfaceObject_t src)
{
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	uchar4 color;
	surf2Dread(&color, src, x * sizeof(uchar4), y, cudaBoundaryModeZero);

	size_t size = sizeof(uchar3);
	uint8_t* dstPtr = (uint8_t*)dst + x * size + y * width * size;
	dstPtr[0] = color.z;
	dstPtr[1] = color.y;
	dstPtr[2] = color.x;

}

cudaError_t
memCopyBRG8USurfaceToRGB8U(
	void* dst,
	int width,
	int height,
	cudaSurfaceObject_t src,
	cudaStream_t stream)
{
	dim3 blockSize(16, 16, 1);
	dim3 gridSize(divUp(width, blockSize.x), divUp(height, blockSize.y), 1);
	copyBRG8USurfaceToRGB8U << <gridSize, blockSize, 0, stream >> > (dst, width, height, src);

	CHECK_CUDA_ERROR_AND_RETURN_STATUS(cudaDeviceSynchronize());
}


__global__ void
copyRGBA8UToBGRA8USurface(cudaSurfaceObject_t dst, int width, int height, const void* src)
{
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	size_t size = sizeof(uchar4);

	//uchar4 color = *(uchar4*)((uint8_t*)src + x * size + y * width * size);
	//surf2Dwrite(color, dst, x * size, y, cudaBoundaryModeZero);

	uint8_t* srcPtr = (uint8_t*)src + x * size + y * width * size;
	uchar4 color;
	color.z = srcPtr[0];
	color.y = srcPtr[1];
	color.x = srcPtr[2];
	color.w = srcPtr[3];
	surf2Dwrite(color, dst, x * size, y, cudaBoundaryModeZero);
}

cudaError_t
memCopyRGBA8UToBGRA8USurface(
	cudaSurfaceObject_t dst,
	int width,
	int height,
	const void* src,
	cudaStream_t stream)
{
	dim3 blockSize(16, 16, 1);
	dim3 gridSize(divUp(width, blockSize.x), divUp(height, blockSize.y), 1);
	copyRGBA8UToBGRA8USurface <<<gridSize, blockSize, 0, stream>>> (dst, width, height, src);

	CHECK_CUDA_ERROR_AND_RETURN_STATUS(cudaDeviceSynchronize());
}