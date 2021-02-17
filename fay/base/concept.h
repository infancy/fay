#pragma once

#include <concepts>


namespace fay
{

template<typename T>
concept arithmetic_c = std::floating_point<T> || std::integral<T>;

} // namespace fay
