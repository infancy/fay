#include "sample_render_app.h"

class defered_shading : public passes
{
public:
    // using fay::app;
    defered_shading(const fay::app_desc& _desc) : passes(_desc)
    {
        desc.window.title = "post_proc";



    }

    std::vector<glm::vec3> objectPositions;
    const unsigned int NR_LIGHTS = 32;
    std::vector<glm::vec3> lightPositions;
    std::vector<glm::vec3> lightColors;

    void setup() override
    {
        add_update_items();

        objectPositions.push_back(glm::vec3(-3.0, -3.0, -3.0));
        objectPositions.push_back(glm::vec3(0.0, -3.0, -3.0));
        objectPositions.push_back(glm::vec3(3.0, -3.0, -3.0));
        objectPositions.push_back(glm::vec3(-3.0, -3.0, 0.0));
        objectPositions.push_back(glm::vec3(0.0, -3.0, 0.0));
        objectPositions.push_back(glm::vec3(3.0, -3.0, 0.0));
        objectPositions.push_back(glm::vec3(-3.0, -3.0, 3.0));
        objectPositions.push_back(glm::vec3(0.0, -3.0, 3.0));
        objectPositions.push_back(glm::vec3(3.0, -3.0, 3.0));

        srand(glfwGetTime());
        for (unsigned int i = 0; i < NR_LIGHTS; i++)
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

        // quad
        std::vector<glm::ivec3> vb{ { 0, 0, 0 },{ 1, 0, 0 },{ 1, 1, 0 },{ 0, 1, 0 } };
        std::vector<uint32_t> ib{ 0,1,2,2,3,0 };

        {
            fay::image img("texture/awesomeface.png", true);
            tex_id = create_2d(this->device, "hello", img);
        }

        {
            fay::shader_desc sd = fay::scan_shader_program("shd", "gfx/30_phong_shading.vs", "gfx/38_g_buffer.fs", desc.render.backend);
            shd_id = device->create(sd);
        }

        {
            fay::shader_desc sd2 = fay::scan_shader_program("shd2", "gfx/two_passes.vs", "gfx/38_deferred_shading.fs", desc.render.backend);
            shd_id2 = device->create(sd2);
        }

        {
            fay::pipeline_desc pd;
            {
                pd.name = "shadow_pipe";
                pd.cull_mode = fay::cull_mode::none;
            }
            pipe_id = device->create(pd);
        }
        {
            fay::pipeline_desc pd;
            {
                pd.name = "pipe2";
                pd.cull_mode = fay::cull_mode::none;
            }
            pipe_id2 = device->create(pd);
        }

        frame = fay::create_Gbuffer(device.get(), "offscreen_frm", 1024, 1024);
    }

    void render() override
    {
        glm::mat4 view = camera->view();
        glm::mat4 proj = glm::perspectiveLH(glm::radians(camera->zoom()),
            (float)desc.window.width / desc.window.height, 0.1f, 10000.0f);

        auto MVP = proj * view * transform->model_matrix();

        fay::command_list pass1, pass2;

        // depth map
        pass1
            .begin_frame(frame)
            .clear_frame()
            .apply_pipeline(pipe_id)
            .apply_shader(shd_id);
        for (unsigned int i = 0; i < objectPositions.size(); i++)
        {
            //glm::mat4 objectmodel = glm::mat4(50);
            glm::mat4 objectmodel = glm::mat4(1);
            objectmodel = glm::translate(objectmodel, objectPositions[i]);
            objectmodel = glm::scale(objectmodel, glm::vec3(5.f));

            glm::mat4 MV = view * objectmodel;
            glm::mat3 NormalMV = glm::mat3(glm::transpose(glm::inverse(MV)));

            pass1
                .bind_uniform("MV", MV)
                .bind_uniform("NormalMV", NormalMV)
                .bind_uniform("MVP", proj * MV);

            mesh->render(pass1);
        }
        pass1.end_frame();

        pass2
            .begin_default_frame()
            .clear_frame()
            .apply_pipeline(pipe_id2)
            .apply_shader(shd_id2)
            //.bind_uniform_block("color", fay::memory{ (uint8_t*)&paras, sizeof(render_paras) })
            .bind_texture(frame[0], "gPosition") // TODO
            .bind_texture(frame[1], "gNormal")
            .bind_texture(frame[2], "gAlbedoSpec")
            .bind_uniform("MVP", MVP)
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
        mesh2->render(pass2);
        pass2.end_frame();

        device->execute({ pass1, pass2 });
    }
};

// defered_lighting

// tile_based_deferred_shading

// clustered_based_deferred_shading

SAMPLE_RENDER_APP_IMPL(defered_shading)