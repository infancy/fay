#include "fay/app/app.h"
#include "fay/core/config.h"
#include "fay/render/device.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#define gl_check_errors()  CHECK(glGetError() == GL_NO_ERROR)

const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";
const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\n\0";


class clear : public fay::app
{
public:
	clear(const fay::app_desc& desc) : fay::app(desc)
	{
		desc_.window.title = "clear";
	}

	void update() override
	{
		// render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}
};

fay::app_desc triangle_desc;
class triangle : public fay::app
{
public:
    triangle(const fay::app_desc& desc) : fay::app(desc)
    {
        desc_.window.title = "triangle";
    }

    void init() override
    {
        render = fay::render::create_device_opengl(fay::g_config);

        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        float vertices[] = {
             0.5f,  0.5f, 0.0f,  // top right
             0.5f, -0.5f, 0.0f,  // bottom right
            -0.5f, -0.5f, 0.0f,  // bottom left
            -0.5f,  0.5f, 0.0f   // top left
        };
        unsigned int indices[] = {  // note that we start from 0!
            0, 1, 3,  // first Triangle
            1, 2, 3   // second Triangle
        };

        fay::render::buffer_desc bd; {
            bd.size = 4;// sizeof(vertices);
            bd.stride = 12; // TODO: do it by helper functions;
            bd.data = vertices;
            bd.type = fay::render::buffer_type::vertex;

            bd.layout = { {fay::render::attribute_usage::position, fay::render::attribute_format::float3} };
        }
        auto vertex_id = render->create(bd);

        fay::render::buffer_desc bd2; {
            bd2.size = 6;
            bd2.data = indices;
        }
        auto index_id = render->create(bd2);

        fay::render::shader_desc sd; {
            sd.vs = vertexShaderSource;
            sd.fs = fragmentShaderSource;
        }
        auto shd_id = render->create(sd);

        fay::render::pipeline_desc pd; {
            pd.primitive_type = fay::render::primitive_type::line_strip;
        }
        auto pipe_id = render->create(pd);

        fay::render::pass_desc default_pass;
        {
           // default_pass.index = index_id;
            default_pass.buffers[0] = vertex_id;
            default_pass.shd_id = shd_id;
            default_pass.pipe_id = pipe_id;
        }
        pass = default_pass;

        // uncomment this call to draw in wireframe polygons.
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    void update() override
    {
        render->draw(pass);
    }

    void clear() override
    {

    }

    fay::render::pass_desc pass;
    fay::render::render_device_ptr render{};
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