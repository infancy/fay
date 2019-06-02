#pragma once

#include "fay/core/fay.h"

namespace fay
{

// https://github.com/Neargye/nameof

template<typename T>
constexpr string nameof() 
{ 

};

template<typename Enum, typename = std::enable_if_t<std::is_enum_v<Enum>>>
constexpr string nameof(Enum e)
{

};

// typeof/decltype

} // namespace fay