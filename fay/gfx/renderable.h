#pragma once

#include "fay/core/fay.h"
#include "fay/math/geometry.h"
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
        return desc_;
    }

    //void set_bounds(bounds3 bounds) { bounds_ = bounds; }
    //bounds3 bounds() const { return bounds_; }

private:
    renderable_desc desc_{};
    //bounds3 bounds_{ 0.f, 0.f };
};

FAY_SHARED_PTR(renderable)

} // namespace fay
  