#pragma once

#include "fay/math/geometry.h"
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

// All in assimp, not glTF???
// glTF???: "meshes" -> "models", "primitives" -> "meshes"

struct resource_scene
{
    std::string name;

    std::vector<size_t> nodes;
};

struct resource_node
{
    std::string name{};
    glm::mat4 transform{ 1.f };

    std::vector<size_t> meshes{}; // submesh in glTF

    std::vector<size_t> children{};
};

struct resource_material
{
    std::string name{};

    image base_color; // use alpha channel as metal revealed.
    image metallic_roughness; // metallic saved in blue channel, roughness saved in green channel.
    image normal;
    // image height;
    image occlusion;
    image emissive;

    glm::vec4 base_factor{ 1.f };
    float metallic_factor = 1.f;
    float roughness_factor = 1.f;

    float normal_factor = 1.f;
    glm::vec3 emissive_factor{ 0.f };
    //DrawPipeline pipeline = DrawPipeline::Opaque;
    bool transparent = false;
    bool two_sided = true; // if have alpha channel, or ...
    bool bandlimited_pixel = true; // if is HDR, false
};

struct resource_mesh
{
    resource_mesh() = default;

    std::string name{};

    // memory_block, buffer_block
    std::vector<uint8_t> vertices;
    uint32_t      size{}; // vertex nums
    vertex_layout layout{};

    std::vector<uint32_t> indices;
    primitive_type primitive_{ primitive_type::triangles };

    // material
    uint32_t material_index{};
    bool has_material{ true };

    bounds3 bounds_;
};

} // namespace fay