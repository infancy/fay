#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "fay/app/app.h"
#include "fay/core/config.h"
#include "fay/core/fay.h"
#include "fay/render/device.h"
#include "fay/render/shader.h"
#include "fay/resource/image.h"


#define glcheck_errors() CHECK(glGetError() == GL_NO_ERROR)

fay::texture_id create_2d(fay::render_device_ptr& device, const std::string& name, const fay::image& img)
{
    fay::texture_desc desc;

    desc.name = name;
    desc.width = img.width();
    desc.height = img.height();
    desc.pixel_format = img.format();

    desc.size = img.size() * img.channel();
    desc.data = { img.data() };

    desc.type = fay::texture_type::two;

    return device->create(desc);
}

class clear : public fay::app
{
public:
	clear(const fay::app_desc& desc) : fay::app(desc)
	{
		desc_.window.title = "clear";
	}

    void setup() override
    {
    }

	void update() override
	{
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glcheck_errors();
    }
};

struct render_paras
{
    glm::vec4 a{};
    glm::vec4 b{};
};

class triangle : public fay::app
{
public:
    // using fay::app;
    triangle(const fay::app_desc& desc) : fay::app(desc)
    {
        desc_.window.title = "triangle";
    }

    void setup() override
    {
        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        float lines[] = {
            -0.3f,  0.9f, 0.0f,
             0.3f,  0.9f, 0.0f,
             0.3f, -0.9f, 0.0f,
             0.3f, -0.9f, 0.0f,
            -0.3f, -0.9f, 0.0f,
            -0.3f,  0.9f, 0.0f,
        };
        fay::buffer_desc bd0; {
            bd0.name = "line_stripe_vb";
            bd0.size = 6;// sizeof(vertices);
            bd0.stride = 12; // TODO: do it by helper functions;
            bd0.data = lines;
            bd0.type = fay::buffer_type::vertex;

            bd0.layout = { {fay::attribute_usage::position, fay::attribute_format::float3} };
        }
        auto line_strip_vb = render->create(bd0);



        float vertices[] = {
             0.6f,  0.45f, 0.0f,   1.f, 1.f, // right top
             0.6f, -0.45f, 0.0f,   1.f, 0.f, // right bottom
            -0.6f, -0.45f, 0.0f,   0.f, 0.f, // left bottom
            -0.6f,  0.45f, 0.0f,   0.f, 1.f, // left top
        };
        unsigned int indices[] = {  // note that we start from 0!
            0, 1, 3,  // first Triangle
            1, 2, 3   // second Triangle
        };
        fay::buffer_desc bd; {
            bd.name = "triangle_vb";
            bd.size = 4;// sizeof(vertices);
            bd.stride = 20; // TODO: do it by helper functions;
            bd.data = vertices;
            bd.type = fay::buffer_type::vertex;

            bd.layout = 
            { 
                {fay::attribute_usage::position,  fay::attribute_format::float3},
                {fay::attribute_usage::texcoord0, fay::attribute_format::float2}
            };
        }
        fay::buffer_desc id(fay::buffer_type::index); {
            id.name = "triangle_ib";
            id.size = 6;
            id.data = indices;
        }
        auto triangle_vb = render->create(bd);
        auto triangle_ib = render->create(id);

        fay::image img("texture/awesomeface.png", true);
        auto triangle_tbo = create_2d(this->render, "hello", img);

        auto vs_code = R"(
                #version 330 core
                layout (location = 0) in vec3 mPos;
                layout (location = 1) in vec2 mTex;

                out vec2 vTex;

                void main()
                {
                   gl_Position = vec4(mPos.x, mPos.y, mPos.z, 1.0);
                   vTex = mTex;   
                }
            )";
        auto fs_code = R"(
                #version 330 core
                in vec2 vTex;
                out vec4 FragColor;

                uniform sampler2D Diffuse;

                layout (std140) uniform color
                {
                    vec4 a;
                    vec4 b;
                };
                
                uniform int flag;

                void main()
                {
                   if(flag == 1)
                       FragColor = texture(Diffuse, vTex);
                   else
                       FragColor = texture(Diffuse, vTex);

                   //FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
                }
            )";
        fay::shader_desc sd = fay::scan_shader_program(vs_code, fs_code, true);
        sd.name = "shd"; //todo
        auto shd_id = render->create(sd);

        fay::pipeline_desc pd; 

        {
            pd.name = "line_strip";
            pd.primitive_type = fay::primitive_type::line_strip;
        }
        auto pipe_id = render->create(pd);

        {
            pd.name = "triangles";
            pd.primitive_type = fay::primitive_type::triangles;
        }
        auto pipe2_id = render->create(pd);

        paras.a = { 1.f, 0.f, 0.f, 1.f };
        paras.b = { 0.f, 1.f, 0.f, 1.f };

        {
            pass1
                .begin_default_frame()
                .clear_frame()
                .apply_shader(shd_id)
                .bind_uniform_block("color", fay::memory{ (uint8_t*)&paras, sizeof(render_paras) })

                //.apply_pipeline(pipe_id)
                //.bind_vertex(line_strip_vb)
                //.bind_uniform("flag", 1)
                //.draw()

                .apply_pipeline(pipe2_id)
                .bind_texture({ triangle_tbo })
                .bind_index(triangle_ib)
                .bind_vertex(triangle_vb)
                .bind_uniform("flag", 0)
                .draw_index()

                .end_frame();
        }

        // render->submit(pass1);
        // render->submit(pass2);
        // device->complie(pass);

        // uncomment this call to draw in wireframe polygons.
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    void update() override
    {
       // render->draw(pass1);
       // render->draw(pass2);
        auto cmds = pass1;
        render->submit(pass1);
        render->execute();
    }

    fay::buffer_id buf_id;

    render_paras paras;
    fay::command_list pass1, pass2;
};

int main(int argc, char** argv)
{
    google::InitGoogleLogging(argv[0]);
    //--stderrthreshold=0 --logtostderr=true
    //FLAGS_logtostderr = true;
    //FLAGS_stderrthreshold = 0;
    //FLAGS_v = 2;

    // vector<{XX_desc, XX}> ...;
    fay::app_desc desc;
    {
        //
    }
    triangle app_{ desc };
    return app_.run();
}