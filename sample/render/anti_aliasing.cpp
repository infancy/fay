#include "sample_render_app.h"

// class FXAA : public passes;

class FXAA : public passes
{
public:
    // using fay::app;
    FXAA(const fay::app_desc& _desc) : passes(_desc)
    {
        desc.window.title = "WBOIT";
    }

    void setup() override
    {
        add_update_items();
        cameras_[0] = fay::camera(glm::vec3(0, 0.5f, -3));

        mesh = fay::create_renderable(fay::Box, device.get());

        fay::image img("texture/wboit.png");//, true);
        tex = create_2d(this->device, "aliasing", img, false);

        fay::shader_desc sd  = fay::scan_shader_program("shd", "gfx/renderable.vs", "gfx/renderable.fs", desc.render.backend);
        fay::shader_desc sd2 = fay::scan_shader_program("shd2", "gfx/post_processing.vs", "gfx/FXAA.fs", desc.render.backend);
        shd  = device->create(sd);
        shd2 = device->create(sd2);

        fay::pipeline_desc pd;
        {
            pd.name = "triangles";
            // pd.cull_mode = fay::cull_mode::none;
        }
        pipe = device->create(pd);

        frame = fay::create_frame(device.get(), "offscreen_frm", 1080u, 720u);
    }

    void render() override
    {
        glm::mat4 M = transform->model_matrix();

        auto VP = camera->world_to_ndc();

        fay::command_list pass1, pass2;

        pass1
            .begin_frame(frame, pipe, shd)
            .bind_uniform("bAlbedo", true)
            .bind_uniform("MVP", VP * M)
            .draw(mesh.get())
            .end_frame();

        pass2
            .begin_default(pipe, shd2)
            .bind_uniform("offset", camera->plane())
            .bind_textures({ tex })
            .draw(6)
            .end_frame();

        device->execute({ pass2 });
    }
};

SAMPLE_RENDER_APP_IMPL(FXAA)