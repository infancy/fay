#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_CORE_FAY_H
#define FAY_CORE_FAY_H

#include "fay/core/config.h"

// Global Include Files
#include <cinttypes>
#include <cmath>
#include <cstring>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

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

enum class third_party { none, gl, dx, vk };

// graphics --------------------------------------------------------------------

enum class model_type { obj, fbx, gltf, blend, unknown };

enum class texture_type { none, diffuse, specular, ambient, emissive, height, alpha, parallax,
	normal, shininess, opacity, displace, lightmap, reflection, cubemap, shadowmap, unknown };

} // namespace fay

#endif //FAY_CORE_FAY_H
