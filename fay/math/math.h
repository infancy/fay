#pragma once

#include <limits>

#include "fay/core/fay.h"

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace fay
{

// -------------------------------------------------------------------------------------------------
// constant value



constexpr float MinFloat = std::numeric_limits<float>::min();
constexpr float MaxFloat = std::numeric_limits<float>::max();
constexpr float Infinity = std::numeric_limits<float>::infinity();

constexpr float MachineEpsilon = std::numeric_limits<float>::epsilon() * 0.5;

constexpr float Pi      = 3.14159265358979323846;
constexpr float InvPi   = 0.31830988618379067154;
constexpr float Inv2Pi  = 0.15915494309189533577;
constexpr float Inv4Pi  = 0.07957747154594766788;
constexpr float PiOver2 = 1.57079632679489661923;
constexpr float PiOver4 = 0.78539816339744830961;
constexpr float Sqrt2   = 1.41421356237309504880;



// -------------------------------------------------------------------------------------------------
//



constexpr float radians(float deg) { return (deg / 180.f) * Pi; }

constexpr float degrees(float rad) { return (rad / Pi) * 180.f; }



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



template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
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
constexpr bool is_equal(T x, T y, T epsilon = fay::equal_epsilon<T>::absolute_epsilon)
{
    if constexpr (std::is_floating_point_v<T>)
        // return std::fabs(x - y) <= std::max(absolute_epsilon, relative_epsilon * std::max(fabs(x), fabs(y)) );
        return std::abs(x - y) <= epsilon * fay::max(T(1), std::abs(x), std::abs(y));
    else
        return x == y;

    // else if constexpr (std::is_arithmetic_v<T>)
    //     return x == y;
    // else
    //     static_assert(false, "T of is_equal<T> must be arithmetic type"); // ???
}

template <typename T>
constexpr bool not_equal(T x, T y, T epsilon = fay::equal_epsilon<T>::absolute_epsilon)
{
    return !is_equal(x, y, epsilon);
}

} // namespace fay
