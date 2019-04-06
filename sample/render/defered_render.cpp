#include "sample_render_app.h"

class defered_shading : public passes
{
public:
    // using fay::app;
    defered_shading(const fay::app_desc& _desc) : passes(_desc)
    {
        desc.window.title = "defered_shading";
    }

    std::vector<glm::vec3> objectPositions;

    const unsigned int num_lights = 32;
    std::vector<glm::vec3> lightPositions;
    std::vector<glm::vec3> lightColors;

    void setup() override
    {
        cameras_[0] = fay::camera(glm::vec3(3.f, 3.f, -10.f));
        add_update_items();

        objectPositions.push_back(glm::vec3(-5.0, -5.0, -5.0));
        objectPositions.push_back(glm::vec3( 0.0, -5.0, -5.0));
        objectPositions.push_back(glm::vec3( 5.0, -5.0, -5.0));

        objectPositions.push_back(glm::vec3(-5.0, -5.0,  0.0));
        objectPositions.push_back(glm::vec3( 0.0, -5.0,  0.0));
        objectPositions.push_back(glm::vec3( 5.0, -5.0,  0.0));

        objectPositions.push_back(glm::vec3(-5.0, -5.0,  5.0));
        objectPositions.push_back(glm::vec3( 0.0, -5.0,  5.0));
        objectPositions.push_back(glm::vec3( 5.0, -5.0,  5.0));

        srand(glfwGetTime());
        for (unsigned int i = 0; i < num_lights; i++)
        {
            // calculate slightly random offsets
            float xPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
            float yPos = ((rand() % 100) / 100.0) * 6.0 - 4.0;
            float zPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
            lightPositions.push_back(glm::vec3(xPos, yPos, zPos));
            // also calculate random color
            float rColor = ((rand() % 100) / 200.0f) + 0.5; // between 0.5 and 1.0
            float gColor = ((rand() % 100) / 200.0f) + 0.5; // between 0.5 and 1.0
            float bColor = ((rand() % 100) / 200.0f) + 0.5; // between 0.5 and 1.0
            lightColors.push_back(glm::vec3(rColor, gColor, bColor));
        }

        mesh = fay::create_renderable(fay::Rei, device.get());
        mesh2 = fay::create_raw_renderable(fay::Box, device.get());

        shd  = fay::create_shader(device.get(), "shd", "gfx/30_phong_shading.vs", "gfx/38_g_buffer.fs");
        shd2 = fay::create_shader(device.get(), "shd2", "gfx/post_processing.vs", "gfx/38_deferred_shading.fs");
        shd3 = fay::create_shader(device.get(), "shd2", "gfx/post_processing.vs", "gfx/post_processing.fs");

        {
            fay::pipeline_desc pd;
            {
                pd.name = "geometry_stage_pipe";
                pd.cull_mode = fay::cull_mode::none;
            }
            pipe = device->create(pd);
        }
        {
            fay::pipeline_desc pd;
            {
                pd.name = "light_stage_pipe";
                pd.cull_mode = fay::cull_mode::none;
            }
            pipe2 = device->create(pd);
        }

        frame = fay::create_Gbuffer(device.get(), "Gbuffer_frm", 1024, 1024);

        {
            fay::texture_desc offscreen_desc("offscreen_tex", 1024, 1024, fay::pixel_format::rgba8);
            offscreen_desc.as_render_target = fay::render_target::color;
            auto offscreen_tex = device->create(offscreen_desc);

            fay::frame_desc fd("offscreen", { { offscreen_tex, 0, 0 } }, { frame.dsv(), 0, 0 });
            auto frm_id = device->create(fd);

            frame2 = fay::frame(frm_id, { offscreen_tex }, frame.dsv());

            frame2 = fay::create_frame(device.get(), "offscreen", 1024, 1024);
        }
    }

    void render() override
    {
        glm::mat4 view = camera->view();
        glm::mat4 proj = camera->persp();

        auto MVP = proj * view * transform->model_matrix();

        fay::command_list pass1, pass2, pass3;

        // depth map
        pass1.begin_frame(frame, pipe, shd);
        for (unsigned int i = 0; i < objectPositions.size(); i++)
        {
            glm::mat4 objectmodel = glm::mat4(1);
            objectmodel = glm::translate(objectmodel, objectPositions[i]);
            objectmodel = glm::scale(objectmodel, glm::vec3(5.f));

            glm::mat4 MV = view * objectmodel;
            glm::mat3 NormalMV = glm::mat3(glm::transpose(glm::inverse(MV)));

            pass1
                .bind_uniform("MV", MV)
                .bind_uniform("NormalMV", NormalMV)
                .bind_uniform("MVP", proj * MV)
                .draw(mesh.get());
        }
        pass1.end_frame();

        pass2
            .begin_frame(frame2, pipe2, shd2)
            .bind_texture(frame[0], "gPosition")
            .bind_texture(frame[1], "gNormal")
            .bind_texture(frame[2], "gAlbedoSpec")
            .bind_uniform("offset", glm::vec2(0.f))
            .bind_uniform("viewPos", camera->position());
        for (unsigned int i = 0; i < lightPositions.size(); i++)
        {
            // render attenuation parameters and calculate radius
            const float constant = 1.0; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
            const float linear = 0.7;
            const float quadratic = 1.8;

            pass2
                .bind_uniform("lights[" + std::to_string(i) + "].Position", lightPositions[i])
                .bind_uniform("lights[" + std::to_string(i) + "].Color", lightColors[i])
                .bind_uniform("lights[" + std::to_string(i) + "].Linear", linear)
                .bind_uniform("lights[" + std::to_string(i) + "].Quadratic", quadratic);
        }
        pass2.draw(6);

        // use frame.dsv as frame2.dsv
        pass2.end_frame();

        pass3
            .begin_default(pipe, shd3)
            .bind_uniform("offset", glm::vec2(0.f))
            .bind_texture(frame2[0], "frame")
            .draw(6)
            .end_frame();

        device->execute({ pass1, pass2, pass3 });
    }
};

// defered_lighting

// tile_based_deferred_shading

// clustered_based_deferred_shading

SAMPLE_RENDER_APP_IMPL(defered_shading)