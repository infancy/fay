#include "sample_render_app.h"

//fay::app_desc clear_desc = global_desc;
//clear_desc.window.title = "clear";

class init : public fay::app
{
public:
    init(const fay::app_desc& _desc) : fay::app(_desc)
    {
        desc.window.title = "init";
    }
};

// swapchian, clear
class clear : public fay::app
{
public:
    uint8_t color = 0;
    int counter = 1;

public:
    clear(const fay::app_desc& _desc) : fay::app(_desc)
    {
        desc.window.title = "clear";
    }

    void setup() override
    {
    }

    void render() override
    {
        color += counter;
        if (color == 0 or color == 255) counter = -counter;

        float red = float(color) / float(255);

        fay::command_list cmd;
        cmd
            .begin_default_frame()
            //.set_viewport(360, 240, 720, 480)
            //.set_scissor(360, 240, 720, 480) // not valid in D3D11/D3D12?
            .clear_color(red, 0.f, 0.f, 1.f)
            .end_frame();

        device->execute(cmd);
    }
};

//#define test_pipeline
class shader_pipeline_ : public fay::app
{
public:
    fay::command_list pass1;

public:
    // using fay::app;
    shader_pipeline_(const fay::app_desc& _desc) : fay::app(_desc)
    {
        desc.window.title = "shader_pipeline_";
    }

    void setup() override
    {
        fay::buffer_desc bd;
        {
            bd.type = fay::buffer_type::vertex;
            bd.btsz = 36; // can't be zero
            bd.usage = fay::resource_usage::dynamic;
            bd.layout =
            {
                {fay::attribute_usage::position,  fay::attribute_format::float3},
            };
        }
        auto buf_id = device->create(bd);

        fay::shader_desc sd = fay::create_shader_desc("default", desc.render.backend, "shader/base/shader");
        sd.layout = bd.layout;
        auto shd_id = device->create(sd);

        fay::pipeline_desc pd;
#ifndef test_pipeline
        pd.primitive_type = fay::primitive_type::triangles;
        pd.cull_mode = fay::cull_mode::none;
#else
        pd.primitive_type = fay::primitive_type::triangles;
        pd.cull_mode = fay::cull_mode::front; // cw
        pd.fill_mode = fay::fill_mode::wireframe;
#endif
        pd.depth_enabled = false;
        auto pipe_id = device->create(pd);

        pass1
            .begin_default(pipe_id, shd_id)
            .bind_vertex(buf_id)
            .draw(6)
            .end_frame();
    }

    void render() override
    {
        device->execute(pass1);
    }
};

#define test_index
class vertex_index_ : public fay::app
{
public:
    fay::command_list pass1;

public:
    // using fay::app;
    vertex_index_(const fay::app_desc& _desc) : fay::app(_desc)
    {
        desc.window.title = "vertex_index_";
    }

    void setup() override
    {
        fay::buffer_id vertex_id0, vertex_id1, index_id1;
        fay::shader_id shd_id0, shd_id1;

        {
            float vertices[] =
            {
                -0.50f,  0.25f, 0.0f,
                -0.75f, -0.25f, 0.0f,
                 0.75f, -0.25f, 0.0f,
            };
            fay::buffer_desc bd;
            {
                bd.type = fay::buffer_type::vertex;
                bd.btsz = 48;
                bd.data = vertices;
                bd.layout =
                {
                    {fay::attribute_usage::position, fay::attribute_format::float3},
                };
            }
            vertex_id0 = device->create(bd);

            fay::shader_desc sd = fay::create_shader_desc("default", desc.render.backend, "shader/base/vertex");
            sd.layout = bd.layout;
            shd_id0 = device->create(sd);
        }

#ifdef test_index
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


            fay::shader_desc sd = fay::create_shader_desc("default", desc.render.backend, "shader/base/index");
            sd.layout = bd.layout;
            shd_id1 = device->create(sd);
        }
#endif

