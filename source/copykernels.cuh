#pragma once

#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <stdint.h>
#include <cstdio>

#include <cuda_fp16.h>
#include "cudadatatypes.h"

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
// RGBA -> RGBA (uint16_t)
// RGBA -> BGRA (uint16_t)
// RGBA -> RGBA (float32)
// RGBA -> BGRA (float32)
template<typename ColType, typename CompType, int R, int G, int B, int A> __global__ void 
fromSurfaceToPlanar(void* dst, int width, int height, cudaSurfaceObject_t src) 
{
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

// RGBA -> RGBA (float16)
template<> __global__ void
fromSurfaceToPlanar<Half4, half, 0, 1, 2, 3>(void* dst, int width, int height, cudaSurfaceObject_t src)
{
    unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= width || y >= height)
        return;

    size_t stride = width * height;
    half* dstPtr = static_cast<half*>(dst);
    size_t idx = x + (height - y - 1) * width;

    ushort4 data;
    surf2Dread(&data, src, x * sizeof(ushort4), y, cudaBoundaryModeZero);

    half* halfPtr = reinterpret_cast<half*>(&data);

    dstPtr[idx] = halfPtr[0];
    dstPtr[idx + stride] = halfPtr[1];
    dstPtr[idx + 2 * stride] = halfPtr[2];
    dstPtr[idx + 3 * stride] = halfPtr[3];
}

// RGBA -> BGRA (float16)
template<> __global__ void
fromSurfaceToPlanar<Half4, half, 2, 1, 0, 3>(void* dst, int width, int height, cudaSurfaceObject_t src)
{
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	size_t stride = width * height;
	half* dstPtr = static_cast<half*>(dst);
	size_t idx = x + (height - y - 1) * width;

    ushort4 data;
	surf2Dread(&data, src, x * sizeof(ushort4), y, cudaBoundaryModeZero);

	half* halfPtr = reinterpret_cast<half*>(&data);

	dstPtr[idx] = halfPtr[2];
	dstPtr[idx + stride] = halfPtr[1];
	dstPtr[idx + 2 * stride] = halfPtr[0];
	dstPtr[idx + 3 * stride] = halfPtr[3];
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

// RGBA -> RGB (float16)
template<> __global__ void
fromSurfaceToPlanar<Half4, half, 0, 1, 2>(void* dst, int width, int height, cudaSurfaceObject_t src)
{
    unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= width || y >= height)
        return;

    size_t stride = width * height;
    half* dstPtr = static_cast<half*>(dst);
    size_t idx = x + (height - y - 1) * width;

    ushort4 data;
    surf2Dread(&data, src, x * sizeof(ushort4), y, cudaBoundaryModeZero);

    half* halfPtr = reinterpret_cast<half*>(&data);

    dstPtr[idx] = halfPtr[0];
    dstPtr[idx + stride] = halfPtr[1];
    dstPtr[idx + 2 * stride] = halfPtr[2];
}

// RGBA -> BGR (float16)
template<> __global__ void
fromSurfaceToPlanar<Half4, half, 2, 1, 0>(void* dst, int width, int height, cudaSurfaceObject_t src)
{
    unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= width || y >= height)
        return;

    size_t stride = width * height;
    half* dstPtr = static_cast<half*>(dst);
    size_t idx = x + (height - y - 1) * width;

    ushort4 data;
    surf2Dread(&data, src, x * sizeof(ushort4), y, cudaBoundaryModeZero);

    half* halfPtr = reinterpret_cast<half*>(&data);

    dstPtr[idx] = halfPtr[2];
    dstPtr[idx + stride] = halfPtr[1];
    dstPtr[idx + 2 * stride] = halfPtr[0];
}



