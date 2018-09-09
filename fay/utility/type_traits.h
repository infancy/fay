#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_UTILITY_TYPE_TRAITS_H
#define FAY_UTILITY_TYPE_TRAITS_H

#include <type_traits>

namespace fay
{

template <typename T>
struct is_not_void_ : std::true_type {};

template <>
struct is_not_void_<void> : std::false_type {};

template <typename T>
struct is_not_void : is_not_void_<std::remove_cv_t<T>> {};

template <typename T>
constexpr bool is_not_void_v = is_not_void<T>::value;

template <typename R, typename... ArgTypes>
using return_type = R;

} // namespace fay

#endif // FAY_UTILITY_TYPE_TRAITS_H
