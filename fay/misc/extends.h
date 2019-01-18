#pragma once

#include "fay/core/fay.h"

namespace fay
{

// vector, array, list, deque ...
template<typename T>
class extends_sequence
{
public:
    using value_type = typename T::value_type;
    using pointer = typename T::pointer;
    using const_pointer = typename T::const_pointer;
    using reference = typename T::reference;
    using const_reference = typename T::const_reference;
    using size_type = typename T::size_type;
    using iterator = typename T::iterator;
    using const_iterator = typename T::const_iterator;
    using reverse_iterator = typename T::reverse_iterator;
    using const_reverse_iterator = typename T::const_reverse_iterator;

    constexpr extends_sequence() = default;
    constexpr explicit extends_sequence(std::initializer_list<value_type> il) : list_(il) {}

    //constexpr explicit extends_sequence(const value_type& s) : list_(s) {}
    //constexpr explicit extends_sequence(const value_type* p)  { for (auto& e : list_) e = *p++; }

    iterator               begin()         noexcept { return list_.begin(); }
    const_iterator         begin()   const noexcept { return list_.begin(); }
    iterator               end()           noexcept { return list_.end(); }
    const_iterator         end()     const noexcept { return list_.end(); }

    reverse_iterator       rbegin()        noexcept { return list_.rbegin(); }
    const_reverse_iterator rbegin()  const noexcept { return list_.rbegin(); }
    reverse_iterator       rend()          noexcept { return list_.rend(); }
    const_reverse_iterator rend()    const noexcept { return list_.rend(); }

    const_iterator         cbegin()  const noexcept { return list_.cbegin(); }
    const_iterator         cend()    const noexcept { return list_.cend(); }
    const_reverse_iterator crbegin() const noexcept { return list_.crbegin(); }
    const_reverse_iterator crend()   const noexcept { return list_.crend(); }

    size_type size() const { return list_.size(); }

    reference       operator[](size_t i) { return list_[i]; }
    const_reference operator[](size_t i) const { return list_[i]; }


protected:
    T list_;
};

} // namespace fay
