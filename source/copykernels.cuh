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


// BGRA -> BGRA (uint8_t)
// BGRA -> RGBA (uint8_t)
// RGBA -> RGBA (float32)
// RGBA -> BGRA (float32)
template<typename ColType, typename CompType, int R, int G, int B, int A> __global__ void 
fromSurfaceToPlanar(void* dst, int width, int height, cudaSurfaceObject_t src) {
    unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= width || y >= height)
        return;

    ColType color;
    surf2Dread(&color, src, x * sizeof(ColType), y, cudaBoundaryModeZero);

    size_t stride = width * height;
    CompType* dstPtr = static_cast<CompType*>(dst);
    size_t idx = x + (height - y - 1) * width;

    dstPtr[idx + R * stride] = color.x;
    dstPtr[idx + G * stride] = color.y;
    dstPtr[idx + B * stride] = color.z;
    dstPtr[idx + A * stride] = color.w;
}

// BGRA -> BGR (uint8_t)
// BGRA -> RGB (uint8_t)
// RGBA -> RGB (float32)
// RGBA -> BGR (float32)
template<typename ColType, typename CompType, int R, int G, int B> __global__ void 
fromSurfaceToPlanar(void* dst, int width, int height, cudaSurfaceObject_t src) {
    unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= width || y >= height)
        return;

    ColType color;
    surf2Dread(&color, src, x * sizeof(ColType), y, cudaBoundaryModeZero);

    size_t stride = width * height;
    CompType* dstPtr = static_cast<CompType*>(dst);
    size_t idx = x + (height - y - 1) * width;

    dstPtr[idx + R * stride] = color.x;
    dstPtr[idx + G * stride] = color.y;
    dstPtr[idx + B * stride] = color.z;
}

// RG -> RG (uint8_t, float32)
template<typename ColType, typename CompType, int R, int G> __global__ void 
fromSurfaceToPlanar(void* dst, int width, int height, cudaSurfaceObject_t src) {
    unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= width || y >= height)
        return;

    ColType color;
    surf2Dread(&color, src, x * sizeof(ColType), y, cudaBoundaryModeZero);

    size_t stride = width * height;
    CompType* dstPtr = static_cast<CompType*>(dst);
    size_t idx = x + (height - y - 1) * width;

    dstPtr[idx + R * stride] = color.x;
    dstPtr[idx + G * stride] = color.y;
}

// BGRA -> RGBA (uint8_t) to interleaved
// RGBA -> BGRA (float32) to interleaved
template<typename ColType, typename CompType, int R, int G, int B, int A> __global__ void
fromSurface(void* dst, int width, int height, cudaSurfaceObject_t src) {
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	ColType color;
	surf2Dread(&color, src, x * sizeof(ColType), y, cudaBoundaryModeZero);
	CompType* dstPtr = static_cast<CompType*>(dst) + (x + (height - y - 1) * width) * 4;

	dstPtr[R] = color.x;
	dstPtr[G] = color.y;
	dstPtr[B] = color.z;
	dstPtr[A] = color.w;
}

// BGRA -> BGR (uint8_t) to interleaved
// BGRA -> RGB (uint8_t) to interleaved
// RGBA -> RGB (float32) to interleaved
// RGBA -> BGR (float32) to interleaved
template<typename ColType, typename CompType, int R, int G, int B> __global__ void
fromSurface(void* dst, int width, int height, cudaSurfaceObject_t src) {
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	ColType color;
	surf2Dread(&color, src, x * sizeof(ColType), y, cudaBoundaryModeZero);
	CompType* dstPtr = static_cast<CompType*>(dst) + (x + (height - y - 1) * width) * 3;

	dstPtr[R] = color.x;
	dstPtr[G] = color.y;
	dstPtr[B] = color.z;
}


// BGRA -> BRGA interleaved
// RGBA -> RGBA interleaved
// RG -> RG interleaved
// R -> R (uint8_t, float32) Planar/Interleaved
template<typename T> __global__ void
fromSurface(void* dst, int width, int height, cudaSurfaceObject_t src)
{
    unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= width || y >= height)
        return;

    T color;
    surf2Dread(&color, src, x * sizeof(T), y, cudaBoundaryModeZero);
    T* dstPtr = (T*)((T*)dst + (height - y - 1) * width);
    dstPtr[x] = color;
}


