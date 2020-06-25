#pragma once

// #include <fay/core/fay.h>
#include <fay/container/container.h>


#pragma region reference

// https://github.com/foonathan/array
// https://github.com/abseil/abseil-cpp/blob/master/absl/container/fixed_array.h

#pragma endregion reference


namespace fay
{

#pragma region static_array/stack_array

template <typename T, size_t N>
class array : public sequence<T, array<T, N>>
{
#pragma region value
    static_assert(N > 0);

public:
    using this_type = array<T, N>;
    FAY_SEQUENCE_tYPE_ALIAS

public:
    // ctor
    constexpr array() = default;

    // WARNING: never provide dangerous interfaces without "unsafe", "uncheck", "ambiguous"
    constexpr explicit array(std::initializer_list<value_type> il) : array(il.begin(), il.end()) 
    { 
        DCHECK(il.size() <= sz_);
        if(il.size() == 1) LOG(WARNING) << "shouldn't use 'array a{ n }' style ctor";
    }

    // not use 'array a(fill_value)'
    // constexpr explicit array(const_reference v) { fill(v); }

    //! Sequence must have 'begin' and 'end' method
    template<typename Sequence>
    constexpr explicit array(const Sequence& c) : array(cbegin(c), cbegin(c) + std::min(std::size(c), sz_)) {}

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
        DCHECK(diff <= sz_);

        std::copy(first, last, a_); // TODO: deque and list
    }

public:
    // set
    constexpr void fill(const_reference v) { std::fill_n(a_, sz_, v); }

    // get
    reference       operator[](size_type i)       { return a_[i]; }
    const_reference operator[](size_type i) const { return a_[i]; }

    pointer         data()       noexcept { return a_; }
    const_pointer   data() const noexcept { return a_; }

    // capaciiity
    constexpr size_type size()  const noexcept { return sz_; }
    constexpr bool      empty() const noexcept { return false; }

private:
    // because in the beginning provides user-define ctor, fay::array is not a aggregate anymore
    enum { sz_ = N };
    T a_[N]{};

#pragma endregion

#pragma region static

public:
    static void fill();

#pragma endregion
};

#pragma endregion



#pragma region dynamic_array/heap_array

// fay::heap_array<T> is a variable-length arrays(VLA)
// TODO: fay::heap_array<T> is a fay::container(concept)
// WARNING: when use fay::container, always use 'fay::' to avoid confusion with standard container
// TODO: heap_value

template<typename T>
/*TODO: constexpr */class heap_array : public sequence<T, heap_array<T>>
{
public:
    using this_type = heap_array<T>;
    FAY_SEQUENCE_tYPE_ALIAS

public:
    explicit heap_array(fay::size size, value_type value = value_type{}) :
        sz_{ size },
        a_{ std::make_unique<T[]>(sz_) }
    {
        DCHECK(sz_ > 0);

        fill(value);
    }

    explicit heap_array(fay::size size, std::initializer_list<value_type> il) :
        sz_{ size },
        a_{ std::make_unique<T[]>(sz_) }
    {
        DCHECK(sz_ > 0);
        DCHECK(sz_ >= il.size());

        std::copy(il.begin(), il.end(), a_.get());
    }

    explicit heap_array(const_pointer first, const_pointer last) :
        sz_{ std::distance(first, last) },
        a_{ std::make_unique<T[]>(sz_) }
    {
        DCHECK(sz_ > 0);

        std::copy(first, last, a_.get());
    }

    // defined in template unieque<T[]>
    //this_type(const this_type&) = delete;
    //this_type& operator=(const this_type&) = delete;

public:
    // set
    void fill(const_reference v) { std::fill_n(a_.get(), sz_, v); }

    // get
    this_type clone() { return this_type(a_.get(), a_.get() + sz_); }

    reference       operator[](size_type i)       noexcept { return a_[i]; }
    const_reference operator[](size_type i) const noexcept { return a_[i]; }

    pointer         data()       noexcept { return a_.get(); }
    const_pointer   data() const noexcept { return a_.get(); }

    // capaciiity
    size_type size()  const noexcept { return sz_; }
    bool      empty() const noexcept { return false; }

private:
    size_type sz_;
    std::unique_ptr<T[]> a_;
};

#pragma endregion

} // namespace fay