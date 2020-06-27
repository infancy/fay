#include "sample_render_app.h"

class shadow_map : public passes
{
public:
    shadow_map(const fay::app_desc& _desc) : passes(_desc)
    {
        desc.window.title = "shadow_map";
    }

    void setup() override
    {
        add_update_items();
        debug_setup();


        cameras_[0] = fay::camera{ glm::vec3{ 0, 20, -80 }, 90, 0, 1.f, 300.f }; // look at the positive-z axis
        cameras_[1] = fay::camera{ glm::vec3{ -300, 300, 0 }, /*-180*/0, -45, 1.f, 1000.f }, // look at the positive-x axis
        //cameras_[1] = fay::camera{ glm::vec3{ -300, 0, 0 }, /*-180*/0, 0, 1.f, 1000.f }; // look at the positive-x axis

        lights_[0] = fay::light{ glm::vec3{ 0, 150, 0 } };


        mesh = fay::create_renderable(fay::Plants, device.get());

        {
            fay::shader_desc sd = fay::scan_shader_program("shadow_map_depth_pass", "gfx/32_shadow_map.vs", "gfx/32_shadow_map.fs", desc.render.backend);
            sd.name = "shd"; //todo
            shd = device->create(sd);

            fay::pipeline_desc pd;
            pd.name = "shadow_pipe";
            pd.cull_mode = fay::cull_mode::none;
            // pd.cull_mode = fay::cull_mode::front; // sometimes could improve the quality
            pd.stencil_enabled = false;
            pipe = device->create(pd);
        }

        {
            fay::shader_desc sd = fay::scan_shader_program("shadow_model", "gfx/shadow_model.vs", "gfx/shadow_model.fs", desc.render.backend);
            sd.name = "shd2"; //todo
            shd2 = device->create(sd);

            fay::pipeline_desc pd;
            pd.name = "pipe2";
            //pd.cull_mode = fay::cull_mode::none;
            pipe2 = device->create(pd);
        }

        frame = fay::create_depth_frame("shadowmap_frame", 1024, 1024, device.get());
    }

    glm::mat4 frustum_to_ortho(glm::vec3 light_position, fay::frustum box, glm::vec3 camera_up = glm::vec3(0.f, 1.f, 0.f))
    {
        glm::mat4 lightView = glm::lookAt(light_position, box.center(), camera_up);

        // transform to light space
        //glm::vec3 aa = glm::vec3(lightView * glm::vec4(bounds.min(), 1.f));
        //glm::vec3 bb = glm::vec3(lightView * glm::vec4(bounds.max(), 1.f));

        auto corners = box.corners();
        for (auto& c : corners)
            c = glm::vec3(lightView * glm::vec4(c, 1.f));

        fay::bounds3 bounds(corners[0], corners[1]);
        for (size_t i : fay::range(2, 8))
            bounds.expand(corners[i]);

        glm::vec3 min = bounds.min(), max = bounds.max();

        return  glm::orthoLH(
            min.x - 10, max.x + 10,
            min.y - 10, max.y + 10,
            min.z - 10, max.z + 10
        );
    }

