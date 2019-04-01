#pragma once
#include "fay/app/app_flexible.h"

#define SAMPLE_RENDER_APP_DECL( class_name ) \
class class_name; \
std::unique_ptr<fay::app> create_##class_name##_app(const fay::app_desc& desc);

#define SAMPLE_RENDER_APP_IMPL( class_name ) \
class class_name; \
std::unique_ptr<fay::app> create_##class_name##_app(const fay::app_desc& desc) \
{ \
    return std::make_unique<class_name>(desc); \
}

/*
#define SAMPLE_RENDER_APP( class_name )
#ifdef SAMPLE_RENDER_MAIN
    SAMPLE_RENDER_APP_DECL(class_name)
#else
    SAMPLE_RENDER_APP_IMPL(class_name)
#endif
*/

#define CREATE_SAMPLE_RENDER_APP( class_name, desc ) create_##class_name##_app(desc)


class passes : public fay::app
{
public:
    fay::camera cameras_[3]
    {
        fay::camera{ glm::vec3{ 0, 20, -80 }, 90, 0, 1.f, 300.f }, // look at the positive-z axis

        //pitch 0 -> -30
        fay::camera{ glm::vec3{ -300, 300, 0 }, /*-180*/0, -45, 1.f, 1000.f }, // look at the positive-x axis
    };
    fay::light lights_[3]
    {
        fay::light{ glm::vec3{ -300, 300, 0 } },
        fay::light{ glm::vec3{ 100, 100, 0 } },
        fay::light{ glm::vec3{ 0, 100, -100 } },
    };
    fay::transform transforms_[2]
    {
        fay::transform{ glm::vec3{ 0, 0, 0 } },
        fay::transform{ glm::vec3{ -100, 0, 0 } },
    };
    const fay::camera* camera{ cameras_ };
    const fay::light* light{ lights_ };
    const fay::transform* transform{ transforms_ };

    fay::shader_id debug_shd;
    fay::pipeline_id debug_pipe;

    fay::buffer_id vbo;
    fay::buffer_id ibo;

    fay::renderable_sp mesh;
    fay::renderable_sp mesh2;
    fay::renderable_sp mesh3;
    fay::texture_id tex;
    fay::texture_id tex2;

    fay::shader_id shd;
    fay::shader_id shd2;
    fay::pipeline_id pipe;
    fay::pipeline_id pipe2;

    fay::frame frame;
    fay::frame frame2;
    fay::frame frame3;
    fay::frame frame4;

    //fay::command_list pass1, pass2;

public:
    using fay::app::app;

    void add_update_items()
    {
        add_update_items_
        (
            { cameras_, cameras_ + 1 },
            { lights_, lights_ + 1 },
            { transforms_, transforms_ + 1 }
        );
    }

    void debug_setup()
    {
        fay::shader_desc sd = fay::scan_shader_program("light_shd", "gfx/lines.vs", "gfx/lines.fs", desc.render.backend);
        debug_shd = device->create(sd);

        fay::pipeline_desc pd;
        {
            pd.name = "light_pipe";
            pd.stencil_enabled = false;
            pd.primitive_type = fay::primitive_type::lines;
            debug_pipe = device->create(pd);
        }
    }

    // TODO: merge to fay::app
    void update(const fay::single_input& io) override
    {
        static size_t current_mode_{};
        static size_t current_items_[3]{};

        if (io[' ']) current_mode_ = ++current_mode_ % 3;
        if (io['z']) current_mode_ = 0;
        if (io['x']) current_mode_ = 1;
        if (io['c']) current_mode_ = 2;

        //if (io['0']) current_item_ = 0;
        if (io['1']) current_items_[current_mode_] = 0;
        if (io['2']) current_items_[current_mode_] = 1;
        if (io['3']) current_items_[current_mode_] = 2;

        camera = cameras_ + current_items_[0];
        light = lights_ + current_items_[1];
        transform = transforms_ + current_items_[2];
    }
};
