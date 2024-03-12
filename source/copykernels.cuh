#pragma once

#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <stdint.h>
#include <cstdio>

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
fromSurfaceBRGA8UToRGBA8U(void* dst, int width, int height, cudaSurfaceObject_t src)
{
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	uchar4 color;
	size_t size = sizeof(uchar4);
	surf2Dread(&color, src, x * size, y, cudaBoundaryModeZero);

	uint8_t* dstPtr = (uint8_t*)dst + x * size + y * width * size;
	dstPtr[0] = color.z;
	dstPtr[1] = color.y;
	dstPtr[2] = color.x;
	dstPtr[3] = color.w;
}

__global__ void
fromSurfaceBRGA8UToRGB8U(void* dst, int width, int height, cudaSurfaceObject_t src)
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

// RGBAFS32, RGFS32, RFS32, RGBAFS16, RGFS16, RFS16, RGU8, RU8
template<typename T> __global__ void
fromSurface(void* dst, int width, int height, cudaSurfaceObject_t src)
{
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	T color;
	size_t size = sizeof(T);
	surf2Dread(&color, src, x * size, y, cudaBoundaryModeZero);
	T* dstPtr = (T*)((T*)dst + y * width);
	dstPtr[x] = color;
}

__global__ void
toSurfaceBRGA8UFromRGBA8U(cudaSurfaceObject_t dst, int width, int height, const void* src)
{
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	size_t size = sizeof(uchar4);
	uint8_t* srcPtr = (uint8_t*)src + x * size + y * width * size;
	uchar4 color;
	color.z = srcPtr[0];
	color.y = srcPtr[1];
	color.x = srcPtr[2];
	color.w = srcPtr[3];
	surf2Dwrite(color, dst, x * size, y, cudaBoundaryModeZero);
}

__global__ void
toSurfaceBRGA8UFromRGB8U(cudaSurfaceObject_t dst, int width, int height, const void* src)
{
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	size_t size = sizeof(uchar3);
	uint8_t* srcPtr = (uint8_t*)src + x * size + y * width * size;
	uchar4 color;
	color.z = srcPtr[0];
	color.y = srcPtr[1];
	color.x = srcPtr[2];
	color.w = 255;
	surf2Dwrite(color, dst, x * sizeof(uchar4), y, cudaBoundaryModeZero);
}

// RGBAFS32, RGFS32, RFS32, RGBAFS16, RGFS16, RFS16, RGU8, RU8
template<typename T> __global__ void
toSurface(cudaSurfaceObject_t dst, int width, int height, const void* src)
{
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	size_t size = sizeof(T);  
	T color = *(T*)((T*)src + x + y * width);
	surf2Dwrite(color, dst, x * size, y, cudaBoundaryModeZero);
}

template<typename DstT, typename SrcT> __global__ void
toSurface(cudaSurfaceObject_t dst, int width, int height, const void* src)
{
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	SrcT srcColor = *(SrcT*)((SrcT*)src + x + y * width);
	DstT color;
	color.x = srcColor.x;
	color.y = srcColor.y;
	color.z = srcColor.z;
	color.w = 255;
	surf2Dwrite(color, dst, x * sizeof(DstT), y, cudaBoundaryModeZero);
}