        fay::pipeline_desc pd;
        pd.primitive_type = fay::primitive_type::triangles;
        pd.cull_mode = fay::cull_mode::none;
        pd.depth_enabled = false;
        auto pipe_id = device->create(pd);

        pass1
            //.begin_default(pipe_id, shd_id)

            .begin_default_frame()
            .clear()
            .apply_pipeline(pipe_id)
            .apply_shader(shd_id0)
            .bind_vertex(vertex_id0)
            .draw(3, 0)
        #ifdef test_index
            .apply_shader(shd_id1)
            .bind_vertex(vertex_id1)
            .bind_index(index_id1)
            .draw_index(6, 0)
        #endif
            .end_frame();
    }

    void render() override
    {
        device->execute(pass1);
    }
};

//texture, uniform, respack
//#define test_texture
//#define test_uniform
//#define test_respack
class texture_uniform_ : public fay::app
{
public:
    fay::command_list pass1;

    // TODO: animation_curve, graph
    uint8_t color = 0;
    int counter = 1;

    glm::vec4 offset{};

public:
    // using fay::app;
    texture_uniform_(const fay::app_desc& _desc) : fay::app(_desc)
    {
        desc.window.title = "texture_uniform_";
    }

    void setup() override
    {
        fay::buffer_id vertex_id1, index_id1;
        fay::shader_id shd_id1;


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
            bd.btsz = 80;
            bd.data = vertices;
            bd.layout =
            {
                {fay::attribute_usage::position,  fay::attribute_format::float3},
                {fay::attribute_usage::texcoord0, fay::attribute_format::float2},
            };
        }
        vertex_id1 = device->create(bd);


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

        fay::buffer_id uniform_id0, uniform_id1;
        {
            fay::buffer_desc uniform_desc;
            uniform_desc.type = fay::buffer_type::uniform_cbv;
            uniform_desc.btsz = 16;
            uniform_desc.usage = fay::resource_usage::stream;
            uniform_id0 = device->create(uniform_desc);
        }
        {
            fay::buffer_desc uniform_desc;
            uniform_desc.type = fay::buffer_type::uniform_cbv;
            uniform_desc.btsz = 16;
            uniform_desc.usage = fay::resource_usage::stream;
            uniform_id1 = device->create(uniform_desc);
        }


        fay::image img0("texture/awesomeface2.png", true);
        auto tex_id0 = create_2d(this->device, "hello", img0);


        fay::image img1("texture/grass.png", true);
        auto tex_id1 = create_2d(this->device, "hello", img1);


#pragma region customization point
        fay::respack_desc res{};
        res.textures.push_back(tex_id0);
        res.textures.push_back(tex_id1);
        res.uniforms.push_back(uniform_id0);
        res.uniforms.push_back(uniform_id1);
        auto res_id = device->create(res);


        //fay::shader_desc sd = fay::create_shader_desc("default", desc.render.backend, "shader/base/texture");
        //fay::shader_desc sd = fay::create_shader_desc("default", desc.render.backend, "shader/base/uniform");
        fay::shader_desc sd = fay::create_shader_desc("default", desc.render.backend, "shader/base/respack");
        sd.layout = bd.layout;
        shd_id1 = device->create(sd);
#pragma endregion

        fay::pipeline_desc pd;
        pd.primitive_type = fay::primitive_type::triangles;
        pd.cull_mode = fay::cull_mode::none;
        pd.depth_enabled = false;
        auto pipe_id = device->create(pd);

        pass1
            .begin_default(pipe_id, shd_id1)
            .bind_respack(res_id)
            .update_buffer(uniform_id0, &offset)
            .update_buffer(uniform_id1, &offset)
            .bind_vertex(vertex_id1)
            .bind_index(index_id1)
            .draw_index(6, 0)
            .end_frame();
    }

    void render() override
    {
        color += counter;
        if (color == 0 or color == 255) counter = -counter;
        offset.x = float(color) / float(255);

        device->execute(pass1);
    }
};



