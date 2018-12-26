#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_CORE_FAY_H
#define FAY_CORE_FAY_H

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

namespace fay
{



} // namespace fay

#endif // FAY_CORE_FAY_H
