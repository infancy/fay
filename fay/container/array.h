#pragma once

// https://github.com/llvm-mirror/libcxx/blob/master/include/array
// https://github.com/microsoft/STL/blob/master/stl/inc/array
// https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/include/std/array
// https://github.com/electronicarts/EASTL/blob/master/include/EASTL/array.h
// https://github.com/boostorg/array/blob/develop/include/boost/array.hpp

// https://github.com/fffaraz/awesome-cpp#frameworks
// https://github.com/abseil/abseil-cpp/blob/master/absl/container/fixed_array.h
// https://github.com/facebook/folly/blob/master/folly/container/Array.h



// #include <fay/core/fay.h>
#include <fay/container/container.h>

namespace fay
{

#pragma region static_array/stack_array

#pragma region internal

// template <typename T, size_t R = 1, size_t C = 1>
template <typename T, size_t N_>
struct base_array_
{
    enum { N = N_ };
    union
    {
        // all of anonymous union's data members must be public
        T a_[N]{};
        // T m_[R][C]{};
    };
};

template <typename T>
struct base_array_<T, 2>
{
    enum { N = 2 };
    union
    {
        struct { T x, y; };
        struct { T s, t; };
        struct { T u, v; };

        T a_[N]{};
    };
};

template <typename T>
struct base_array_<T, 3>
{
    enum { N = 3 };
    union
    {
        struct { T x, y, z; };
        struct { T r, g, b; };

        T a_[N]{};
    };
};

template <typename T>
struct base_array_<T, 4>
{
    enum { N = 4 };
    union
    {
        struct { T x, y, z, w; };
        struct { T r, g, b, a; };
        //struct { array3 xyz; };
        //struct { array2 xy; array2 zw; };

        T a_[N]{};
    };
};

#pragma endregion

#pragma region interface

template <typename T, size_t N>
struct array : base_array_<T, N>, sequence<array<T, N>>
{
    // TODO: fay::array<0> a;
    static_assert(N > 0);

public:
    using size_type              = size_t;
    using difference_type        = ptrdiff_t;
    using value_type             = T;
    using reference              = value_type&;
    using const_reference        = const value_type&;
    using pointer                = value_type*;
    using const_pointer          = const value_type*;
    using iterator               = value_type*;
    using const_iterator         = const value_type*;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

public:
    // TODO: make fay::array to be a aggregate

    // WARNING!!! array(n) != array{ n }
    // TODO£ºremove£¿£¿£¿
    constexpr explicit array(std::initializer_list<value_type> il) /*: a_{ il }*/
    {
        DCHECK(il.size() <= N);
        auto p = a_; auto q = il.begin(); // The rest is defalut value
        for (; q != il.end(); ++p, ++q)
            *p = *q;
    }
    constexpr explicit array(const_reference v) { fill(v); }
    constexpr explicit array(const_pointer   p) { for (auto& e : a_) e = *p++; }

    // set
    constexpr void fill(const_reference v) { std::fill_n(a_, N, v); }

    // get
    reference       operator[](size_type i)       noexcept { return a_[i]; }
    const_reference operator[](size_type i) const noexcept { return a_[i]; }
    /*
    reference       at(size_type i)       { if (i >= N) throw std::out_of_range("fay::array::at"); return a_[i]; }
    const_reference at(size_type i) const { if (i >= N) throw std::out_of_range("fay::array::at"); return a_[i]; }

    reference       front()       noexcept { return a_[0]; }
    const_reference front() const noexcept { return a_[0]; }
    reference       back()        noexcept { return a_[N - 1]; }
    const_reference back()  const noexcept { return a_[N - 1]; }
    */
    pointer         data()       noexcept { return a_; }
    const_pointer   data() const noexcept { return a_; }

    // capaciiity
    constexpr size_type    size()    const noexcept { return N; }
    constexpr bool         emptry()  const noexcept { return false; }
    /*
    // iterators
    iterator               begin()         noexcept { return       iterator(data()); }
    const_iterator         begin()   const noexcept { return const_iterator(data()); }
    iterator               end()           noexcept { return       iterator(data() + N); }
    const_iterator         end()     const noexcept { return const_iterator(data() + N); }

    reverse_iterator       rbegin()        noexcept { return       reverse_iterator(end()); }
    const_reverse_iterator rbegin()  const noexcept { return const_reverse_iterator(end()); }
    reverse_iterator       rend()          noexcept { return       reverse_iterator(begin()); }
    const_reverse_iterator rend()    const noexcept { return const_reverse_iterator(begin()); }

    const_iterator         cbegin()  const noexcept { return begin(); }
    const_iterator         cend()    const noexcept { return end(); }
    const_reverse_iterator crbegin() const noexcept { return rbegin(); }
    const_reverse_iterator crend()   const noexcept { return rend(); }
    */
};
#pragma endregion

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
    size_type sz_;
    std::unique_ptr<T[]> a_;
};

#pragma endregion

} // namespace fay