class camera_ : public passes
{
public:
    // using fay::app;
    camera_(const fay::app_desc& _desc) : passes(_desc)
    {
        desc.window.title = "camera_";
    }

    void setup() override
    {
        add_update_items();

        mesh = fay::create_raw_renderable(fay::Box, device.get());
        //mesh2 = fay::create_renderable(fay::Plants, device.get());

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
        auto model2 = glm::translate(model1, glm::vec3(5.f, 0.f, 0.f)); // y, x, z
        auto model3 = glm::translate(model1, glm::vec3(5.f, -5.f, 0.f));

        auto VP = camera->world_to_ndc();

        fay::command_list pass1, pass2;

        pass1
            .begin_frame(frame)
            .clear_color({ 1.f, 0.f, 0.f, 1.f })
            .clear_depth().clear_stencil()
            .apply_pipeline(pipe).apply_shader(shd)
            .bind_textures({ tex })
            .bind_uniform("bAlbedo", true)
            .bind_uniform("MVP", VP * model1).draw(mesh.get())
            .bind_uniform("MVP", VP * model2).draw(mesh.get())
            .bind_uniform("MVP", VP * model3).draw(mesh.get())
            .end_frame();

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
    }
};

class offscreen_ : public passes
{
public:
    // using fay::app;
    offscreen_(const fay::app_desc& _desc) : passes(_desc)
    {
        desc.window.title = "offscreen_";
    }

    void setup() override
    {
        add_update_items();

        mesh = fay::create_raw_renderable(fay::Box, device.get());
        //mesh2 = fay::create_renderable(fay::Plants, device.get());

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
        auto model2 = glm::translate(model1, glm::vec3(5.f, 0.f, 0.f)); // y, x, z
        auto model3 = glm::translate(model1, glm::vec3(5.f, -5.f, 0.f));

        auto VP = camera->world_to_ndc();

        fay::command_list pass1, pass2;

        pass1
            .begin_frame(frame)
            .clear_color({ 1.f, 0.f, 0.f, 1.f })
            .clear_depth().clear_stencil()
            .apply_pipeline(pipe).apply_shader(shd)
            .bind_textures({ tex })
            .bind_uniform("bAlbedo", true)
            .bind_uniform("MVP", VP * model1).draw(mesh.get())
            .bind_uniform("MVP", VP * model2).draw(mesh.get())
            .bind_uniform("MVP", VP * model3).draw(mesh.get())
            .end_frame();

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
    }
};

// texture
class triangle : public fay::app
{
public:
    struct render_paras
    {
        glm::vec4 window{};
        int flag;
    };

    fay::buffer_id buf_id;

    render_paras paras1, paras2;
    fay::command_list pass1, pass2;

public:
    // using fay::app;
    triangle(const fay::app_desc& _desc) : fay::app(_desc)
    {
        desc.window.title = "triangle";
    }

