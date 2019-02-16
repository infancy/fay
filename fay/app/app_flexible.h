#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "fay/app/app.h"
#include "fay/core/config.h"
#include "fay/core/fay.h"
#include "fay/gfx/camera.h"
#include "fay/gfx/light.h"
#include "fay/gfx/mesh.h"
#include "fay/render/device.h"
#include "fay/render/shader.h"
#include "fay/resource/image.h"
#include "fay/resource/model.h"

namespace fay
{

using buffer_sp = std::shared_ptr<buffer_id>;
inline buffer_sp create_buffer_sp(render_device* device, const buffer_desc desc)
{
    auto id = device->create(desc);
    return buffer_sp(new buffer_id(id), [device](const buffer_id* ptr) { device->destroy(*ptr); delete ptr; });
}

#define glcheck_errors() CHECK(glGetError() == GL_NO_ERROR)



// -------------------------------------------------------------------------------------------------

// inline app_desc global_desc;

template<typename Box>
inline renderable_sp create_box_mesh(const Box& box, render_device* device)
//inline renderable_sp create_box_mesh(const bounds3& box, render_device* device)
{
    resource_mesh mesh;
    mesh.size = 8;
    mesh.layout = vertex_layout{
        {fay::attribute_usage::position,  fay::attribute_format::float3}
    };

    std::vector<glm::vec3> box_mesh
    {
        box.corner(box_corner::I),
        box.corner(box_corner::II),
        box.corner(box_corner::III),
        box.corner(box_corner::IV),
        box.corner(box_corner::V),
        box.corner(box_corner::VI),
        box.corner(box_corner::VII),
        box.corner(box_corner::VIII),
    };

    size_t byte_size = mesh.size * mesh.layout.stride();
    mesh.vertices.reserve(byte_size);
    mesh.vertices.resize(byte_size);
    std::memcpy(mesh.vertices.data(), box_mesh.data(), byte_size);

    mesh.indices = 
    { 
        0, 1, 1, 2, 2, 3, 3, 0,
        4, 5, 5, 6, 6, 7, 7, 4,
        0, 4, 1, 5, 2, 6, 3, 7
    };
    mesh.primitive_ = primitive_type::lines;

    return std::make_shared<raw_mesh>(device, mesh);
}

inline renderable_sp create_raw_renderable(const std::string& model_path, render_device* device)
{
    auto model = create_resource_model(model_path, device->type());

    std::vector<renderable_sp> mesh_list;

    for (const auto& mesh : model->meshes())
    {
        mesh_list.emplace_back(std::make_shared<raw_mesh>(device, mesh));
    }

    return std::make_shared<array_mesh>(mesh_list);
}

inline std::vector<renderable_sp> create_renderables(const resource_model& model, render_device* device)
{
    std::vector<material_sp> material_list;
    std::vector<renderable_sp> mesh_list;

    for (const auto& mat : model.materials())
    {
        material_list.emplace_back(std::make_shared<material>(device, mat));
    }

    for (const auto& mesh : model.meshes())
    {
        mesh_list.emplace_back(std::make_shared<static_mesh>(device, mesh, material_list[mesh.material_index]));
    }

    return mesh_list;
}

inline std::vector<renderable_sp> create_renderables(const std::string& model_path, render_device* device)
{
    auto model = create_resource_model(model_path, device->type());
    return create_renderables(*model, device);
}

inline renderable_sp create_renderable(const std::string& model_path, render_device* device)
{
    auto meshes = create_renderables(model_path, device);
    return std::make_shared<array_mesh>(meshes);
}

// -------------------------------------------------------------------------------------------------

inline texture_id create_2d(render_device_ptr& device, const std::string& name, const image& img)
{
    texture_desc desc;

    desc.name = name;
    desc.width = img.width();
    desc.height = img.height();
    desc.pixel_format = img.format();

    desc.size = img.size() * img.channel();
    desc.data = { img.data() };

    desc.type = texture_type::two;

    return device->create(desc);
}

// -------------------------------------------------------------------------------------------------

inline std::tuple<frame_id, texture_id, texture_id> create_depth_frame(const std::string& name, size_t width, size_t height, render_device* device)
{
    texture_desc desc;

    desc.name = name;
    desc.width = width;
    desc.height = height;
    desc.data = { nullptr };
    desc.type = texture_type::two;

    desc.min_filter = filter_mode::nearest;
    desc.max_filter = filter_mode::nearest;
    desc.wrap_u = wrap_mode::repeat;
    desc.wrap_v = wrap_mode::repeat;
    desc.mipmap = false;

    desc.as_render_target = render_target::color;
    desc.pixel_format = pixel_format::rgba32f; // for debug
    desc.size = width * height * 16; // byte size
    auto color_id = device->create(desc);

    desc.as_render_target = render_target::depth;
    desc.pixel_format = pixel_format::r32f; // float
    desc.size = width * height * 4; // byte size
    auto ds_id = device->create(desc);

    frame_desc fd;
    fd.name = name;
    fd.width = width;
    fd.height = height;

    // TODO: depth frame doesn't need color frame
    fd.render_targets = { { color_id, 0, 0 } };
    fd.depth_stencil = { ds_id, 0, 0 };
    auto frm_id = device->create(fd);

    return { frm_id, color_id, ds_id };
}

inline std::tuple<frame_id, texture_id, texture_id> create_frame(render_device* device, const std::string& name, size_t width, size_t height)
{
    texture_desc desc;

    fay::image img("texture/container2.png", true);

    desc.name = name;
    desc.width = width;
    desc.height = height;
    desc.size = width * height * 4; // byte size
    desc.data = { img.data() };
    desc.type = texture_type::two;
    desc.mipmap = false;

    desc.as_render_target = render_target::color;
    desc.pixel_format = pixel_format::rgba8;
    auto color_id = device->create(desc);

    desc.as_render_target = render_target::depth_stencil;
    desc.pixel_format = pixel_format::depthstencil; // TODO: depth_stencil;
    auto ds_id = device->create(desc);

    frame_desc fd;
    fd.name = name;
    fd.width = width;
    fd.height = height;
    fd.render_targets = { { color_id, 0, 0 } };
    fd.depth_stencil = { ds_id, 0, 0 };
    auto frm_id = device->create(fd);

    return { frm_id, color_id, ds_id };
}

inline std::tuple<frame_id, texture_id, texture_id, texture_id, texture_id> create_Gbuffer(render_device* device, const std::string& name, size_t width, size_t height)
{
    texture_desc desc;

    fay::image img("texture/container2.png", true);

    desc.name = name;
    desc.width = width;
    desc.height = height;
    desc.size = width * height * 4; // byte size
    desc.data = { nullptr };
    desc.type = texture_type::two;

    desc.min_filter = filter_mode::nearest;
    desc.max_filter = filter_mode::nearest;
    desc.wrap_u = wrap_mode::repeat;
    desc.wrap_v = wrap_mode::repeat;
    desc.mipmap = false;

    desc.as_render_target = render_target::color;
    desc.pixel_format = pixel_format::rgb32f;
    auto color_id = device->create(desc);

    desc.as_render_target = render_target::color;
    desc.pixel_format = pixel_format::rgb32f;
    auto color_id2 = device->create(desc);

    desc.as_render_target = render_target::color;
    desc.pixel_format = pixel_format::rgba8;
    auto color_id3 = device->create(desc);

    desc.as_render_target = render_target::depth_stencil;
    desc.pixel_format = pixel_format::depthstencil; // TODO: depth_stencil;
    auto ds_id = device->create(desc);

    frame_desc fd;
    fd.name = name;
    fd.width = width;
    fd.height = height;
    fd.render_targets = { { color_id, 0, 0 }, { color_id2, 0, 0 }, { color_id3, 0, 0 } };
    fd.depth_stencil = { ds_id, 0, 0 };
    auto frm_id = device->create(fd);

    return { frm_id, color_id, color_id2, color_id3, ds_id };
}

// -------------------------------------------------------------------------------------------------
// IO

const inline std::string Nier_2b           = "model/Nier_2b/2b.obj";
const inline std::string Box               = "model/box/box.obj";
const inline std::string Blocks            = "model/blocks/blocks.obj";
const inline std::string Rei               = "model/Rei/Rei.obj";
const inline std::string CornellBox        = "model/CornellBox/CornellBox.obj";
const inline std::string Planet            = "model/planet/planet.obj";
const inline std::string Rock              = "model/rock/rock.obj";
const inline std::string Fairy             = "model/fairy/fairy.obj";
const inline std::string Nanosuit          = "model/nanosuit/nanosuit.obj";
const inline std::string silly_dancing     = "model/silly_dancing.fbx";
const inline std::string nierautomata_2b   = "model/nierautomata_2b/scene.gltf";
const inline std::string ftm_sketchfab     = "model/ftm/ftm_sketchfab.blend";
const inline std::string Nier_2b_ik_rigged = "model/Nier_2b_ik_rigged/scene.gltf";
const inline std::string Sponza            = "model/sponza/sponza.obj";
const inline std::string LightBulb         = "model/LightBulb/LightBulb.obj";
//const inline std::string shadows           = "model/shadows/shadows.obj";

struct render_paras
{
    glm::vec4 a{};
    glm::vec4 b{};
};

} // namespace fay