#pragma once

#include "fay/core/ecs.h"
#include "fay/core/fay.h"
#include "fay/gfx/renderable.h"

namespace fay
{

// TODO: https://stackoverflow.com/questions/81870/is-it-possible-to-print-a-variables-type-in-standard-c

// remove
struct AComponent : component
{
    FAY_COMPONENT_TYPE_HASH(AComponent)

        AComponent(int v)
        : v(v)
    {
    }
    int v;
};

struct transform_component : component
{
    FAY_COMPONENT_TYPE_HASH(transform_component)

    glm::mat4 transform { 1.f };
};

struct renderable_component : component
{
    FAY_COMPONENT_TYPE_HASH(renderable_component)

    // TODO: replace sp with raw_ptr???
    renderable_sp renderable{};
};

} // namespace fay