    void setup() override
    {
        float vertices[] = 
        {
          // position             texture
             0.6f,  0.45f, 0.5f,  1.f, 0.f, // right top
             0.6f, -0.45f, 0.5f,  1.f, 1.f, // right bottom
            -0.6f, -0.45f, 0.5f,  0.f, 1.f, // left bottom
            -0.6f,  0.45f, 0.5f,  0.f, 0.f, // left top

            -0.85f, -0.95f, 0.5f,  1.f, 1.f, // right bottom
            -0.95f, -0.95f, 0.5f,  0.f, 1.f, // left bottom
            -0.90f,  0.95f, 0.5f,  0.f, 0.f, // left top
        };
        unsigned int indices[] = 
        {  // note that we start from 0!
            0, 1, 3,  // first Triangle
            1, 2, 3,   // second Triangle

            4, 5, 6
        };
        fay::buffer_desc bd; 
        {
            bd.name = "triangle_vb";
            bd.size = 7;// sizeof(vertices);
            bd.stride = 20; // TODO: do it by helper functions;
            bd.data = vertices;
            bd.type = fay::buffer_type::vertex;

            bd.layout =
            {
                {fay::attribute_usage::position,  fay::attribute_format::float3},
                {fay::attribute_usage::texcoord0, fay::attribute_format::float2}
            };
        }
        auto triangle_vb = device->create(bd);

        fay::buffer_desc id("triangle_ib", 9, indices);
        auto triangle_ib = device->create(id);

        fay::image img("texture/awesomeface.png", true);
        auto triangle_tbo = create_2d(this->device, "hello", img);



        //fay::shader_desc sd = fay::scan_shader_program("shd", "gfx/test/0_basic.vs", "gfx/test/0_basic.fs", desc.render.backend);
        //fay::shader_desc sd = fay::scan_shader_program("shd", "gfx/test/1_buffer.vs", "gfx/test/1_buffer.fs", desc.render.backend);
        //fay::shader_desc sd = fay::scan_shader_program("shd", "gfx/test/2_texture.vs", "gfx/test/2_texture.fs", desc.render.backend);
        fay::shader_desc sd = fay::scan_shader_program("shd", "gfx/test/3_uniform.vs", "gfx/test/3_uniform.fs", desc.render.backend);
        auto shd_id = device->create(sd);

        fay::pipeline_desc pd;
        pd.name = "triangles";
        pd.primitive_type = fay::primitive_type::triangles;
        pd.cull_mode = fay::cull_mode::none;
        auto pipe_id = device->create(pd);

        paras1.window = glm::vec4(0.f, 0.f, 1080.f, 720.f);
        paras1.flag = 1;

        paras2.window = glm::vec4(0.f, 0.f, 1080.f, 720.f);
        paras2.flag = 0;

        pass1
            .begin_default(pipe_id, shd_id)
            .bind_textures({ triangle_tbo })
            .bind_index(triangle_ib)
            .bind_vertex(triangle_vb)
            // TODO: rename bind_uniform/bind_buffer
            .bind_uniform_block("para", fay::memory{ (uint8_t*)&paras1, sizeof(render_paras) })
            .draw_index(6, 0)

            .bind_uniform_block("para", fay::memory{ (uint8_t*)&paras2, sizeof(render_paras) })
            .draw_index(3, 6)
            //.draw(6)
            .end_frame();
    }

    void render() override
    {
        device->execute(pass1);
    }
};

class instancing : public fay::app
{
public:
    fay::buffer_id buf_id;

    static const inline size_t nInstance = 2;
    glm::mat4 instance_transforms[nInstance]
    {
        fay::transform{ glm::vec3{ 0, 0, 0 } }.model_matrix(),
        fay::transform{ glm::vec3{ 0.5, 0, 0 } }.model_matrix(),
    };
    fay::buffer_id instance_transforms_id;

    fay::command_list pass1, pass2;

    instancing(const fay::app_desc& _desc) : fay::app(_desc)
    {
        desc.window.title = "instancing";
    }

