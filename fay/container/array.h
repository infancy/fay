#pragma once

// #include <fay/core/fay.h>
#include <fay/container/container.h>

namespace fay
{

#pragma region static_array/stack_array

template <typename T, size_t N>
struct array : sequence<array<T, N>>
{
    static_assert(N > 0);

public:

    FAY_SEQUENCE_tYPE_ALIAS

public:
    //! WARNING!!! can not use 'array a', 'array a{}', always init by something 
    //constexpr array() = default;

    //! WARNING!!! array(n) != array{ n }
    constexpr explicit array(std::initializer_list<value_type> il) : array(il.begin(), il.end()) { DCHECK(il.size() <= N); }

    constexpr explicit array(const_reference v) { fill(v); }

    //! Sequence must have 'begin' and 'end' method
    template<typename Sequence>
    constexpr explicit array(const Sequence& c) : array(cbegin(c), cbegin(c) + std::min(std::size(c), N)) {}
    template<typename Sequence>
    constexpr explicit array(const Sequence& c, size_type n) : array(cbegin(c), cbegin(c) + n) { DCHECK(n <= std::size(c)); }

    // TODO: wait for C++20
    //! if n >= N, all elems are init by [*p, *(p+N))
    //! if n <  N, the front elems are same, the rest are defalut value
    //constexpr explicit array(const_pointer p, size_type n) : array(p, p + std::min(n, N)) {}

    constexpr explicit array(const_pointer first, const_pointer last)
    {
        auto diff = std::distance(first, last);
        DCHECK(diff > 0) << "range isn't valid, is it used 'array a{}' ???";
        DCHECK(diff <= N);

        std::copy(first, last, a_); // TODO: deque and list
    }

public:
    // set
    constexpr void fill(const_reference v) { std::fill_n(a_, N, v); }

    // get
    reference       operator[](size_type i)       { return a_[i]; }
    const_reference operator[](size_type i) const { return a_[i]; }

    pointer         data()       noexcept { return a_; }
    const_pointer   data() const noexcept { return a_; }

    // capaciiity
    constexpr size_type size()   const noexcept { return N; }
    constexpr bool      emptry() const noexcept { return false; }

private:
    // because in the beginning provides user-define ctor, fay::array is not a aggregate anymore
    T a_[N]{};
};

#pragma endregion



#pragma region dynamic_array/heap_array

// fay::array<T> is a variable-length arrays(VLA)
// TODO: fay::array<T> is a fay::container(concept)
// WARNING: when use fay::container, always use 'fay::' to avoid confusion with standard container

template<typename T>
class array<T, 0>
{
public:
    using size_type = int;

public:
    explicit array(size_type size)
    {

    }

    size_type size()
    {
        return sz_;
    }

private:
    // TODO: heap_value, heap_aray
    size_type sz_;
    std::unique_ptr<T[]> a_;
};

#pragma endregion

} // namespace fay