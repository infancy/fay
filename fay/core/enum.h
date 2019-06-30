#pragma once

#include "fay/core/fay.h"

namespace fay
{

// https://github.com/Neargye/magic_enum
// https://github.com/aantron/better-enums

// -------------------------------------------------------------------------------------------------

/*
// a scoped enum with methods
template<typename E>
class base_enum
{
public:
    using base_type = base_enum<E>;
    using enum_type = typename E::enum_t;
    // ...
};

*/

// -------------------------------------------------------------------------------------------------

template<typename E>
using enum_cast_t = std::underlying_type_t<E>;

// get the value of underlying type
template <typename E>
constexpr auto enum_cast(E e) noexcept
{
    return static_cast<enum_cast_t<E>>(e);
}

// enum_cast<E>("name")
// enum_cast("E::name")
// enum_name("E::name")

// -------------------------------------------------------------------------------------------------

struct enum_class_hash_func
{
    template <typename T>
    constexpr auto operator()(T t) const noexcept
    {
        return enum_cast(t);
    }
};

template <typename EKey, typename T>
using enum_class_map = std::unordered_map<EKey, T, enum_class_hash_func>;
// using enum_class_map = std::unordered_map<Key, T, decltype([]<typename EKey>(E e) { return enum_cast(e); })>;

// -------------------------------------------------------------------------------------------------

#define FAY_ENUM_CLASS_OPERATORS(Enum)                                                                                        \
inline Enum  operator~(const Enum a)               { return static_cast<Enum>(~static_cast<int>(a)); }                        \
inline Enum  operator&(const Enum a, const Enum b) { return static_cast<Enum>( static_cast<int>(a) & static_cast<int>(b) ); } \
inline Enum  operator|(const Enum a, const Enum b) { return static_cast<Enum>( static_cast<int>(a) | static_cast<int>(b) ); } \
inline Enum& operator|=(Enum& a, const Enum b) { a = a | b; return a; };                                                      \
inline Enum& operator&=(Enum& a, const Enum b) { a = a & b; return a; };                                                      \
inline bool enum_have(const Enum group, const Enum value) { return (group & value) != (Enum)0; }

// TODO: rename type_
#define FAY_ENUM_METHODS( type_ )                                                                          \
using this_type = type_;                                                                                   \
using value_type = std::underlying_type_t<enum_type>;                                                      \
                                                                                                           \
value_type e_{};                                                                                           \
                                                                                                           \
constexpr explicit type_(value_type e) noexcept : e_{ e } {}                                               \
                                                                                                           \
constexpr bool operator==(const this_type right) const noexcept { return e_ == right.e_; }                 \
constexpr bool operator!=(const this_type right) const noexcept { return operator==(right); }              \
                                                                                                           \
constexpr this_type operator~() const noexcept { return this_type{ ~e_ }; }                                \
                                                                                                           \
constexpr this_type operator&(const this_type right) const noexcept { return this_type{ e_ & right.e_ }; } \
constexpr this_type operator|(const this_type right) const noexcept { return this_type{ e_ | right.e_ }; } \
                                                                                                           \
constexpr this_type& operator|=(const this_type right) noexcept { e_ = e_ | right.e_; return *this; }      \
constexpr this_type& operator&=(const this_type right) noexcept { e_ = e_ & right.e_; return *this; }      \
                                                                                                           \
constexpr value_type value() const noexcept { return e_; }                                                 \
constexpr bool have(value_type e) const noexcept { return (e_ & e) == value_type(0); }

} // namespace fay
