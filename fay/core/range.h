#pragma once

#include "fay/core/fay.h"

namespace fay
{

// https://github.com/JeffPaine/beautiful_idiomatic_python
// https://github.com/ryanhaining/cppitertools
// std::range::view::iota

// a naive implementation
// return value of range should provide 'begin' and 'end' functions
template<typename T>
std::vector<T> range(T start, T end, T step)
{
    static_assert(std::is_integral_v<T>, "fay::range have to use intergral type");
    
    // TODO: should check it?
    // count()
    //DCHECK(step != T(0));
    DCHECK(
        ((start <= end) && step > T(0)) ||
        ((start >= end) && step < T(0)));

    std::vector<T> v{};

    /*
    bool flag = strat < end;
    while ((start < end) == flag)
    {
        v.push_back(start);
        start += step;
    }
    */

    if (start < end)
    {
        while (start < end) // while (start >= end)
        {
            v.push_back(start);
            start += step;
        }
    }
    else if (start > end)
    {
        while (start > end)
        {
            v.push_back(start);
            start += step;
        }
    }

    return v;
}

template<typename T>
std::vector<T> range(T start, T end)
{
    return range(start, end, T(1));
}

template<typename T>
std::vector<T> range(T end)
{
    return range(T(0), end, T(1));
}

} // namespace fay