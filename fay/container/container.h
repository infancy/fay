#pragma once

#include <fay/core/fay.h>

namespace fay
{

// https://github.com/mitsuba-renderer/enoki/blob/master/include/enoki/array_base.h
// https://github.com/foonathan/array

// TODO: constexpr flat_map

/*
template<typename T>
concept Container = requires(T&& t) 
{
    // ...
};

template<typename T>
concept Container = std::Range
*/

template<typename Derived> // template<Container C>
struct container
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
struct associative : container<associative<Derived>>
{

};



#define FAY_SEQUENCE_tYPE_ALIAS                                 \
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

// array, vector, deque, list...
// template<typename Derived>
// struct sequence : container<sequence<Derived>>
template<typename T, typename Derived> // template<Sequence Seq>
struct sequence : container<sequence<T, Derived>>
{
public:
    FAY_SEQUENCE_tYPE_ALIAS

/*
public:
    constexpr explicit sequence(const_pointer first, const_pointer last)
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

    void check_at(size_type i) { if (i >= derived()->size()) throw std::out_of_range("fay::sequence::check_at"); }

          Derived* derived()       noexcept { return (Derived*)this; }
    const Derived* derived() const noexcept { return (Derived*)this; }
};

} // namespace fay