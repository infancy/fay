#pragma once

#include "fay/core/fay.h"
#include "fay/math/bounds.h"
#include "fay/render/define.h"

namespace fay
{

// an object that can be drawn
// renderable = mesh_ptr + material_ptr + effect_ptr

struct renderable_desc // common render info, mainly used in render queue;
{
    uint64_t sort_key{};
};

class renderable
{
public:
    virtual void render(command_list& cmd) = 0;

    renderable_desc desc()
    {
        return renderable_desc_;
    }

private:
    renderable_desc renderable_desc_{};
};

using renderable_sptr = std::shared_ptr<renderable>;

} // namespace fay
  