    void setup() override
    {
        float vertices[] = 
        {
             0.6f,  0.45f, 0.5f,   0.f, 0.f, -1.f,   1.f, 1.f, // right top
             0.6f, -0.45f, 0.5f,   0.f, 0.f, -1.f,   1.f, 0.f, // right bottom
            -0.6f, -0.45f, 0.5f,   0.f, 0.f, -1.f,   0.f, 0.f, // left bottom
            -0.6f,  0.45f, 0.5f,   0.f, 0.f, -1.f,   0.f, 1.f, // left top
        };
        unsigned int indices[] = 
        {  // note that we start from 0!
            0, 1, 3,  // first Triangle
            1, 2, 3   // second Triangle
        };
        fay::buffer_desc bd; 
        {
            bd.name = "triangle_vb";
            bd.size = 4;// sizeof(vertices);
            bd.data = vertices;
            bd.type = fay::buffer_type::vertex;

            bd.layout =
            {
                {fay::attribute_usage::position,  fay::attribute_format::float3},
                {fay::attribute_usage::normal,    fay::attribute_format::float3},
                {fay::attribute_usage::texcoord0, fay::attribute_format::float2}
            };
            bd.stride = bd.layout.stride();
        }
        fay::buffer_desc id("triangle_ib", 6, indices);
        auto triangle_vb = device->create(bd);
        auto triangle_ib = device->create(id);

        {
            fay::buffer_desc desc;
            desc.name = "instance_model";
            desc.size = nInstance;
            desc.layout =
            {
                {fay::attribute_usage::instance_model,  fay::attribute_format::float4, 4},
            };
            desc.stride = desc.layout.stride();
            desc.data = &instance_transforms;
            desc.type = fay::buffer_type::instance;
            instance_transforms_id = device->create(desc);
        }

        fay::image img("texture/awesomeface.png", true);
        auto triangle_tbo = create_2d(this->device, "hello", img);

        fay::shader_desc sd = fay::scan_shader_program("shd", "gfx/29_instancing.vs", "gfx/29_instancing.fs", desc.render.backend);
        auto shd_id = device->create(sd);

        fay::pipeline_desc pd;
        pd.name = "triangles";
        pd.primitive_type = fay::primitive_type::triangles;
        pd.cull_mode = fay::cull_mode::none;
        auto pipe_id = device->create(pd);

        pass1
            .begin_default(pipe_id, shd_id)
            .bind_textures({ triangle_tbo })
            .bind_index(triangle_ib)
            .bind_vertex(triangle_vb)
            .bind_instance(instance_transforms_id, { fay::attribute_usage::instance_model })
            .draw_index(6, 0, 2)

            .end_frame();
    }

    void render() override
    {
        device->execute(pass1);
    }
};

// post_processing
class offscreen : public passes
{
public:
    // using fay::app;
    offscreen(const fay::app_desc& _desc) : passes(_desc)
    {
        desc.window.title = "post_proc";
    }

    void setup() override
    {
        add_update_items();

        mesh = fay::create_raw_renderable(fay::Box, device.get());
        //mesh2 = fay::create_renderable(fay::Plants, device.get());

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
        auto model2 = glm::translate(model1, glm::vec3(5.f, 0.f, 0.f)); // y, x, z
        auto model3 = glm::translate(model1, glm::vec3(5.f, -5.f, 0.f));

        auto VP = camera->world_to_ndc();

        fay::command_list pass1, pass2;

        pass1
            .begin_frame(frame)
            .clear_color({ 1.f, 0.f, 0.f, 1.f })
            .clear_depth().clear_stencil()
            .apply_pipeline(pipe).apply_shader(shd)
            .bind_textures({ tex })
            .bind_uniform("bAlbedo", true)
            .bind_uniform("MVP", VP * model1).draw(mesh.get())
            .bind_uniform("MVP", VP * model2).draw(mesh.get())
            .bind_uniform("MVP", VP * model3).draw(mesh.get())
            .end_frame();

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
    }
};

SAMPLE_RENDER_APP_IMPL(init)
SAMPLE_RENDER_APP_IMPL(clear)
SAMPLE_RENDER_APP_IMPL(shader_pipeline_)
SAMPLE_RENDER_APP_IMPL(vertex_index_)
SAMPLE_RENDER_APP_IMPL(texture_uniform_)
SAMPLE_RENDER_APP_IMPL(camera_)
SAMPLE_RENDER_APP_IMPL(offscreen_)

SAMPLE_RENDER_APP_IMPL(triangle)
SAMPLE_RENDER_APP_IMPL(instancing)
SAMPLE_RENDER_APP_IMPL(offscreen)


