#include "sample_render_app.h"

class PBR : public passes
{
public:
    fay::texture_id tex_id0, tex_id1, tex_id2, tex_id3, tex_id4;

    PBR(const fay::app_desc& _desc) : passes(_desc)
    {
        desc.window.title = "PBR";
    }

    void setup() override
    {
        add_update_items();
        debug_setup();

        mesh = fay::create_raw_renderable(fay::Sphere, device.get());

        fay::image img0("texture/pbr/rusted_iron/albedo.png", true);
        fay::image img1("texture/pbr/rusted_iron/metallic.png", true);
        fay::image img2("texture/pbr/rusted_iron/roughness.png", true);
        fay::image img3("texture/pbr/rusted_iron/normal.png", true);
        fay::image img4("texture/pbr/rusted_iron/ao.png", true);
        tex_id0 = create_2d(this->device, "albedo", img0);
        tex_id1 = create_2d(this->device, "metal", img1);
        tex_id2 = create_2d(this->device, "rough", img2);
        tex_id3 = create_2d(this->device, "normal", img3);
        tex_id4 = create_2d(this->device, "ao", img4);

        fay::shader_desc sd = fay::scan_shader_program("shd", "gfx/pbr.vs", "gfx/pbr.fs", desc.render.backend);
        shd_id = device->create(sd);

        fay::pipeline_desc pd;
        pd.name = "pipe";
        pipe_id = device->create(pd);
    }

    void render() override
    {
        GLfloat near_plane = 1.f, far_plane = 200.f;
        glm::mat4 lightOrtho = glm::orthoLH(-150.f, 150.f, -100.0f, 100.0f, near_plane, far_plane);
        glm::mat4 lightProj = glm::perspectiveLH(glm::radians(90.f),
            1080.f / 720.f, near_plane, far_plane);
        glm::mat4 lightView = glm::lookAtLH(
            light->position(), glm::vec3(0.0f), glm::vec3(0.f, 1.f, 0.f));
        glm::mat4 lightSpace = lightProj * lightView;

        // debug info
        // FIXME: over the GPU memory
        //fay::bounds3 box_light(-70, 70);
        fay::frustum box_light(lightSpace);
        auto debug_light = create_box_mesh(box_light, device.get());

        fay::command_list pass;

        pass
            .begin_default(pipe_id, shd_id)
            // TODO: check uniform (blocks)
            .bind_uniform("proj", camera->persp())
            .bind_uniform("view", camera->view())
            .bind_uniform("camPos", camera->position())
            .bind_uniform("lightPositions[0]", light->position())
            .bind_uniform("lightPositions[1]", glm::vec3{ 0, 0, -100 })
            .bind_uniform("lightPositions[2]", glm::vec3{ -100, 100, -100 })
            .bind_uniform("lightPositions[3]", glm::vec3{ 100, 100, -100 })
            .bind_uniform("lightColor", glm::vec3(1.f, 1.f, 1.f))

            .bind_textures({ tex_id0, tex_id1, tex_id2, tex_id3, tex_id4, })
            .bind_uniform("Albedo", glm::vec3(0.5f, 0.0f, 0.0f))
            .bind_uniform("Ao", 1.f);

        int nrRows = 7;
        int nrColumns = 7;
        float spacing = 25;
        glm::mat4 model = glm::mat4(1.f);
        for (int row = 0; row < nrRows; ++row)
        {
            for (int col = 0; col < nrColumns; ++col)
            {
                // we clamp the roughness to 0.025 - 1.0 as perfectly smooth surfaces (roughness of 0.0) tend to look a bit off
                // on direct lighting.

                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(
                    (col - (nrColumns / 2)) * spacing,
                    (row - (nrRows / 2)) * spacing + 40.f,
                    0.0f
                ));
                model = glm::scale(model, glm::vec3(0.5));

                pass
                    .bind_uniform("model", model)
                    .bind_uniform("Metallic", (float)row / (float)nrRows)
                    .bind_uniform("Roughness", glm::clamp((float)col / (float)nrColumns, 0.05f, 0.95f))
                    .draw(mesh.get());
            }
        }

        pass
            .apply_pipeline(debug_pipe_id)
            .apply_shader(debug_shd_id)
            .bind_uniform("MVP", camera->world_to_ndc())
            .draw(debug_light.get())
            .end_frame();

        device->execute(pass);
    }
};

class IBL : public passes
{
public:
    fay::texture_id tex_id0, tex_id1, tex_id2, tex_id3, tex_id4;

    fay::texture_id env_tex_id;
    fay::shader_id background_shd_id;


    IBL(const fay::app_desc& _desc) : passes(_desc)
    {
        desc.window.title = "PBR";
    }

