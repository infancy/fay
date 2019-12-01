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

    constexpr explicit array(const_reference v) { fill(v); }

    //! WARNING!!! array(n) != array{ n }
    constexpr explicit array(std::initializer_list<value_type> il) : array(il.begin(), il.size())
    {
        if (il.size() > N) 
            throw std::out_of_range("fay::array::array(std::initializer_list<value_type> il): il is too long");
    }

    //! Sequence must have 'begin' and 'size' method
    template<typename Sequence>
    constexpr explicit array(const Sequence& c) : array(std::cbegin(c), std::size(c)) {}
    template<typename Sequence>
    constexpr explicit array(const Sequence& c, size_type n) : array(std::cbegin(c), n) { DCHECK(n <= std::size(c)); }

private:
    // TODO: wait for C++20
    //! if n >= N, all elems are init by [*p, *(p+N))
    //! if n <  N, the front elems are same, the rest are defalut value
    constexpr explicit array(const_pointer p, size_type n) 
    {
        if (n <= 0)
            throw std::invalid_argument("fay::array::array(const_pointer p, size_type n): para 'n' isn't positive, is it used 'array a{}' ???");

        std::copy(p, p + std::min(n, N), a_); // TODO: deque and list
    }

public:
    // set
    constexpr void fill(const_reference v) { std::fill_n(a_, N, v); }

    // get
    reference       operator[](size_type i)       { return a_[i]; }
    const_reference operator[](size_type i) const { return a_[i]; }

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