// BGRA -> BGRA (uint8_t)
// RGBA -> BGRA (uint8_t)
// RGBA -> RGBA (float32)
// BGRA -> RGBA (float32)
template<typename ColType, typename CompType, int R, int G, int B, int A> __global__ void
planarToSurface(cudaSurfaceObject_t dst, int width, int height, const void* src) {
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	size_t stride = width * height;
	const CompType* srcPtr = static_cast<const CompType*>(src);
	size_t idx = x + (height - y - 1) * width;

    ColType color;
	color.x = srcPtr[idx + R * stride];
	color.y = srcPtr[idx + G * stride];
	color.z = srcPtr[idx + B * stride];
	color.w = srcPtr[idx + A * stride];

	surf2Dwrite(color, dst, x * sizeof(ColType), y);
}

// BGR -> BGRA (uint8_t)
// RGB -> BGRA (uint8_t)
// RGB -> RGBA (float32)
// BGR -> RGBA (float32)
template<typename ColType, typename CompType, int R, int G, int B> __global__ void
planarToSurface(cudaSurfaceObject_t dst, int width, int height, const void* src) {
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	size_t stride = width * height;
	const CompType* srcPtr = static_cast<const CompType*>(src);
	size_t idx = x + (height - y - 1) * width;

    ColType color;
    color.x = srcPtr[idx + R * stride];
    color.y = srcPtr[idx + G * stride];
    color.z = srcPtr[idx + B * stride];
    color.w = 0;

	surf2Dwrite(color, dst, x * sizeof(ColType), y);
}

// RG -> RG (uint8_t, float32)
template<typename ColType, typename CompType, int R, int G> __global__ void
planarToSurface(cudaSurfaceObject_t dst, int width, int height, const void* src) {
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	size_t stride = width * height;
	const CompType* srcPtr = static_cast<const CompType*>(src);
	size_t idx = x + (height - y - 1) * width;

    ColType color;
    color.x = srcPtr[idx + R * stride];
    color.y = srcPtr[idx + G * stride];

	surf2Dwrite(color, dst, x * sizeof(ColType), y);
}

// RGBA -> BGRA (uint8_t) from interleaved
// BGRA -> RGBA (float32) from interleaved
template<typename ColType, typename CompType, int R, int G, int B, int A> __global__ void
toSurface(cudaSurfaceObject_t dst, int width, int height, const void* src) {
    unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= width || y >= height)
        return;

    const CompType* srcPtr = static_cast<const CompType*>(src);
    size_t idx = (x + (height - y - 1) * width) * 4;

    ColType color;
    color.x = srcPtr[idx + R];
    color.y = srcPtr[idx + G];
    color.z = srcPtr[idx + B];
    color.w = srcPtr[idx + A];

    surf2Dwrite(color, dst, x * sizeof(ColType), y);
}

// BGR -> BGRA (uint8_t) from interleaved
// RGB -> BGRA (uint8_t) from interleaved
// RGB -> RGBA (float32) from interleaved
// BGR -> RGBA (float32) from interleaved
template<typename ColType, typename CompType, int R, int G, int B> __global__ void
toSurface(cudaSurfaceObject_t dst, int width, int height, const void* src) {
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	const CompType* srcPtr = static_cast<const CompType*>(src);
	size_t idx = (x + (height - y - 1) * width) * 3;

    ColType color;
    color.x = srcPtr[idx + R];
    color.y = srcPtr[idx + G];
    color.z = srcPtr[idx + B];
    color.w = 0;

	surf2Dwrite(color, dst, x * sizeof(ColType), y);
}

// BGRA -> BRGA from interleaved
// RGBA -> RGBA from interleaved
// RG -> RG from interleaved
// R -> R (uint8_t, float32) from Planar/Interleaved
template<typename T> __global__ void
toSurface(cudaSurfaceObject_t dst, int width, int height, const void* src)
{
    unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= width || y >= height)
        return;

    T color = *(T*)((T*)src + x + (height - y - 1) * width);
    surf2Dwrite(color, dst, x * sizeof(T), y, cudaBoundaryModeZero);
}

