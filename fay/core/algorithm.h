#pragma once
#include "fay/core/fay.h"

namespace fay
{

// TODO
/*
template<class InputIt, class OutputIt, class BinaryOperation>
inline void transform(InputIt first1, InputIt last1, OutputIt first2,
    BinaryOperation binary_op)
{
    while (first1 != last1)
    {
        binary_op(*first1++, *first2++);
    }
}
*/

inline void loop(uint cnt, std::function<void()> functor)
{
    for (uint i = 0; i < cnt; ++i)
        functor();
}

inline void loop(uint cnt, std::function<void(uint)> functor)
{
    for (uint i = 0; i < cnt; ++i)
        functor(i);
}

#pragma region equal

// one func don't do two different things
// bool is_seq_equal(const Sequence& a0, const Sequence& a1, std::optional<size_t> count)

// TODO???
//template<Sequence T0, Sequence T1>
//inline bool operator==(const T0& a, const T1& b, size_t count) noexcept



//! a.size() == b.size() and a[i] == b[i]
//! WARNING: can't directly use {...} in is_seq_equal(...);
template<typename T0, typename T1>
// requires T0 and T1 is seq, then rename to is_equal
// requires T0::value_type == T1::value_type
/*constexpr*/
inline bool is_seq_equal(const T0& a, const T1& b) noexcept
{
    if (std::size(a) != std::size(b))
        return false;

    return std::equal(std::begin(a), std::end(a), std::begin(b));
}

template<typename T0, typename T1>
/*constexpr*/
inline bool not_seq_equal(const T0& a, const T1& b) noexcept
{
    return !is_seq_equal(a, b);
}

#pragma endregion equal

} // namespace fay
