#include "sample_render_app.h"

class SSR : public passes
{
public:
    // using fay::app;
    SSR(const fay::app_desc& _desc) : passes(_desc)
    {
        desc.window.title = "SSR";
    }

    void setup() override
    {
        cameras_[0] = fay::camera(glm::vec3(0.f, 30.f, -50.f));
        lights_[0] = fay::light(glm::vec3(0.f, 15.f, 0.f));

        add_update_items();
 
        mesh = fay::create_renderable("model/low_poly_car/scene.obj", device.get());

        shd = fay::create_shader(device.get(),  "shd",  "gfx/phong_shading.vs", "gfx/deferred_shading_gbuffer.fs");
        shd2 = fay::create_shader(device.get(), "shd2", "gfx/phong_shading.vs", "gfx/deferred_shading_depth.fs");
        shd3 = fay::create_shader(device.get(), "shd3", "gfx/post_processing.vs", "gfx/SSR.fs");
        shd4 = fay::create_shader(device.get(), "shd4", "gfx/post_processing.vs", "gfx/post_processing.fs");

        {
            fay::pipeline_desc pd;
            {
                pd.name = "geometry_stage_pipe_front";
                pd.cull_mode = fay::cull_mode::back;
            }
            pipe = device->create(pd);
        }
        {
            fay::pipeline_desc pd;
            {
                pd.name = "geometry_stage_pipe_back";
                pd.cull_mode = fay::cull_mode::front;
            }
            pipe2 = device->create(pd);
        }
        {
            fay::pipeline_desc pd;
            {
                pd.name = "light_stage_pipe";
                pd.cull_mode = fay::cull_mode::none;
                pd.depth_enabled = false; // why can't use frame.dsv
            }
            pipe3 = device->create(pd);
        }

        frame = fay::create_Gbuffer(device.get(), "Gbuffer_frm", 1024, 1024);

        {
            fay::texture_desc offscreen_desc("offscreen_tex", 1024, 1024, fay::pixel_format::rgba8);
            offscreen_desc.as_render_target = fay::render_target::color;
            auto offscreen_tex = device->create(offscreen_desc);

            fay::frame_desc fd("offscreen", { { offscreen_tex, 0, 0 } }, { frame.dsv(), 0, 0 });
            fd.width = 1024;
            fd.height = 1024;
            auto frm_id = device->create(fd);

            frame2 = fay::frame(frm_id, { offscreen_tex }, frame.dsv());

            //frame2 = fay::create_frame(device.get(), "offscreen", 1024, 1024);
        }
    }

    void render() override
    {
        glm::mat4 model = glm::mat4(1);
        model = glm::scale(model, glm::vec3(0.8f));
        glm::mat4 view = camera->view();
        glm::mat4 proj = camera->persp();

        glm::mat4 MV = view * model;
        glm::mat3 NormalMV = glm::mat3(glm::transpose(glm::inverse(MV)));

        fay::command_list pass1, pass2, pass3;

        // depth map
        pass1
            .begin_frame(frame, pipe, shd)
            .bind_uniform("MV", MV)
            .bind_uniform("NormalMV", NormalMV)
            .bind_uniform("MVP", proj * MV)
            .draw(mesh.get())
            .apply_shader(shd2)
            .bind_uniform("bFront", true)
            .draw(mesh.get())
            .apply_pipeline(pipe2)
            .bind_uniform("bFront", false)
            .draw(mesh.get())
            .end_frame();

        pass2
            .begin_frame(frame2)
            .clear_color() // WARNNING
            .apply_state(pipe3, shd3)
            .bind_texture(frame[0], "gPosition")
            .bind_texture(frame[1], "gNormal")
            .bind_texture(frame[2], "gAlbedoSpec")
            .bind_uniform("offset", glm::vec2(0.f))
            .bind_uniform("lightPosition", light->position())
            .bind_uniform("lightColor", glm::vec3(1.f, 1.f, 1.f))
            .bind_uniform("viewPos", camera->position())
            .draw(6)
            .end_frame();

        pass3
            .begin_default(pipe3, shd4)
            .bind_uniform("offset", glm::vec2(0.f))
            .bind_texture(frame2[0], "frame")
            .draw(6)
            .end_frame();

        device->execute({ pass1, pass2, pass3 });
    }
};

SAMPLE_RENDER_APP_IMPL(SSR)