/* readme

# intro

# usage

# class

# reference
    * https://github.com/mitsuba-renderer/enoki/blob/master/include/enoki/array_base.h
    * https://zhuanlan.zhihu.com/p/96089089
*/



#pragma once

#include <fay/core/fay.h>

namespace fay
{

// TODO: constexpr flat_map

template<typename T>
concept iterable_c = requires(T&& t)
{
    t.begin();
    t.end();
};

//template<typename T>
//concept Container = std::range;

//! mixin by inherit it
template<iterable_c Derived> // template<Container C>
struct mixin_container
{
public:
    auto rbegin()        noexcept { return reverse<decltype(derived()->end())>  (derived()->end());   }
    auto rbegin()  const noexcept { return reverse<decltype(derived()->end())>  (derived()->end());   }
    auto rend()          noexcept { return reverse<decltype(derived()->begin())>(derived()->begin()); }
    auto rend()    const noexcept { return reverse<decltype(derived()->begin())>(derived()->begin()); }

    auto cbegin()  const noexcept { return derived()->begin();  }
    auto crbegin() const noexcept { return derived()->rbegin(); }
    auto cend()    const noexcept { return derived()->end();    }
    auto crend()   const noexcept { return derived()->rend();   }

    auto& front()       noexcept { return *(derived()->begin()); }
    auto& front() const noexcept { return *(derived()->begin()); }
    auto& back()        noexcept { return *(derived()->end() - 1); }
    auto& back()  const noexcept { return *(derived()->end() - 1); }

private:
    template<typename T>
    using reverse = std::reverse_iterator<T>; // using iterator_category = typename iterator_traits<_BidIt>::iterator_category; ???

          Derived* derived()       noexcept { return (Derived*)this; }
    const Derived* derived() const noexcept { return (Derived*)this; }
};



template<typename Derived> // template<Associative Seq>
struct mixin_associative : mixin_container<mixin_associative<Derived>>
{

};



#define FAY_SEQUENCE_TYPE_ALIAS                                 \
using size_type              = fay::size;                       \
using difference_type        = ptrdiff_t;                       \
using value_type             = T;                               \
using reference              = value_type&;                     \
using const_reference        = const value_type&;               \
using pointer                = value_type*;                     \
using const_pointer          = const value_type*;               \
using iterator               = value_type*;                     \
using const_iterator         = const value_type*;               \
using reverse_iterator       = std::reverse_iterator<iterator>; \
using const_reverse_iterator = std::reverse_iterator<const_iterator>;

template<typename T>
concept sequence_c = requires(T&& t, fay::size index)
{
    t.data();
    t.size();
    t.operator[](index);
};

// array, vector, deque, list...
// template<typename Derived>
// struct mixin_sequence : container_mixin<mixin_sequence<Derived>>
template<typename T, sequence_c Derived> // template<Sequence Seq>
struct mixin_sequence : mixin_container<mixin_sequence<T, Derived>>
{
public:
    FAY_SEQUENCE_TYPE_ALIAS

/*
public:
    constexpr explicit mixin_sequence(const_pointer first, const_pointer last)
    {
        auto diff = std::distance(first, last);
        DCHECK(diff > 0) << "range isn't valid, is it used 'tensor t{}' ???";
        DCHECK(diff <= N);

        std::copy(first, last, derived()->data()); // TODO: deque and list
    }
*/

public:
    auto begin()       noexcept { return derived()->data(); }
    auto begin() const noexcept { return derived()->data(); }
    auto end()         noexcept { return derived()->data() + derived()->size(); }
    auto end()   const noexcept { return derived()->data() + derived()->size(); }

    // TODO: derived name
    auto& at(size_type i)       { check_at(i); return derived()->operator[](i); }
    auto& at(size_type i) const { check_at(i); return derived()->operator[](i); }

private:
    //using Derived = Derived_;

    void check_at(size_type i) { if (i >= derived()->size()) throw std::out_of_range("fay::mixin_sequence::check_at"); }

          Derived* derived()       noexcept { return (Derived*)this; }
    const Derived* derived() const noexcept { return (Derived*)this; }
};

} // namespace fay