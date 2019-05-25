#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_MATH_MATH_H
#define FAY_MATH_MATH_H

#include <limits>

#include "fay/core/fay.h"

namespace fay
{

constexpr inline float MinFloat = std::numeric_limits<float>::min();
constexpr inline float MaxFloat = std::numeric_limits<float>::max();
constexpr inline float Infinity = std::numeric_limits<float>::infinity();

constexpr inline float MachineEpsilon = std::numeric_limits<float>::epsilon() * 0.5;

constexpr inline float Pi      = 3.14159265358979323846;
constexpr inline float InvPi   = 0.31830988618379067154;
constexpr inline float Inv2Pi  = 0.15915494309189533577;
constexpr inline float Inv4Pi  = 0.07957747154594766788;
constexpr inline float PiOver2 = 1.57079632679489661923;
constexpr inline float PiOver4 = 0.78539816339744830961;
constexpr inline float Sqrt2   = 1.41421356237309504880;

// -------------------------------------------------------------------------------------------------
//



constexpr float radians(float deg) { return (Pi / 180.f) * deg; }

constexpr float degrees(float rad) { return (180.f / Pi) * rad; }

template<typename T, typename... Ts, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr T min(T a, T b, Ts... c)
{
    std::initializer_list<T> il{ a, b, c... };
    return *std::min_element(il.begin(), il.end());
}

template<typename T, typename... Ts, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr T max(T a, T b, Ts... c)
{
    std::initializer_list<T> il{ a, b, c... };
    return *std::max_element(il.begin(), il.end());
}

// TODO:remove
template <typename T>
constexpr T clamp(T val, T low, T high)
{
    return std::clamp(val, low, high);
}

constexpr float gamma(int n)
{
	return (n * MachineEpsilon) / (1 - n * MachineEpsilon);
}



// -------------------------------------------------------------------------------------------------
// numeric



// https://stackoverflow.com/questions/17333/what-is-the-most-effective-way-for-float-and-double-comparison
// http://realtimecollisiondetection.net/blog/?p=89

template <typename T>
struct equal_epsilon
{
	static constexpr T absolute_epsilon = std::numeric_limits<T>::epsilon();
	static constexpr T relative_epsilon = std::numeric_limits<T>::epsilon();
};

template <typename T>
constexpr bool is_equal(T a, T b, typename std::enable_if_t<!std::is_floating_point_v<T>>* = nullptr)
{
	return a == b;
}

template <typename T>
constexpr bool is_equal(T a, T b, typename std::enable_if_t<std::is_floating_point_v<T>>* = nullptr)
{
	// return std::fabs(a - b) <= std::max(absolute_epsilon, relative_epsilon * std::max(fabs(a), fabs(b)) );
	return std::abs(a - b) <= equal_epsilon<T>::absolute_epsilon * fay::max(T(1), std::abs(a), std::abs(b));
}

} // namespace fay

#endif // FAY_MATH_MATH_H
