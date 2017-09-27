#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef FAY_RAY_RAYTRACING_H
#define FAY_RAY_RAYTRACING_H

#include "fay.h"

void ray_cast();

namespace fay
{

int ray_cast_gui();

}	// namespace fay

#endif //FAY_RAY_RAYTRACING_H