// RG -> RG (uint8_t, float32, float16)
template<typename ColType, typename CompType, int R, int G> __global__ void 
fromSurfaceToPlanar(void* dst, int width, int height, cudaSurfaceObject_t src) 
{
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

// RG -> RG (float16)
template<> __global__ void
fromSurfaceToPlanar<half2, half, 0, 1>(void* dst, int width, int height, cudaSurfaceObject_t src)
{
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	size_t stride = width * height;
	half* dstPtr = static_cast<half*>(dst);
	size_t idx = x + (height - y - 1) * width;

	ushort2 data;
	surf2Dread(&data, src, x * sizeof(ushort2), y, cudaBoundaryModeZero);

	half* halfPtr = reinterpret_cast<half*>(&data);

	dstPtr[idx] = halfPtr[0];
	dstPtr[idx + stride] = halfPtr[1];
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

// RGBA -> BGRA (float16) to interleaved
template<> __global__ void
fromSurface<Half4, half, 2, 1, 0, 3>(void* dst, int width, int height, cudaSurfaceObject_t src)
{
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	ushort4 color;
	surf2Dread(&color, src, x * sizeof(ushort4), y, cudaBoundaryModeZero);
	half* dstPtr = static_cast<half*>(dst) + (x + (height - y - 1) * width) * 4;

	half* halfPtr = reinterpret_cast<half*>(&color);
	dstPtr[0] = halfPtr[2];
	dstPtr[1] = halfPtr[1];
	dstPtr[2] = halfPtr[0];
	dstPtr[3] = halfPtr[3];
}



// BGRA -> BGR (uint8_t) to interleaved
// BGRA -> RGB (uint8_t) to interleaved
// RGBA -> RGB (float32) to interleaved
// RGBA -> BGR (float32) to interleaved
template<typename ColType, typename CompType, int R, int G, int B> __global__ void
fromSurface(void* dst, int width, int height, cudaSurfaceObject_t src) 
{
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

// RGBA -> RGB (float16) to interleaved
template<> __global__ void
fromSurface<Half4, half, 0, 1, 2>(void* dst, int width, int height, cudaSurfaceObject_t src)
{
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	ushort4 color;
	surf2Dread(&color, src, x * sizeof(ushort4), y, cudaBoundaryModeZero);
	half* dstPtr = static_cast<half*>(dst) + (x + (height - y - 1) * width) * 3;

	half* halfPtr = reinterpret_cast<half*>(&color);
	dstPtr[0] = halfPtr[0];
	dstPtr[1] = halfPtr[1];
	dstPtr[2] = halfPtr[2];
}

// RGBA -> BGR (float16) to interleaved
template<> __global__ void
fromSurface<Half4, half, 2, 1, 0>(void* dst, int width, int height, cudaSurfaceObject_t src)
{
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	ushort4 color;
	surf2Dread(&color, src, x * sizeof(ushort4), y, cudaBoundaryModeZero);
	half* dstPtr = static_cast<half*>(dst) + (x + (height - y - 1) * width) * 3;

	half* halfPtr = reinterpret_cast<half*>(&color);
	dstPtr[0] = halfPtr[2];
	dstPtr[1] = halfPtr[1];
	dstPtr[2] = halfPtr[0];
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

// RGBA -> RGBA (float16) interleaved
template<> __global__ void
fromSurface<Half4>(void* dst, int width, int height, cudaSurfaceObject_t src)
{
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	ushort4 color;
	surf2Dread(&color, src, x * sizeof(ushort4), y, cudaBoundaryModeZero);
    ushort4* dstPtr = (ushort4*)((ushort4*)dst + (height - y - 1) * width);
	dstPtr[x] = color;
}

// RG -> RG (float16) interleaved
template<> __global__ void
fromSurface<half2>(void* dst, int width, int height, cudaSurfaceObject_t src)
{
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	ushort2 color;
	surf2Dread(&color, src, x * sizeof(ushort2), y, cudaBoundaryModeZero);
	ushort2* dstPtr = (ushort2*)((ushort2*)dst + (height - y - 1) * width);
	dstPtr[x] = color;
}

// R -> R (float16)
template<> __global__ void
fromSurface<half>(void* dst, int width, int height, cudaSurfaceObject_t src)
{
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	short color;
	surf2Dread(&color, src, x * sizeof(short), y, cudaBoundaryModeZero);
	short* dstPtr = (short*)((short*)dst + (height - y - 1) * width);
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

// RGBA -> RGBA (float16)
template<> __global__ void
planarToSurface<Half4, half, 0, 1, 2, 3>(cudaSurfaceObject_t dst, int width, int height, const void* src) {
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	size_t stride = width * height;
	const short* srcPtr = static_cast<const short*>(src);
	size_t idx = x + (height - y - 1) * width;

	ushort4 color;
	color.x = srcPtr[idx];
	color.y = srcPtr[idx + stride];
	color.z = srcPtr[idx + 2 * stride];
	color.w = srcPtr[idx + 3 * stride];

	surf2Dwrite(color, dst, x * sizeof(ushort4), y);
}

// RGBA -> BGRA (float16)
template<> __global__ void
planarToSurface<Half4, half, 2, 1, 0, 3>(cudaSurfaceObject_t dst, int width, int height, const void* src) {
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	size_t stride = width * height;
	const short* srcPtr = static_cast<const short*>(src);
	size_t idx = x + (height - y - 1) * width;

	ushort4 color;
	color.z = srcPtr[idx];
	color.y = srcPtr[idx + stride];
	color.x = srcPtr[idx + 2 * stride];
	color.w = srcPtr[idx + 3 * stride];

	surf2Dwrite(color, dst, x * sizeof(ushort4), y);
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

// RGB -> RGBA (float16)
template<> __global__ void
planarToSurface<Half4, half, 0, 1, 2>(cudaSurfaceObject_t dst, int width, int height, const void* src) {
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	size_t stride = width * height;
	const short* srcPtr = static_cast<const short*>(src);
	size_t idx = x + (height - y - 1) * width;

	ushort4 color;
	color.x = srcPtr[idx];
	color.y = srcPtr[idx + stride];
	color.z = srcPtr[idx + 2 * stride];
	color.w = 0;

	surf2Dwrite(color, dst, x * sizeof(ushort4), y);
}

// BGR -> RGBA (float16)
template<> __global__ void
planarToSurface<Half4, half, 2, 1, 0>(cudaSurfaceObject_t dst, int width, int height, const void* src) {
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	size_t stride = width * height;
	const short* srcPtr = static_cast<const short*>(src);
	size_t idx = x + (height - y - 1) * width;

	ushort4 color;
	color.z = srcPtr[idx];
	color.y = srcPtr[idx + stride];
	color.x = srcPtr[idx + 2 * stride];
	color.w = 0;

	surf2Dwrite(color, dst, x * sizeof(ushort4), y);
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

// RG -> RG (float16)
template<> __global__ void
planarToSurface<half2, half, 0, 1>(cudaSurfaceObject_t dst, int width, int height, const void* src) {
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	size_t stride = width * height;
	const short* srcPtr = static_cast<const short*>(src);
	size_t idx = x + (height - y - 1) * width;

	ushort2 color;
	color.x = srcPtr[idx];
	color.y = srcPtr[idx + stride];

	surf2Dwrite(color, dst, x * sizeof(ushort2), y);
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

// RGBA -> BGRA (float16) from interleaved
template<> __global__ void
toSurface<Half4, half, 2, 1, 0, 3>(cudaSurfaceObject_t dst, int width, int height, const void* src)
{
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	const short* srcPtr = static_cast<const short*>(src);
	size_t idx = (x + (height - y - 1) * width) * 4;

	ushort4 color;
	color.x = srcPtr[idx + 2];
	color.y = srcPtr[idx + 1];
	color.z = srcPtr[idx + 0];
	color.w = srcPtr[idx + 3];

	surf2Dwrite(color, dst, x * sizeof(ushort4), y);
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

// RGBA -> RGB (float16) from interleaved
template<> __global__ void
toSurface<Half4, half, 0, 1, 2>(cudaSurfaceObject_t dst, int width, int height, const void* src)
{
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	const short* srcPtr = static_cast<const short*>(src);
	size_t idx = (x + (height - y - 1) * width) * 3;

	ushort4 color;
	color.x = srcPtr[idx];
	color.y = srcPtr[idx + 1];
	color.z = srcPtr[idx + 2];
	color.w = 0;

	surf2Dwrite(color, dst, x * sizeof(ushort4), y);
}

// BGR -> RGBA (float16) from interleaved
template<> __global__ void
toSurface<Half4, half, 2, 1, 0>(cudaSurfaceObject_t dst, int width, int height, const void* src)
{
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	const short* srcPtr = static_cast<const short*>(src);
	size_t idx = (x + (height - y - 1) * width) * 3;

	ushort4 color;
	color.z = srcPtr[idx];
	color.y = srcPtr[idx + 1];
	color.x = srcPtr[idx + 2];
	color.w = 0;

	surf2Dwrite(color, dst, x * sizeof(ushort4), y);
}


// BGRA -> BRGA from interleaved
// RGBA -> RGBA from interleaved
// RG -> RG from interleaved
// R -> R (uint8_t, float32, float16) from Planar/Interleaved
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

// RGBA -> RGBA (float16) from interleaved
template<> __global__ void
toSurface<Half4>(cudaSurfaceObject_t dst, int width, int height, const void* src)
{
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	ushort4 color = *(ushort4*)((ushort4*)src + x + (height - y - 1) * width);
	surf2Dwrite(color, dst, x * sizeof(ushort4), y, cudaBoundaryModeZero);
}

// RG -> RG (float16) from interleaved
template<> __global__ void
toSurface<half2>(cudaSurfaceObject_t dst, int width, int height, const void* src)
{
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	ushort2 color = *(ushort2*)((ushort2*)src + x + (height - y - 1) * width);
	surf2Dwrite(color, dst, x * sizeof(ushort2), y, cudaBoundaryModeZero);
}

// R -> R (float16) from interleaved
template<> __global__ void
toSurface<half>(cudaSurfaceObject_t dst, int width, int height, const void* src)
{
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

	if (x >= width || y >= height)
		return;

	short color = *(short*)((short*)src + x + (height - y - 1) * width);
	surf2Dwrite(color, dst, x * sizeof(short), y, cudaBoundaryModeZero);
}
