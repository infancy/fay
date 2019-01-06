#pragma once

#include "fay/core/fay.h"
#include "fay/gfx/renderable.h"


namespace fay
{

/*
struct mesh
{
    transform_component transform_;
    renderable_component renderable_;
    physical_component physical_;
};

*/

struct scene_node
{
    std::string name;
    std::vector<uint32_t> node_indices;
};

class scene
{
public:

private:
    scene_node root_node_;
};

} // namespace fay
