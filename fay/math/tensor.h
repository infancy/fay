#pragma once

// #include <fay/core/fay.h>
#include "fay/container/container.h"

namespace fay
{

#pragma region tensor

#pragma region internal

template <typename T, size_t R_, size_t C_>
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

template <typename T, size_t R_, size_t C_>
struct tensor : base_tensor_<T, R_, C_>, sequence<tensor<T, R_, C_>>
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

    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using value_type = T;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = value_type*;
    using const_iterator = const value_type*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

public:
    // TODO: make fay::tensor to be a aggregate

    // WARNING!!! tensor(n) != tensor{ n }
    // TODO£ºremove£¿£¿£¿
    constexpr explicit tensor(std::initializer_list<value_type> il) /*: a_{ il }*/
    {
        if (il.size() > N) 
            throw std::out_of_range("fay::tensor::tensor(std::initializer_list<value_type> il)");

        // the rest is defalut value
        std::copy(il.begin(), il.end(), a_);
    }
    constexpr explicit tensor(const_reference v) { fill(v); }
    constexpr explicit tensor(const_pointer   p, size_type n = 0) { std::copy(p, p + (n > 0 ? n : N), a_); }

    // set
    constexpr void fill(const_reference v) { std::fill_n(a_, N, v); }

    // get
    reference       operator[](size_type i)       noexcept { return a_[i]; }
    const_reference operator[](size_type i) const noexcept { return a_[i]; }

    reference       at(size_type i) { if (i >= N) throw std::out_of_range("fay::tensor::at"); return a_[i]; }
    const_reference at(size_type i) const { if (i >= N) throw std::out_of_range("fay::tensor::at"); return a_[i]; }

    pointer         data()       noexcept { return a_; }
    const_pointer   data() const noexcept { return a_; }

    // capaciiity
    constexpr size_type    size()    const noexcept { return N; }
    constexpr bool         emptry()  const noexcept { return false; }
};
#pragma endregion

#pragma endregion

} // namespace fay
