#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_UTILITY_MATH_H
#define FAY_UTILITY_MATH_H

#include <limits>

namespace fay
{

static constexpr float MaxFloat = std::numeric_limits<float>::max();
static constexpr float Infinity = std::numeric_limits<float>::infinity();

static constexpr float MachineEpsilon = std::numeric_limits<float>::epsilon() * 0.5;

static constexpr float Pi = 3.14159265358979323846;
static constexpr float InvPi = 0.31830988618379067154;
static constexpr float Inv2Pi = 0.15915494309189533577;
static constexpr float Inv4Pi = 0.07957747154594766788;
static constexpr float PiOver2 = 1.57079632679489661923;
static constexpr float PiOver4 = 0.78539816339744830961;
static constexpr float Sqrt2 = 1.41421356237309504880;

inline float radians(float deg) { return (Pi / 180) * deg; }

inline float degrees(float rad) { return (180 / Pi) * rad; }

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
inline T tmin(T a, T b, T c)
{
	return (a < b) ? (a < c ? a : c) : (b < c ? b : c);
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
inline T tmax(T a, T b, T c)
{
	return (a > b) ? (a > c ? a : c) : (b > c ? b : c);
}

template <typename T>
inline T clamp(T val, T low, T high)
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

} // namespace fay

#endif // FAY_UTILITY_MATH_H
