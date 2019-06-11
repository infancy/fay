#pragma once

namespace fay
{

// https://github.com/Neargye/magic_enum
// https://github.com/aantron/better-enums

/*
auto enum_class_hash = [] (auto k)
{
    // return static_cast<std::underlying_type_t<Key>>(k);
    return static_cast<std::size_t>(k);
};
*/

struct enum_class_hash
{
    template <typename T>
    constexpr auto operator()(T t) const
    {
        return static_cast<std::underlying_type_t<T>>(t);
    }
};

template <typename Key, typename T>
using enum_class_map = std::unordered_map<Key, T, enum_class_hash>;

#define FAY_ENUM_CLASS_OPERATORS(Enum)                                                                                        \
inline Enum  operator~(const Enum a)               { return static_cast<Enum>(~static_cast<int>(a)); }                        \
inline Enum  operator&(const Enum a, const Enum b) { return static_cast<Enum>( static_cast<int>(a) & static_cast<int>(b) ); } \
inline Enum  operator|(const Enum a, const Enum b) { return static_cast<Enum>( static_cast<int>(a) | static_cast<int>(b) ); } \
inline Enum& operator|=(Enum& a, const Enum b) { a = a | b; return a; };                                                      \
inline Enum& operator&=(Enum& a, const Enum b) { a = a & b; return a; };                                                      \
inline bool enum_have(const Enum group, const Enum value) { return (group & value) != (Enum)0; }

// TODO:
template <typename T>
constexpr std::underlying_type_t<T> enum_cast(T x)
{
    return static_cast<std::underlying_type_t<T>>(x);
}

} // namespace fay
