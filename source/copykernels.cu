
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <algorithm>
#include <stdio.h>



#ifdef _DEBUG
#define CHECK_CUDA_ERROR_AND_RETURN_STATUS(call) do { \
    cudaError_t cudaStatus = (call); \
    if (cudaStatus != cudaSuccess) { \
        fprintf(stderr, "CUDA Error at %s:%d: %s\n", __FILE__, __LINE__, cudaGetErrorString(cudaStatus)); \
        return cudaStatus; \
    } \
} while(0)
#else
#define CHECK_CUDA_ERROR_AND_RETURN_STATUS(call) return cudaSuccess;
#endif


inline int divUp(int a, int b)
{
	return ((a % b) != 0) ? (a / b + 1) : (a / b);
}

__global__ void
copyFromSurfaceCharBRGA(unsigned char* dst, int width, int height, cudaSurfaceObject_t input)
{
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	uchar4 color;
	surf2Dread(&color, input, x * 4, y, cudaBoundaryModeZero);

	uchar4* dstPtr = (uchar4*)(dst + y * width * sizeof(uchar4));
	dstPtr[x] = color;
}

cudaError_t
memCopyFromSurfaceCharBRGA(
	unsigned char* dst,
	int width,
	int height,
	cudaSurfaceObject_t input,
	cudaStream_t stream)
{
	dim3 blockSize(16, 16, 1);
	dim3 gridSize(divUp(width, blockSize.x), divUp(height, blockSize.y), 1);
	copyFromSurfaceCharBRGA << < gridSize, blockSize, 0, stream >> > (dst, width, height, input);

	CHECK_CUDA_ERROR_AND_RETURN_STATUS(cudaDeviceSynchronize());
}
