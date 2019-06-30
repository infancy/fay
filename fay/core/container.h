#pragma once

#include <fay/core/fay.h>

namespace fay
{

/*
template<typename T>
concept Container = requires(T&& t) 
{
    // ...
};

template<typename T>
concept Container = std::Range
*/

template<typename T>
class container
{
public:
    using size_type = int;
};

} // namespace fay