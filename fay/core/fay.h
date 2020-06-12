#pragma once

#include "fay/core/platform.h"
#include "fay/core/define.h"

// Global Include Files
#include <cinttypes>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>

// TODO: eastl
#include <algorithm>
#include <array>
#include <concepts>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <numeric>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

// #include <boost/noncopyable.hpp>
#include <glog/logging.h>
#define FAY_CHECK CHECK
#define FAY_DCHECK DCHECK
#define FAY_LOG LOG
#define FAY_DLOG DLOG

#if defined(FAY_IN_MSVC)
#pragma warning(disable : 4305)  // double constant assigned to float
#pragma warning(disable : 4244)  // int -> float conversion
#pragma warning(disable : 4843)  // double -> float conversion
#pragma warning(disable : 4201)  // nonstandard extension used : nameless struct/union
#endif

#define FAY_UNIQUE_PTR( type ) \
using type##_ptr       = std::unique_ptr<type>; \
using const_##type##_ptr = std::unique_ptr<const type>;

#define FAY_SHARED_PTR( type ) \
using type##_sp = std::shared_ptr<type>; \
using type##_wp = std::weak_ptr<type>; \
using const_##type##_sp = std::shared_ptr<const type>; \
using const_##type##_wp = std::weak_ptr<const type>;

namespace fay
{

// try to avoid implicit behavior, express expected behavior explicitly

// ???
using std::begin;
using std::cbegin;

using uchar  = uint8_t;
using ushort = uint16_t;
using uint   = uint32_t;
using ulong  = uint32_t;
using ullong = uint64_t;

using string = std::string;
using string_view = std::string_view;
using namespace std::string_literals;


static_assert(std::is_same_v<size_t, unsigned long long int>);
static_assert(std::is_same_v<size_t, uint64_t>);

// TODO: Compatible with future standards "std::size"
// not named as "size_t", in order not to confuse with "std::size_t"
class size
{
public:
    constexpr explicit size(const size_t sz) : sz_{ sz } 
    {
        DLOG_IF(ERROR, sz == 0);
        DLOG_IF(ERROR, sz > (uint32_t)-1);
    }

    constexpr size_t value() const noexcept { return sz_; }
    constexpr operator size_t() const noexcept { return sz_; }

private:
    size_t sz_{};
};

// https://zh.cppreference.com/w/cpp/language/user_literal
// constexpr size operator "" _sz(size_t sz) { return size(sz); }
constexpr size operator "" sz(size_t sz) { return size(sz); }

// static_assert(sizeof(int) >= 4,    "byte size of int should big than 4");
// static_assert(sizeof(long) >= 4);

// static_assert(sizeof(ulong) >= 8, "byte size of ullong should big than 8");
// static_assert(sizeof(ullong) >= 16, "byte size of ullong should big than 16");

// template<typename T>
// using vector<T> = std::vector<T>;
// template<typename T>
// using map = std::unordered_map;

} // namespace fay
