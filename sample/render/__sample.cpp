#include "sample_render_app.h"

class __sample : public passes
{
public:
    // using fay::app;
    __sample(const fay::app_desc& _desc) : passes(_desc)
    {
        desc.window.title = "post_proc";
    }

    void setup() override
    {
        add_update_items();

        mesh = fay::create_raw_renderable(fay::Box, device.get());

        fay::image img("texture/awesomeface.png");//, true);
        tex = create_2d(this->device, "hello", img, false);

        fay::shader_desc sd = fay::scan_shader_program("shd", "gfx/offscreen.vs", "gfx/offscreen.fs", desc.render.backend);
        shd = device->create(sd);

        fay::pipeline_desc pd;
        {
            pd.name = "triangles";
            // pd.cull_mode = fay::cull_mode::none;
        }
        pipe = device->create(pd);

        frame = fay::create_frame(device.get(), "offscreen_frm", 512, 512);
    }

    void render() override
    {
        glm::mat4 model(1.f);
        auto model1 = glm::scale(model, glm::vec3(40.f, 40.f, 40.f));

        auto VP = camera->world_to_ndc();

        fay::command_list pass1, pass2;

        pass1
            .begin_frame(frame)
            .clear_color({ 1.f, 0.f, 0.f, 1.f })
            .clear_depth()
            .clear_stencil()
            .apply_pipeline(pipe)
            .apply_shader(shd)
            .bind_textures({ tex })
            .bind_uniform("bAlbedo", true)
            .bind_uniform("MVP", VP * model1)
            .draw(mesh.get())
            .end_frame();

        pass2
            .begin_default(pipe, shd)
            .bind_textures({ frame[0] })
            .bind_uniform("bAlbedo", true)
            .bind_uniform("MVP", VP * model1)
            .draw(mesh.get())
            .end_frame();

        device->execute({ pass1, pass2 });
    }
};

SAMPLE_RENDER_APP_IMPL(__sample)

