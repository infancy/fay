#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "fay/app/app.h"
#include "fay/core/config.h"
#include "fay/core/fay.h"
#include "fay/gfx/camera.h"
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

// inline app_desc global_desc;

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

inline renderable_sp create_single_renderable(const std::string& model_path, render_device* device)
{
    auto meshes = create_renderables(model_path, device);
    std::make_shared<array_mesh>(meshes);
}



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

} // namespace fay