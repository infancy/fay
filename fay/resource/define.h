#pragma once

#include "fay/core/fay.h"

namespace fay
{

enum class resource_location
{
    none,
    buildin,
    local,

    // remote
    https,
};

} // namespace fay