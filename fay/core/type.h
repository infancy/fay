#pragma once

#include <type_traits>

namespace fay
{

// -------------------------------------------------------------------------------------------------
// type_enum

enum class type_enum
{
    none_,

    bool_,
    char_,
    short_,
    int_,
    long_,
    long_long_,

    unsigned_char_,
    unsigned_short_,
    unsigned_int_,
    unsigned_long_,
    unsigned_long_long_,

    float_,
    double_,
    long_double_,

    // TODO: cv-*

    glm_vec4_
};

template <type_enum t>
using type_constant = std::integral_constant<type_enum, t>;

template <typename T>
struct type_to_enum : type_constant<type_enum::none_> {};

template<type_enum t>
struct enum_to_type {};

#define FAY_TYPE_ENUM(T) \
template <> \
struct type_to_enum<T> : type_constant<type_enum::T##_> {}; \
template<> \
struct enum_to_type<type_enum::T##_> { using type = T; };

#define FAY_TYPE_ENUM2(T1, T2) \
template <> \
struct type_to_enum<T1 T2> : type_constant<type_enum::T1##_##T2##_> {}; \
template<> \
struct enum_to_type<type_enum::T1##_##T2##_> { using type = T1 T2; };

FAY_TYPE_ENUM(bool)
FAY_TYPE_ENUM(char)
FAY_TYPE_ENUM(short)
FAY_TYPE_ENUM(int)
FAY_TYPE_ENUM(long)
FAY_TYPE_ENUM2(long, long)

FAY_TYPE_ENUM2(unsigned, char)
FAY_TYPE_ENUM2(unsigned, short)
FAY_TYPE_ENUM2(unsigned, int)
FAY_TYPE_ENUM2(unsigned, long)

FAY_TYPE_ENUM(float)
FAY_TYPE_ENUM(double)
FAY_TYPE_ENUM2(long, double)

#undef FAY_TYPE_ENUM
#undef FAY_TYPE_ENUM2

template <>
struct type_to_enum<unsigned long long> : type_constant<type_enum::unsigned_long_long_> {};

template <>
struct enum_to_type<type_enum::unsigned_long_long_> { using type = unsigned long long; };

template <typename T>
constexpr type_enum type_v = type_to_enum<T>::value;

template <type_enum t>
using type_t = typename enum_to_type<t>::type; // WARNNING: missing the 'typename'

template <typename T>
constexpr type_enum typeof(const T&) { return type_v<T>; }


// -------------------------------------------------------------------------------------------------
// is_not_void

template <typename T>
struct is_not_void_ : std::true_type {};

template <>
struct is_not_void_<void> : std::false_type {};

template <typename T>
struct is_not_void : is_not_void_<std::remove_cv_t<T>> {};

template <typename T>
constexpr bool is_not_void_v = is_not_void<T>::value;

// -------------------------------------------------------------------------------------------------
// return_type

template <typename R, typename... Args>
using return_type = R;

// consteval

} // namespace fay