    void render() override
    {
        // debug info
        // FIXME: over the GPU memory
        //fay::bounds3 box(-70, 70);
        fay::frustum box_camera(cameras_[0].world_to_ndc());
        auto debug_camera = create_box_mesh(box_camera, device.get());



        GLfloat near_plane = 1.f, far_plane = 200.f;
        glm::mat4 lightProj = glm::perspective(glm::radians(90.f),
            1080.f / 720.f, near_plane, far_plane);

        glm::mat4 lightView = glm::lookAt(
            light->position(), glm::vec3(1.f, -10.f, 1.f), glm::vec3(0.f, 1.f, 0.f)); // TODO: camera_up
        glm::mat4 lightOrtho = glm::ortho(-150.f, 150.f, -100.0f, 100.0f, near_plane, far_plane);
        //glm::mat4 lightOrtho = frustum_to_ortho(light->position(), box_camera);

        glm::mat4 lightSpace = lightOrtho * lightView;



        //fay::bounds3 box_light(-70, 70);
        fay::frustum box_light(lightSpace);
        auto debug_light = create_box_mesh(box_light, device.get());

        auto plane_mat = transform->model_matrix();
        auto plane_mat1 = glm::scale(plane_mat, glm::vec3(0.5f, 0.5f, 0.5f));
        plane_mat1 = glm::translate(plane_mat1, glm::vec3(50.f, 50.f, 50.f));
        
        fay::command_list pass1, pass2;
        // depth map
        pass1
            .begin_frame(frame)
            .clear_color({ 1.f, 0.f, 0.f, 1.f }) // rgb32f
            .clear_depth()
            .clear_stencil()
            .apply_pipeline(pipe)
            .apply_shader(shd)
            .bind_uniform("MVP", lightSpace * plane_mat).draw(mesh.get())
            .bind_uniform("MVP", lightSpace * plane_mat1).draw(mesh.get())
            .end_frame();

        pass2
            .begin_default(pipe2, shd2)
            //.bind_uniform_block("color", fay::memory{ (uint8_t*)&paras, sizeof(render_paras) })
            .bind_uniform("Proj", camera->persp())
            .bind_uniform("View", camera->view())
            .bind_uniform("Model", plane_mat)
            .bind_uniform("LightSpace", lightSpace)
            .bind_uniform("LightPos", light->position())
            .bind_uniform("ViewPos", camera->position())
            .bind_texture(frame.dsv(), "Shadowmap")
            .draw(mesh.get())
            .bind_uniform("Model", plane_mat1).draw(mesh.get())
            // debug info
            .apply_pipeline(debug_pipe)
            .apply_shader(debug_shd)
            .bind_uniform("MVP", camera->world_to_ndc())
            .draw(debug_camera.get()) // they are in the world space, doesn't need model matrix.
            .draw(debug_light.get())
            .end_frame();

        device->execute({ pass1, pass2 });
    }
};


class cascade_shadow_map : public passes
{
public:
    cascade_shadow_map(const fay::app_desc& _desc) : passes(_desc)
    {
        desc.window.title = "shadow_map";
    }

    void setup() override
    {
        cameras_[0] = fay::camera(glm::vec3{ 0, 20, -80 }, 90, 0, 1.f, 500.f);
        add_update_items();
        debug_setup();

        mesh = fay::create_renderable(fay::Plants, device.get());

        {
            fay::shader_desc sd = fay::scan_shader_program("shd", "gfx/32_shadow_map.vs", "gfx/32_shadow_map.fs", desc.render.backend);
            shd = device->create(sd);

            fay::pipeline_desc pd;
            pd.name = "shadow_pipe";
            // pd.cull_mode = fay::cull_mode::front; // sometimes could improve the quality
            pd.stencil_enabled = false;
            pipe = device->create(pd);
        }

        {
            fay::shader_desc sd = fay::scan_shader_program("shd2", "gfx/32_shadow_model.vs", "gfx/32_shadow_model.fs", desc.render.backend);
            shd2 = device->create(sd);

            fay::pipeline_desc pd;
            pd.name = "pipe2";
            pipe2 = device->create(pd);
        }

        frame = fay::create_depth_frame("shadowmap_frame", 1024, 1024, device.get());
        frame2 = fay::create_depth_frame("shadowmap_frame2", 1024, 1024, device.get());
        frame3 = fay::create_depth_frame("shadowmap_frame3", 1024, 1024, device.get());
    }

    glm::mat4 frustum_to_ortho(glm::vec3 light_position, fay::frustum camera_frustum, glm::vec3 camera_up = glm::vec3(0.f, 1.f, 0.f))
    {
        glm::mat4 lightView = glm::lookAtLH(light_position, camera_frustum.center(), camera_up);

        // transform to light space
        //glm::vec3 aa = glm::vec3(lightView * glm::vec4(bounds.min(), 1.f));
        //glm::vec3 bb = glm::vec3(lightView * glm::vec4(bounds.max(), 1.f));

        auto corners = camera_frustum.corners();

        for (auto& c : corners)
            c = glm::vec3(lightView * glm::vec4(c, 1.f)); // transform camera_frustum to light space

        fay::bounds3 bounds(corners[0], corners[1]);
        for (size_t i : fay::range(2, 8))
            bounds.expand(corners[i]);

        glm::vec3 a = bounds.min(), b = bounds.max();

        return  glm::orthoLH(
            a.x - 10, b.x + 10,
            a.y - 10, b.y + 10,
            a.z - 100, b.z + 100
        );
    }

