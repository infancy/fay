#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_UTILITY_TYPE_TRAITS_H
#define FAY_UTILITY_TYPE_TRAITS_H

#include <type_traits>

namespace fay
{

template <typename T>
struct _is_not_void : std::true_type {};

template <>
struct _is_not_void<void> : std::false_type {};

template <typename T>
struct is_not_void : _is_not_void<std::remove_cv_t<T>> {};

template <typename T>
constexpr bool is_not_void_v = is_not_void<T>::value;

} // namespace fay

#endif // FAY_UTILITY_TYPE_TRAITS_H
