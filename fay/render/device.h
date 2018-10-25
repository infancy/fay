#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_RENDER_DEVICE_H
#define FAY_RENDER_DEVICE_H

#include "fay/core/fay.h"

namespace fay::render
{

class render_device
{

};

using render_device_ptr = std::unique_ptr<render_device>;

} // namespace fay

#endif // FAY_RENDER_DEVICE_H