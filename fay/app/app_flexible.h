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

inline renderable_sp create_single_renderable(const std::string& model_path, render_device* device)
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

inline std::tuple<frame_id, texture_id, texture_id> create_depth_frame(render_device* device, const std::string& name, size_t width, size_t height)
{
    texture_desc desc;

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

    desc.as_render_target = render_target::color;
    desc.pixel_format = pixel_format::rgba8;
    auto color_id = device->create(desc);

    desc.as_render_target = render_target::depth;
    desc.pixel_format = pixel_format::r32f; // float
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

const inline std::string Nier_2b = "object/Nier_2b/2b.obj";
const inline std::string Box = "object/box/box.obj";
const inline std::string Blocks = "object/blocks/blocks.obj";
const inline std::string Rei = "object/Rei/Rei.obj";
const inline std::string CornellBox = "object/CornellBox/CornellBox.obj";
const inline std::string Planet = "object/planet/planet.obj";
const inline std::string Rock = "object/rock/rock.obj";
const inline std::string Fairy = "object/fairy/fairy.obj";
const inline std::string Nanosuit = "object/nanosuit/nanosuit.obj";
const inline std::string silly_dancing = "object/silly_dancing.fbx";
const inline std::string nierautomata_2b = "object/nierautomata_2b/scene.gltf";
const inline std::string ftm_sketchfab = "object/ftm/ftm_sketchfab.blend";
const inline std::string Nier_2b_ik_rigged = "object/Nier_2b_ik_rigged/scene.gltf";
const inline std::string Sponza = "object/sponza/sponza.obj";

// model
struct render_misc
{
    const unsigned int Width = 1080;
    const unsigned int Height = 720;

    // timing
    float currentFrame = 0.f;
    float deltaTime = 0.f;
    float lastFrame = 0.f;

    // model
    glm::vec3 model_scale{1.f};

    // camera_
    fay::camera camera_{glm::vec3{ 0, 0, 10 }};
    float lastX = Width / 2.0f;
    float lastY = Height / 2.0f;
    bool firstMouse = true;

    //light 
    glm::vec3 lightPosition = glm::vec3(15, 15, 0); //objectspace light position
    float light_speed = 2.f;
    glm::vec3 light_scale{ 0.5f, 0.5f, 0.5f };

    // some flag
    bool some_flag = false;
    char mouse_move = 'z';
    int render_state = 1;

    void update_io(const fay::single_input& io = fay::input)
    {
        currentFrame = glfwGetTime();
        deltaTime = (currentFrame - lastFrame) * 4;
        lastFrame = currentFrame;

        float xpos = io.posx, ypos = io.posy;
        if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
        float xoffset = xpos - lastX; lastX = xpos;
        // reversed since y-coordinates go from bottom to top but z_xais form out to in
        float yoffset = lastY - ypos; lastY = ypos;

        // TODO: io[z], io.x
        if (io.key[' ']) mouse_move = ++mouse_move % 3;
        if (io.key['z']) mouse_move = 'z';
        if (io.key['x']) mouse_move = 'x';
        if (io.key['c']) mouse_move = 'c';

        if (mouse_move == 'z')
        {
            if (model_scale.x <= 1.f)
                model_scale -= glm::vec3(0.1f, 0.1f, 0.1f) * glm::vec3(io.wheel);
            else
                model_scale -= glm::vec3(1.f, 1.f, 1.f) * glm::vec3(io.wheel);

            if (model_scale.x < 0.f)
                model_scale = glm::vec3(0.1f, 0.1f, 0.1f);
            else if (model_scale.x > 10.f)
                model_scale = glm::vec3(10.f, 10.f, 10.f);

            camera_.ProcessMouseMovement(xoffset, yoffset);
            if (io.key['w']) camera_.ProcessKeyboard(fay::FORWARD, deltaTime);
            if (io.key['s']) camera_.ProcessKeyboard(fay::BACKWARD, deltaTime);
            if (io.key['a']) camera_.ProcessKeyboard(fay::LEFT, deltaTime);
            if (io.key['d']) camera_.ProcessKeyboard(fay::RIGHT, deltaTime);
            //camera_.ProcessMouseScroll(io.wheel); 
        }
        else if (mouse_move == 'x')
        {

            light_scale -= glm::vec3(0.1f, 0.1f, 0.1f) * glm::vec3(io.wheel);
            if (light_scale.x < 0.f)
                light_scale = glm::vec3(0.1f, 0.1f, 0.1f);
            else if (light_scale.x > 1.f)
                light_scale = glm::vec3(1.f, 1.f, 1.f);

            light_speed -= io.wheel;
            if (light_speed <= 0.f)
                light_speed = 0.f;
            else if (light_speed >= 10.f)
                light_speed = 10.f;

            if (io.key['w']) lightPosition.z -= deltaTime * light_speed;
            if (io.key['s']) lightPosition.z += deltaTime * light_speed;
            if (io.key['a']) lightPosition.x -= deltaTime * light_speed;
            if (io.key['d']) lightPosition.x += deltaTime * light_speed;
            if (io.left_down) lightPosition.y += deltaTime * light_speed;
            if (io.right_down) lightPosition.y -= deltaTime * light_speed;

            // if (io.MouseDown[2]) clear_color = ImColor(255, 255, 255);
        }
        else if (mouse_move == 'c')
        {
            // GUI
        }
        else
        {
            LOG(ERROR) << "shouldn't be here";
        }
    }
};

struct render_paras
{
    glm::vec4 a{};
    glm::vec4 b{};
};

} // namespace fay