#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef FAY_UTI_FAY_H
#define FAY_UTI_FAY_H

#include "config.h"

// Global Include Files
#include <cassert>
#include <cinttypes>
#include <cmath>
#include <cstdint>
#include <cstring>

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// #include <boost/noncopyable.hpp>
#include <glog/logging.h>

// Platform-specific definitions
#if defined(FAY_IN_MSVC)
#include <float.h>
#include <intrin.h>
#pragma warning(disable : 4305)  // double constant assigned to float
#pragma warning(disable : 4244)  // int -> float conversion
#pragma warning(disable : 4843)  // double -> float conversion
#pragma warning(disable : 4201)  // 非标准拓展：匿名的结构、联合
#endif

namespace fay
{

// Global Constants
/*
static constexpr float Maxfloat = std::numeric_limits<float>::max();
static constexpr float Infinity = std::numeric_limits<float>::infinity();

static constexpr float MachineEpsilon = std::numeric_limits<float>::epsilon() * 0.5;

static constexpr float Pi	   = 3.14159265358979323846;
static constexpr float InvPi   = 0.31830988618379067154;
static constexpr float Inv2Pi  = 0.15915494309189533577;
static constexpr float Inv4Pi  = 0.07957747154594766788;
static constexpr float PiOver2 = 1.57079632679489661923;
static constexpr float PiOver4 = 0.78539816339744830961;
static constexpr float Sqrt2   = 1.41421356237309504880;

// Global Inline Functions

inline float radians(float deg) { return (Pi / 180) * deg; }

inline float degrees(float rad) { return (180 / Pi) * rad; }

template <typename T, typename U, typename V>
inline T clamp(T val, U low, V high)
{
	if (val < low)
		return low;
	else if (val > high)
		return high;
	else
		return val;
}

inline float gamma(int n) 
{
	return (n * MachineEpsilon) / (1 - n * MachineEpsilon);
}
*/

enum class Thirdparty { none, gl, dx, vk };

// graphics --------------------------------------------------------------------

enum class ModelType { obj, fbx, gltf, blend, unknown };

enum class TexType { none, diffuse, specular, ambient, emissive, height, alpha,
	normals, shininess, opacity, displace, lightmap, reflection, unknown };

} // namespace fay


#endif //FAY_UTI_FAY_H
