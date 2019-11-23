#pragma once

#include <fay/core/fay.h>

namespace fay
{

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
    //using const_value_type       = const typename Derived::valye_type;
    //using const_pointer          = const typename Derived_::pointer;
    //using cosnt_reference        = const typename Derived_::reference;
    //using const_iterator         = const typename Derived_::iterator;
    //using const_reverse_iterator = const typename Derived_::reverse_iterator;

public:
    auto rbegin()        noexcept { return reverse<decltype(derived().end())>(derived().end());     } // ->std::reverse_iterator<decltyp(auto)>
    auto rbegin()  const noexcept { return reverse<decltype(derived().end())>(derived().end());     }
    auto rend()          noexcept { return reverse<decltype(derived().begin())>(derived().begin()); }
    auto rend()    const noexcept { return reverse<decltype(derived().begin())>(derived().begin()); }

    auto cbegin()  const noexcept { return derived().begin();  }
    auto crbegin() const noexcept { return derived().rbegin(); }
    auto cend()    const noexcept { return derived().end();    }
    auto crend()   const noexcept { return derived().rend();   }

    //auto at() . . . { . . . return operator[](i); }
    // operator[]

    auto front()       noexcept { return *derived().begin(); }
    auto front() const noexcept { return *derived().begin(); }
    auto back()        noexcept { return *derived().end();   }
    auto back()  const noexcept { return *derived().end();   }

private:
    //using Derived = Derived_;
    template<typename T>
    using reverse = std::reverse_iterator<T>; // using iterator_category = typename iterator_traits<_BidIt>::iterator_category; ???

          Derived& derived()       noexcept { return (Derived&)*this; }
    const Derived& derived() const noexcept { return (Derived&)*this; }
};

template<typename Derived> // template<Associative Seq>
struct associative : container<associative<Derived>>
{

};

template<typename Derived> // template<Sequence Seq>
struct sequence : container<sequence<Derived>>
{
public:
    auto begin()       noexcept { return derived().data(); }
    auto begin() const noexcept { return derived().data(); }
    auto end()         noexcept { return derived().data() + derived().size(); }
    auto end()   const noexcept { return derived().data() + derived().size(); }

private:
    //using Derived = Derived_;

          Derived& derived()       noexcept { return (Derived&)*this; }
    const Derived& derived() const noexcept { return (Derived&)*this; }
};

} // namespace fay