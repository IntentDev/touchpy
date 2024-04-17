#pragma once

struct Vec3ToVec4Tag {};

template<typename T>
struct Vec3
{
	T x, y, z;
};

template<typename T>
struct Vec4
{
	T x, y, z, w;
};
