#pragma once

// #include <fay/core/fay.h>
#include "fay/base/concept.h"
#include "fay/container/container.h"

namespace fay
{

#pragma region tensor

#pragma region internal

// TODO: move to heap???
template <arithmetic_c T, size_t R_, size_t C_>
struct base_tensor_
{
    enum { R = R_, C = C_, N = R * C };
    union
    {
        // all of anonymous union's data members must be public
        T m_[R][C];
        T a_[N]{};
    };
};

template <typename T>
struct base_tensor_<T, 1, 1>
{
    enum { R = 1, C = 1, N = R * C };
    union
    {
        struct { T x; };
        struct { T s; };
        struct { T u; };

        T m_[R][C];
        T a_[N]{};
    };
};

template <typename T>
struct base_tensor_<T, 1, 2>
{
    enum { R = 1, C = 2, N = R * C };
    union
    {
        struct { T x, y; };
        struct { T s, t; };
        struct { T u, v; };

        T m_[R][C];
        T a_[N]{};
    };
};

template <typename T>
struct base_tensor_<T, 1, 3>
{
    enum { R = 1, C = 3, N = R * C };
    union
    {
        struct { T x, y, z; };
        struct { T r, g, b; };

        T m_[R][C];
        T a_[N]{};
    };
};

template <typename T>
struct base_tensor_<T, 1, 4>
{
    enum { R = 1, C = 4, N = R * C };
    union
    {
        struct { T x, y, z, w; };
        struct { T r, g, b, a; };
        //struct { tensor3 xyz; };
        //struct { tensor2 xy; tensor2 zw; };

        T m_[R][C];
        T a_[N]{};
    };
};

#pragma endregion

#pragma region interface

// TODO: rename to vec, remove T m_[R][C]
// template <scaler T, size_t R_, size_t C_>
template <arithmetic_c T, size_t R_, size_t C_>
struct tensor : base_tensor_<T, R_, C_>, mixin_sequence<T, tensor<T, R_, C_>>
{
    // TODO: fay::tensor<T> a;
    static_assert(R_ > 0 && C_ > 0);

public:
    using base_type = base_tensor_<T, R_, C_>;
    using base_type::C;
    using base_type::R;
    using base_type::N;
    using base_type::m_;
    using base_type::a_;

    FAY_SEQUENCE_TYPE_ALIAS

public:
    // TODO: make fay::tensor to be a aggregate

    // TODO: move ctors to mixin_sequence

    // WARNING!!! tensor(n) != tensor{ n }
    // TODO£ºremove£¿£¿
    constexpr explicit tensor(std::initializer_list<value_type> il) : tensor(il.begin(), il.end()) { DCHECK(il.size() <= N); }
    constexpr explicit tensor(const_reference v) { fill(v); }

    //! Sequence must have 'begin' and 'end' method
    template<typename Sequence>
    constexpr explicit tensor(const Sequence& c) : tensor(cbegin(c), cbegin(c) + std::min(std::size(c), N)) {}
    template<typename Sequence>
    constexpr explicit tensor(const Sequence& c, size_type n) : tensor(cbegin(c), cbegin(c) + n) { DCHECK(n <= std::size(c)); }

    // TODO: wait for C++20
    //! if n >= N, all elems are init by [*p, *(p+N))
    //! if n <  N, the front elems are same, the rest are defalut value
    //constexpr explicit tensor(const_pointer p, size_type n) : tensor(p, p + std::min(n, N)) {}

    constexpr explicit tensor(const_pointer first, const_pointer last)
    {
        auto diff = std::distance(first, last);
        DCHECK(diff > 0) << "range isn't valid, is it used 'tensor t{}' ???";
        DCHECK(diff <= N);

        std::copy(first, last, a_); // TODO: deque and list
    }

public:
    // set
    constexpr void fill(const_reference v) { std::fill_n(a_, N, v); }

    // TODO: move to mixin_sequence
    // get
    reference       operator[](size_type i)       noexcept { return a_[i]; }
    const_reference operator[](size_type i) const noexcept { return a_[i]; }

    pointer       data()       noexcept { return a_; }
    const_pointer data() const noexcept { return a_; }

    // capaciiity
    constexpr size_type size()   const noexcept { return N; }
    constexpr bool      emptry() const noexcept { return false; }
};
#pragma endregion

#pragma endregion tensor

} // namespace fay
