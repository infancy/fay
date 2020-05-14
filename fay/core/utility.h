#pragma once

#include <optional>

#include "fay/core/algorithm.h"
#include "fay/core/range.h"

namespace fay
{

template<typename X, typename Y>
struct pair
{
    union
    {
        struct { X x; Y y; };
        struct { X k; Y v; };
        struct { X key; Y value; };
        struct { X first; Y second; };
    };
};

template<typename X, typename Y, typename Z>
struct triple
{
    union
    {
        struct { X x; Y y; Z z };
        struct { X v0; Y v1; Z v2; };
        struct { X first; Y second; Z third; };
    };
};

template<typename Container, typename UnaryPredicate>
inline decltype(auto) find(const Container& container, UnaryPredicate p)
{
    return std::find_if(std::cbegin(container), std::cend(container), p);
}

// std::equal_to, std::not_equal_to, std::greater, std::less, std::greater_equal(>=), std::less_equal(<=)
template<typename Container, typename BinaryPredicate>
inline decltype(auto) find(const Container& container, BinaryPredicate p, const typename Container::value_type& value)
{
    static_assert(!std::is_floating_point_v<typename Container::value_type>, "not allowed float point");
    //static_assert(std::is_same_v<Container::value_type, T>, "not same");

    return find(container, [&p, &value](typename Container::value_type v) { return p(v, value); });
}



template<typename Container, typename BinaryPredicate>
inline bool all_of(const Container& container, BinaryPredicate p, const typename Container::value_type& value)
{
    return find(container, std::not_fn(p), value) == container.end();
}

template<typename Container, typename UnaryPredicate>
inline bool all_of(const Container& container, UnaryPredicate p)
{
    // The first element that does not meet the condition is at the end, that is, the previous elements all meet the condition
    return find(container, std::not_fn(p)) == container.end();
}

template<typename Container, typename BinaryPredicate>
inline bool any_of(const Container& container, BinaryPredicate p, const typename Container::value_type& value)
{
    return find(container, p, value) != container.end();
}

// have any value of container meet the predicate
// any_of: one is enough, some_of: several
template<typename Container, typename UnaryPredicate>
inline bool any_of(const Container& container, UnaryPredicate p)
{
    return find(container, p) != container.end();
}

template<typename Container, typename BinaryPredicate>
inline bool none_of(const Container& container, BinaryPredicate p, const typename Container::value_type& value)
{
    return find(container, p, value) == container.end();
}

template<typename Container, typename UnaryPredicate>
inline bool none_of(const Container& container, UnaryPredicate p)
{
    return find(container, p) == container.end();
}



// have<true>(array)
// template<auto value, typename Container>
// bool have(const Container& container)
// decltype(value)
// std::find



// index_of(value, container);

// template<typename Container, typename T = typename Container::value_type>
template<typename Container, typename T>
inline std::optional<uint> index(const Container& container, T value,
    typename std::enable_if_t<!std::is_invocable_r_v<bool, T, const typename Container::value_type&>>* = nullptr)
{
    // Double Insurance
    static_assert(std::is_same_v<typename Container::value_type, T>, "not same");

    // auto&& iter = find(container, std::equal_to{}, value);
    auto iter = find(container, std::equal_to{}, value);
    // using T = decltype(std::begin(container));
    // static_assert(std::is_same_v<decltype(iter), decltype(std::begin(container))>, "error");
    return (iter == container.end()) ? std::nullopt : std::optional<uint>{std::distance(std::begin(container), iter)};
}

// TODO: concept

// lambda is not a function, but a callable object
template<typename Container, typename UnaryPredicate>
inline std::optional<uint> index(const Container& container, UnaryPredicate p,
    typename std::enable_if_t<std::is_invocable_r_v<bool, UnaryPredicate, const typename Container::value_type&>>* = nullptr)
    // || std::is_invocable_r_v<bool, UnaryPredicate, typename Container::value_type>
{
    //static_assert(!std::is_integral_v<Lambda>, "error");
    //static_assert(!std::is_function_v<Lambda>, "error");

    auto iter = find(container, p);
    return (iter == container.end()) ? std::nullopt : std::optional<uint>{ std::distance(std::begin(container), iter) };
}

template<typename T>
inline bool is_clamp(T val, T low, T high)
{
    return (val >= low) && (val <= high); // (val >= low) && (val < high)???
};

} // namespace fay