#pragma once

namespace fay
{

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
        return static_cast<typename std::underlying_type_t<T>>(t);
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

template <typename T>
constexpr typename std::underlying_type<T>::type enum_cast(T x)
{
    return static_cast<typename std::underlying_type_t<T>>(x);
}

} // namespace fay
