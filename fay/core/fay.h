#pragma once

#include "fay/core/platform.h"
#include "fay/core/define.h"

// Global Include Files
#include <cinttypes>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <array>
#include <functional>
#include <limits>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <utility>

// #include <boost/noncopyable.hpp>
#include <glog/logging.h>

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

using std::begin;
using std::cbegin;

using uchar  = uint8_t;
using uint   = uint32_t;
using string = std::string;
// static_assert(sizeof(int) >= 4, "byte size of int should big than 4");
// template<typename T>
// using vector<T> = std::vector<T>;
// template<typename T>
// using map = std::unordered_map;

} // namespace fay
