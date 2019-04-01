#pragma once

#include "fay/core/fay.h"
#include "fay/render/define.h"

namespace fay
{

class frame
{
public:
    frame() {}
    // TODO: std::vector<texture_id> color_ids -> folly::string. 1. fixed-length storge in stack 2. variable-length in heap
    frame(frame_id frm_id, texture_id color_id, texture_id ds_id)
        : frm_{ frm_id }
        , colors_{ color_id }
        , ds_{ ds_id }
    {
    }
    frame(frame_id frm_id, std::vector<texture_id> color_ids, texture_id ds_id)
        : frm_{ frm_id }
        , colors_{ std::move(color_ids) }
        , ds_{ ds_id }
    {
    }

    frame_id view() const { return frm_; }

    texture_id rtv(uint i) const { return colors_[i]; }
    texture_id operator[](uint i) const { return rtv(i); }

    texture_id dsv() const { return ds_; }

    /*
    static ...
    */

private:
    frame_id frm_;

    std::vector<texture_id> colors_;
    texture_id ds_;
};

} // namespace fay
