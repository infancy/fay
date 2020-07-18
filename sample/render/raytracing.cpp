#include "sample_render_app.h"

;


class raytracing_triangle_ : public passes
{
public:
    fay::frame raytracing_frame;
    fay::command_list pass1;
    glm::mat4 mvp{};

public:
    // using fay::app;
    raytracing_triangle_(const fay::app_desc& _desc) : passes(_desc) { desc.window.title = "camera_"; }

    void setup() override
    {
        add_update_items();

        mesh = fay::create_raw_renderable(fay::Box, device.get());
        //mesh2 = fay::create_renderable(fay::Plants, device.get());

        fay::buffer_id vertex_id1, index_id1;
        fay::shader_id raytracing_shd_id;
        {
            float vertices[] =
            {
                 -0.5f,  0.5f, 0.f,   0.f, 0.f,
                  0.5f,  0.5f, 0.f,   1.f, 0.f,
                  0.5f, -0.5f, 0.f,   1.f, 1.f,
                 -0.5f, -0.5f, 0.f,   0.f, 1.f,
            };
            fay::buffer_desc bd;
            {
                bd.type = fay::buffer_type::vertex;
                bd.btsz = 96;
                bd.data = vertices;
                bd.layout =
                {
                    {fay::attribute_usage::position,  fay::attribute_format::float3},
                    {fay::attribute_usage::texcoord0, fay::attribute_format::float2},
                };
            }
            vertex_id1 = device->create(bd);


            // float indices[]...
            int indices[] =
            {
                0, 1, 2,
                0, 2, 3
            };
            fay::buffer_desc index;
            {
                index.type = fay::buffer_type::index;
                index.btsz = 24;
                index.data = indices;
            }
            index_id1 = device->create(index);


            fay::shader_desc raytracing_sd = fay::create_shader_desc("raytracing_triangle", desc.render.backend, "shader/raytracing/raytracing_triangle");
            raytracing_sd.layout = bd.layout;
            raytracing_shd_id = device->create(raytracing_sd);
        }

        fay::buffer_id uniform_id;
        {
            fay::buffer_desc uniform_desc;
            uniform_desc.type = fay::buffer_type::uniform_cbv;
            uniform_desc.btsz = 64;
            uniform_desc.usage = fay::resource_usage::stream;
            uniform_id = device->create(uniform_desc);
        }

        fay::image img0("texture/awesomeface2.png", true);
        auto tex_id0 = create_2d(this->device, "hello", img0); // device->/~/:/create_texture2d("hello", img0);

        fay::respack_desc res{};
        res.vertex = vertex_id1;
        res.index = index_id1;
        res.textures.push_back(tex_id0);
        res.uniforms.push_back(uniform_id);
        auto res_id = device->create(res);

        /*
        fay::shader_desc raytracing_sd = fay::create_shader_desc("raytracing_triangle", desc.render.backend, "shader/raytracing/raytracing_triangle");
        raytracing_sd.layout =
        {
            { fay::attribute_usage::position, fay::attribute_format::float3 },
            { fay::attribute_usage::normal, fay::attribute_format::float3 },
            { fay::attribute_usage::texcoord0, fay::attribute_format::float2 },
            { fay::attribute_usage::tangent, fay::attribute_format::float3 },
            { fay::attribute_usage::bitangent, fay::attribute_format::float3 },
        };
        auto raytracing_shd_id = device->create(raytracing_sd);
        */
        fay::pipeline_desc raytracing_pd;
        raytracing_pd.type = fay::pipeline_type::raytracing;
        auto raytracing_pipe_id = device->create(raytracing_pd);

        raytracing_frame = fay::create_frame(device.get(), "offscreen_frm", 1920, 1080);



        fay::shader_desc sd = fay::create_shader_desc("raytracing_triangle", desc.render.backend, "shader/raytracing/raytracing_triangle");
        sd.layout = 
        {
            { fay::attribute_usage::position, fay::attribute_format::float3 },
            { fay::attribute_usage::normal, fay::attribute_format::float3 },
            { fay::attribute_usage::texcoord0, fay::attribute_format::float2 },
            { fay::attribute_usage::tangent, fay::attribute_format::float3 },
            { fay::attribute_usage::bitangent, fay::attribute_format::float3 },
        };
        auto shd_id = device->create(sd);

        fay::pipeline_desc pd;
        pd.depth_enabled = false;
        auto pipe_id = device->create(pd);



        pass1
            .begin_default(raytracing_pipe_id, raytracing_shd_id)
            .bind_respack(res_id)
            //.build_accel(...)
            //.update_buffer(uniform_id, &mvp)
            .tracing_ray()
            .end_frame(); // also do present
            //.begin_default(pipe_id, shd_id)
            //.end_frame();

        /*
        pass2
            .begin_default(pipe, shd)
            .bind_textures({ frame[0] })
            .bind_uniform("bAlbedo", true)
            .bind_uniform("MVP", VP * model1).draw(mesh.get())
            .bind_uniform("MVP", VP * model2).draw(mesh.get())
            .bind_uniform("MVP", VP * model3).draw(mesh.get())
            .bind_uniform("MVP", VP * model)
            //.draw(mesh2.get())
            .end_frame();

        device->execute({ pass1, pass2 });
        */
    }

    void render() override
    {
        glm::mat4 model(1.f);
        auto model1 = glm::scale(model, glm::vec3(40.f, 40.f, 40.f));

        mvp = camera->world_to_ndc() * model1;

        device->execute(pass1);
    }
};


SAMPLE_RENDER_APP_IMPL(raytracing_triangle_)