    void setup() override
    {
        add_update_items();
        debug_setup();

        mesh = fay::create_raw_renderable(fay::Sphere, device.get());
        mesh2 = fay::create_raw_renderable(fay::Face, device.get()); // face
        mesh3 = fay::create_raw_renderable(fay::Box, device.get()); // box

        fay::image env_img("texture/hdr/newport_loft.hdr");
        env_tex_id = create_2d(this->device, "equirectangularMap", env_img);

        auto generate_cube_shd_id = create_shader("generate_cube", "gfx/IBL/cubemap.vs", "gfx/IBL/generate_cubemap.fs", device.get());
        auto irradiance_shd_id = create_shader("irradiance", "gfx/IBL/cubemap.vs", "gfx/IBL/irradiance_convolution.fs", device.get());
        auto prefilter_map_shd_id = create_shader("prefilter_map", "gfx/IBL/cubemap.vs", "gfx/IBL/prefilter.fs", device.get());
        auto brdf_map_shd_id = create_shader("brdf_map", "gfx/IBL/brdf.vs", "gfx/IBL/brdf.fs", device.get());
        background_shd_id = create_shader("background", "gfx/IBL/background.vs", "gfx/IBL/background.fs", device.get());
        shd_id = create_shader("IBL_PBR", "gfx/IBL/pbr.vs", "gfx/IBL/pbr.fs", device.get());

        {
            fay::pipeline_desc pd;
            pd.name = "pipe";
            pd.cull_mode = fay::cull_mode::none; // TODO: default set cull_mode::none
            pipe_id = device->create(pd);
        }

        // TODO: in_fmt, ex_fmt
        const size_t res = 512, res2 = 32, res3 = 128, res4 = 512;
        frame = fay::create_cubemap_frame(device.get(), "cubemap_frame", res, res, fay::pixel_format::rgb32f, 12);
        frame2 = fay::create_cubemap_frame(device.get(), "irradiance_frame", res2, res2, fay::pixel_format::rgb32f, 12);

        fay::command_list pass, pass2;

        glm::mat4 captureProjection = glm::perspectiveLH(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 captureView = glm::lookAtLH(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 captureModel(1.f);
        glm::mat4 captureModels[6] =
        {
            // positive x, negative x, positive y, negative y, positive z, negative z, 
            glm::rotate(captureModel, glm::radians(90.f),  glm::vec3(0.f, 1.f, 0.f)),
            glm::rotate(captureModel, glm::radians(270.f), glm::vec3(0.f, 1.f, 0.f)),

            glm::rotate(captureModel, glm::radians(270.f), glm::vec3(1.f, 0.f, 0.f)),
            glm::rotate(captureModel, glm::radians(90.f),  glm::vec3(1.f, 0.f, 0.f)),

            captureModel,
            glm::rotate(captureModel, glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)),
        };

        pass
            .begin_frame(frame)
            .clear_frame()
            .set_viewport(0, 0, res, res)
            .set_scissor(0, 0, res, res)
            .apply_pipeline(pipe_id)
            .apply_shader(generate_cube_shd_id)
            .bind_uniform("model0", captureModels[0])
            .bind_uniform("model1", captureModels[1])
            .bind_uniform("model2", captureModels[2])
            .bind_uniform("model3", captureModels[3])
            .bind_uniform("model4", captureModels[4])
            .bind_uniform("model5", captureModels[5])
            .bind_uniform("proj", captureProjection)
            .bind_uniform("view", captureView)
            .bind_texture(env_tex_id, "equirectangularMap") // TODO: bind_texture("equirectangularMap", env_tex_id)
            .draw(mesh2.get())
            .end_frame()
            ;

        pass2
            .begin_frame(frame2)
            .clear_frame()
            .set_viewport(0, 0, res2, res2)
            .set_scissor(0, 0, res2, res2)
            .apply_pipeline(pipe_id)
            .apply_shader(irradiance_shd_id)
            .bind_uniform("model0", captureModels[0])
            .bind_uniform("model1", captureModels[1])
            .bind_uniform("model2", captureModels[2])
            .bind_uniform("model3", captureModels[3])
            .bind_uniform("model4", captureModels[4])
            .bind_uniform("model5", captureModels[5])
            .bind_uniform("proj", captureProjection)
            .bind_uniform("view", captureView)
            .bind_texture(frame[0], "environmentMap")
            .draw(mesh2.get())
            .end_frame()
            ;

        const size_t maxLevel = 5;
        fay::command_list pass3[maxLevel];
        {
            auto offscreen_tex_id3 = fay::create_cubemap(device.get(), "prefilter_map_frame", res3, res3, fay::pixel_format::rgb32f, 12, true);
            auto offscreen_ds_id3 = create_depth_stencil_map(device.get(), "prefilter_map_frame", res3, res3);

            for (size_t level = 0; level < maxLevel; ++level)
            {
                // reisze framebuffer according to mip-level size.
                unsigned int mipWidth = res3 * std::pow(0.5, level);
                unsigned int mipHeight = res3 * std::pow(0.5, level);

                // TODO
                frame3 = fay::create_mipmap_cubemap_frame(device.get(), "prefilter_map_frame", mipWidth, mipHeight, offscreen_tex_id3, offscreen_ds_id3, level);

                pass3[level]
                    .begin_frame(frame3)
                    .clear_frame()
                    .set_viewport(0, 0, mipWidth, mipHeight)
                    .set_scissor(0, 0, mipWidth, mipHeight)
                    .apply_pipeline(pipe_id)
                    .apply_shader(prefilter_map_shd_id)
                    .bind_uniform("model0", captureModels[0])
                    .bind_uniform("model1", captureModels[1])
                    .bind_uniform("model2", captureModels[2])
                    .bind_uniform("model3", captureModels[3])
                    .bind_uniform("model4", captureModels[4])
                    .bind_uniform("model5", captureModels[5])
                    .bind_uniform("proj", captureProjection)
                    .bind_uniform("view", captureView)
                    .bind_uniform("roughness", (float)level / (float)(maxLevel - 1))
                    .bind_texture(frame[0], "environmentMap")
                    .draw(mesh2.get())
                    .end_frame()
                    ;
            }

            // frame3 = fay::frame(fay::frame_id(0), offscreen_tex_id3, offscreen_ds_id3);
        }

        fay::command_list pass4;
        {
            // TODO: ctor, set_sampler, set_
            fay::texture_desc desc;
            desc.name = "brdf_tex";
            desc.width = res4;
            desc.height = res4;
            desc.format = fay::pixel_format::rgb32f; // rename: depth_stencil
            desc.size = res4 * res4 * 12;
            desc.type = fay::texture_type::two;
            desc.data = { nullptr };
            desc.wrap_u = fay::wrap_mode::clamp_to_edge;
            desc.wrap_v = fay::wrap_mode::clamp_to_edge;
            desc.mipmap = false; // TODO: default set it false

            auto offscreen_tex_id4 = device->create(desc);
            auto offscreen_ds_id4 = create_depth_stencil_map(device.get(), "brdf_ds", res4, res4);

            fay::frame_desc fd;
            fd.name = "brdf_frame";
            fd.width = res4;
            fd.height = res4;
            fd.render_targets = { { offscreen_tex_id4, 0, 0 } };
            fd.depth_stencil = { offscreen_ds_id4, 0, 0 };
            auto offscreen_frm_id4 = device->create(fd);
            frame4 = fay::frame(offscreen_frm_id4, offscreen_tex_id4, offscreen_ds_id4);

            pass4
                .begin_frame(frame4)
                .clear_frame()
                .set_viewport(0, 0, res4, res4)
                .set_scissor(0, 0, res4, res4)
                .apply_pipeline(pipe_id)
                .apply_shader(brdf_map_shd_id)
                .draw(mesh2.get())
                .end_frame()
                ;
        }

        device->execute({ pass, pass2, pass3[0], pass3[1], pass3[2], pass3[3], pass3[4], pass4 });
    }

    void render() override
    {
        fay::command_list pass;

        pass
            .begin_default(pipe_id, shd_id)
            // TODO: check uniform (blocks)
            .bind_uniform("proj", camera->persp())
            .bind_uniform("view", camera->view())
            .bind_uniform("camPos", camera->position())
            .bind_uniform("lightPositions[0]", light->position())
            .bind_uniform("lightPositions[1]", glm::vec3{ 0, 0, -100 })
            .bind_uniform("lightPositions[2]", glm::vec3{ -100, 100, -100 })
            .bind_uniform("lightPositions[3]", glm::vec3{ 100, 100, -100 })
            .bind_uniform("lightColor", glm::vec3(1.f, 1.f, 1.f))

            .bind_textures({ tex_id0, tex_id1, tex_id2, tex_id3, tex_id4 })
            .bind_texture(frame2[0], "irradianceMap")
            .bind_texture(frame3[0], "prefilterMap")
            .bind_texture(frame4[0], "brdfLUT")
            .bind_uniform("Albedo", glm::vec3(0.5f, 0.0f, 0.0f))
            .bind_uniform("Ao", 1.f)
            ;

        int nrRows = 7;
        int nrColumns = 7;
        float spacing = 25;
        glm::mat4 model = glm::mat4(1.f);
        for (int row = 0; row < nrRows; ++row)
        {
            for (int col = 0; col < nrColumns; ++col)
            {
                // we clamp the roughness to 0.025 - 1.0 as perfectly smooth surfaces (roughness of 0.0) tend to look a bit off
                // on direct lighting.

                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(
                    (col - (nrColumns / 2)) * spacing,
                    (row - (nrRows / 2)) * spacing + 40.f,
                    0.0f
                ));
                model = glm::scale(model, glm::vec3(0.5));

                pass
                    .bind_uniform("model", model)
                    .bind_uniform("Metallic", (float)row / (float)nrRows)
                    .bind_uniform("Roughness", glm::clamp((float)col / (float)nrColumns, 0.05f, 0.95f))
                    .draw(mesh.get());
            }
        }

        // background
        pass
            .apply_pipeline(pipe_id)
            .apply_shader(background_shd_id)
            .bind_texture(frame[0], "environmentMap") // TODO: layer, level
            //.bind_texture(offscreen_tex_id2, "environmentMap")
            .bind_uniform("proj", camera->persp())
            .bind_uniform("view", camera->view())
            .draw(mesh3.get())
            .end_frame();

        //device->execute(pass2);
        device->execute({ pass });
    }
};

SAMPLE_RENDER_APP_IMPL(PBR)
SAMPLE_RENDER_APP_IMPL(IBL)