    float transform_view_z_to_NDC(float _near, float _far, float z)
    {
        return (z - _near) / (_far - _near);
    }

    /*
    void render_depth_map(...);

    for(...)
        render_depth_map(...);
    
    */

    void render() override
    {
        size_t frustum_num = 1;
        float near_plane = 1.f, middle_ = 299.f;
        float depthSection[3] = { near_plane, near_plane + middle_ * 0.2f, near_plane + middle_ * 1.f };

        // debug info
        // FIXME: over the GPU memory
        //fay::bounds3 box(-70, 70);
        glm::mat4 lightProj = glm::perspectiveLH(glm::radians(cameras_[0].zoom()), 1080.f / 720.f, depthSection[0], depthSection[1]);
        glm::mat4 lightProj2 = glm::perspectiveLH(glm::radians(cameras_[0].zoom()), 1080.f / 720.f, depthSection[1], depthSection[2]);
        fay::frustum box_camera(lightProj * cameras_[0].view());
        auto debug_camera = create_box_mesh(box_camera, device.get());
        fay::frustum box_camera2(lightProj2 * cameras_[0].view());
        auto debug_camera2 = create_box_mesh(box_camera2, device.get());

        glm::mat4 lightOrtho = frustum_to_ortho(light->position(), box_camera);
        glm::mat4 lightOrtho2 = frustum_to_ortho(light->position(), box_camera2);

        glm::mat4 lightView = glm::lookAtLH(
            light->position(), box_camera.center(), glm::vec3(0.f, 1.f, 0.f)); // TODO: camera_up
        glm::mat4 lightSpace = lightOrtho * lightView;
        glm::mat4 lightView2 = glm::lookAtLH(
            light->position(), box_camera2.center(), glm::vec3(0.f, 1.f, 0.f)); // TODO: camera_up
        glm::mat4 lightSpace2 = lightOrtho2 * lightView2;

        //fay::bounds3 box_light(-70, 70);
        fay::frustum box_light(lightSpace);
        auto debug_light = create_box_mesh(box_light, device.get());
        fay::frustum box_light2(lightSpace2);
        auto debug_light2 = create_box_mesh(box_light2, device.get());

        fay::command_list pass1, pass2, pass3;
        // depth map
        pass1
            .begin_frame(frame)
            .clear_color({ 1.f, 0.f, 0.f, 1.f }) // rgb32f
            .clear_depth()
            .clear_stencil()
            .apply_pipeline(pipe)
            .apply_shader(shd)
            .bind_uniform("MVP", lightSpace * transform->model_matrix())
            .draw(mesh.get())
            .end_frame();
        pass2
            .begin_frame(frame2)
            .clear_color({ 1.f, 0.f, 0.f, 1.f }) // rgb32f
            .clear_depth()
            .clear_stencil()
            .apply_pipeline(pipe)
            .apply_shader(shd)
            .bind_uniform("MVP", lightSpace2 * transform->model_matrix())
            .draw(mesh.get())
            .end_frame();

        pass3
            .begin_default(pipe2, shd2)
            .bind_uniform("Proj", camera->persp())
            .bind_uniform("View", camera->view())
            .bind_uniform("Model", transform->model_matrix())
            .bind_uniform("LightSpace", lightSpace)
            .bind_uniform("LightSpace2", lightSpace2)
            .bind_uniform("LightPos", light->position())
            .bind_uniform("ViewPos", camera->position())
            .bind_uniform("depthSection[1]", -depthSection[1])
            .bind_texture(frame.dsv(), "Shadowmap")
            .bind_texture(frame2.dsv(), "Shadowmap2")
            .draw(mesh.get())

            // debug info
            .apply_pipeline(debug_pipe)
            .apply_shader(debug_shd)
            .bind_uniform("MVP", camera->world_to_ndc())
            .draw(debug_camera.get()) // they are in the world space, doesn't need model matrix.
            .draw(debug_light.get())
            .draw(debug_camera2.get())
            .draw(debug_light2.get())

            .end_frame();

        device->execute({ pass1, pass2, pass3 });
    }
};

SAMPLE_RENDER_APP_IMPL(shadow_map)
SAMPLE_RENDER_APP_IMPL(cascade_shadow_map)