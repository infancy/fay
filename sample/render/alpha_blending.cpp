#include "sample_render_app.h"

// class WBOIT : public passes;

class WBOIT : public passes
{
public:
    // TODO: using fay::app;
    WBOIT(const fay::app_desc& _desc) : passes(_desc)
    {
        desc.window.title = "WBOIT";
    }

    void setup() override
    {
        add_update_items();
        cameras_[0] = fay::camera(glm::vec3(50, 100.f, -50));

        mesh = fay::create_raw_renderable(fay::Sphere, device.get());

        fay::image img("texture/opengl.png");
        tex = create_2d(this->device, "aliasing", img, false);

        fay::shader_desc sd = fay::scan_shader_program("shd", "gfx/phong_shading.vs", "gfx/wboit_accum.fs", desc.render.backend);
        fay::shader_desc sd2 = fay::scan_shader_program("shd2", "gfx/post_processing.vs", "gfx/wboit_postprocessing.fs", desc.render.backend);
        shd = device->create(sd);
        shd2 = device->create(sd2);

        {
            fay::pipeline_desc pd;
            pd.name = "wboit_accum";
            // TODO: boost::spirit: src + dst, 0 + dst * (1-src)
            pd.cull_mode = fay::cull_mode::none;
            pd.blend_enabled = true;
            pd.blend_rgb   = { fay::blend_factor::one, fay::blend_factor::one, fay::blend_op::add };
            pd.blend_alpha = { fay::blend_factor::zero, fay::blend_factor::one_minus_src_alpha, fay::blend_op::add }; // TODO: one_sub_src_alpha
            pipe = device->create(pd);
        }

        {
            fay::pipeline_desc pd;
            pd.name = "wboit_postprocessing";
            pd.blend_enabled = true;
            pd.blend_rgb = { fay::blend_factor::one, fay::blend_factor::one_minus_src_alpha, fay::blend_op::add };
            pd.blend_alpha = pd.blend_rgb;
            pipe2 = device->create(pd);
        }

        frame = fay::create_OITbuffer(device.get(), "offscreen_frm", 1080u, 720u);
    }

    void render() override
    {
        auto P = camera->persp();
        auto V = camera->view();

        fay::command_list pass1, pass2;

        pass1
            .begin_frame(frame, pipe, shd)
            .bind_texture(tex, "Albedo")
            .bind_uniform("uLightPosition", glm::vec4(light->position(), 1.f)); // TODO: glm::vec4 position();
        const int length = 5;
        for(int i = 0; i < length; ++i)
            for(int j = 0; j < length; ++j)
                for (int k = 0; k < length; ++k)
                {
                    glm::mat4 M = glm::translate(transform->model_matrix(), glm::vec3(i, j, k) * glm::vec3(20));

                    pass1.bind_uniform("MV", V * M)
                        .bind_uniform("NormalMV", glm::mat3(glm::transpose(glm::inverse(V * M))))
                        .bind_uniform("MVP", P * V * M)
                        .draw(mesh.get());
                }
        pass1.end_frame();

        pass2
            .begin_default(pipe2, shd2)
            .bind_texture(frame[0], "uAccumulate")
            .bind_texture(frame[1], "uAccumulateAlpha")
            .draw(6)
            .end_frame();

        device->execute({ pass1, pass2 });
    }
};

SAMPLE_RENDER_APP_IMPL(WBOIT)