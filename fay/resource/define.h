#pragma once

#include "fay/math/bounds.h"
#include "fay/render/define.h"
#include "fay/resource/image.h"

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

// -------------------------------------------------------------------------------------------------

// TODO: serialization
// camera_desc

struct resource_material
{
    std::string name{};

    image base_color;
    image normal;
    image metallic_roughness;
    image occlusion;
    image emissive;

    glm::vec4 uniform_base_color{ 1.f };
    glm::vec3 uniform_emissive_color{ 0.f };
    float uniform_metallic = 1.f;
    float uniform_roughness = 1.f;
    float normal_scale = 1.f;
    //DrawPipeline pipeline = DrawPipeline::Opaque;
    bool two_sided = true; // if have alpha channel, or ...
    bool bandlimited_pixel = true; // if is HDR, false
};

struct resource_mesh
{
    std::string name{};

    // memory_block, buffer_block
    std::vector<uint8_t> vertices;
    uint32_t      size{}; // vertex nums
    vertex_layout layout;

    std::vector<uint32_t> indices;
    primitive_type primitive_{ primitive_type::triangles };

    // material
    uint32_t material_index{};
    bool has_material{ true };

    bounds3 bounds_;
};

struct resource_node
{
    std::vector<uint32_t> meshes;
    glm::mat4 transform{ 1.f };

    std::vector<uint32_t> children;
};

} // namespace fay