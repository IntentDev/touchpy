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
    CompType* baseDst = static_cast<CompType*>(dst);
    size_t idx = x + (height - y - 1) * width;

    // Cache component offsets to minimize calculations
    CompType* rDst = baseDst + R * stride;
    CompType* gDst = baseDst + G * stride;
    CompType* bDst = baseDst + B * stride;
    CompType* aDst = baseDst + A * stride;

    rDst[idx] = color.x;
    gDst[idx] = color.y;
    bDst[idx] = color.z;
    aDst[idx] = color.w;
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
    CompType* baseDst = static_cast<CompType*>(dst);
    size_t idx = x + (height - y - 1) * width;

    // Cache component offsets to minimize calculations
    CompType* rDst = baseDst + R * stride;
    CompType* gDst = baseDst + G * stride;
    CompType* bDst = baseDst + B * stride;

    rDst[idx] = color.x;
    gDst[idx] = color.y;
    bDst[idx] = color.z;
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
    CompType* baseDst = static_cast<CompType*>(dst);
    size_t idx = x + (height - y - 1) * width;

    // Cache component offsets to minimize calculations
    CompType* rDst = baseDst + R * stride;
    CompType* gDst = baseDst + G * stride;

    rDst[idx] = color.x;
    gDst[idx] = color.y;
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
toSurfaceFromPlanar(cudaSurfaceObject_t dst, int width, int height, const void* src) {
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	size_t stride = width * height;
	const CompType* baseSrc = static_cast<const CompType*>(src);
	size_t idx = x + (height - y - 1) * width;

	// Cache component offsets to minimize calculations
	const CompType* rSrc = baseSrc + R * stride;
	const CompType* gSrc = baseSrc + G * stride;
	const CompType* bSrc = baseSrc + B * stride;
	const CompType* aSrc = baseSrc + A * stride;

	ColType color;
	color.x = rSrc[idx];
	color.y = gSrc[idx];
	color.z = bSrc[idx];
	color.w = aSrc[idx];

	surf2Dwrite(color, dst, x * sizeof(ColType), y);
}

// BGR -> BGRA (uint8_t)
// RGB -> BGRA (uint8_t)
// RGB -> RGBA (float32)
// BGR -> RGBA (float32)
template<typename ColType, typename CompType, int R, int G, int B> __global__ void
toSurfaceFromPlanar(cudaSurfaceObject_t dst, int width, int height, const void* src) {
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	size_t stride = width * height;
	const CompType* baseSrc = static_cast<const CompType*>(src);
	size_t idx = x + (height - y - 1) * width;

	// Cache component offsets to minimize calculations
	const CompType* rSrc = baseSrc + R * stride;
	const CompType* gSrc = baseSrc + G * stride;
	const CompType* bSrc = baseSrc + B * stride;

	ColType color;
	color.x = rSrc[idx];
	color.y = gSrc[idx];
	color.z = bSrc[idx];
    color.w = 0;

	surf2Dwrite(color, dst, x * sizeof(ColType), y);
}

// RG -> RG (uint8_t, float32)
template<typename ColType, typename CompType, int R, int G> __global__ void
toSurfaceFromPlanar(cudaSurfaceObject_t dst, int width, int height, const void* src) {
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	size_t stride = width * height;
	const CompType* baseSrc = static_cast<const CompType*>(src);
	size_t idx = x + (height - y - 1) * width;

	// Cache component offsets to minimize calculations
	const CompType* rSrc = baseSrc + R * stride;
	const CompType* gSrc = baseSrc + G * stride;

	ColType color;
	color.x = rSrc[idx];
	color.y = gSrc[idx];

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


//template<typename SrcFormat, typename DstFormat, typename Type>
//class ColorConverter
//{
//public:
//	__device__ static void convert(Type* dst, const Type* src)
//	{
//		// Default implementation
//		*dst = *src;
//	}
//};
//
//
//// Efficient conversion from RGBA to BGRA... 
//template<>
//class ColorConverter<RGBA, BGRA, UChar4>
//{
//public:
//	__device__ static void convert(const uint32_t* src, uint32_t* dst)
//	{
//		*dst = ((*src & 0xFF00FF00) | ((*src & 0x00FF0000) >> 16) | ((*src & 0x000000FF) << 16));
//	}
//}



