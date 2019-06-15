#pragma once

#include "fay/core/fay.h"
#include "fay/math/geometry.h"

// Physical Lights in UE4: https://zhuanlan.zhihu.com/p/69248316

namespace fay
{

class light
{
public:
    light(glm::vec3 position)
        : position_{ position }
    {
    }

    glm::vec3 position() const
    {
        return position_;
    }

    glm::mat4 model_matrix() const
    {
        glm::mat4 model{ 1.f };

        model = glm::translate(model, position_);

        return model;
    }

    bool on_input_event(const fay::single_input& io)
    {
        // if(active)
        speed_ -= io.wheel;
        if (speed_ <= 0.f)
            speed_ = 0.f;
        else if (speed_ >= 10.f)
            speed_ = 10.f;

        if (io.key['w']) position_.z -= io.delta_time * speed_;
        if (io.key['s']) position_.z += io.delta_time * speed_;
        if (io.key['a']) position_.x -= io.delta_time * speed_;
        if (io.key['d']) position_.x += io.delta_time * speed_;
        if (io.left_down) position_.y += io.delta_time * speed_;
        if (io.right_down) position_.y -= io.delta_time * speed_;

        return true;
    }

private:
    glm::vec3 position_{}; // world space light position
    float speed_{ 10.f };
};

/*
class light : public fay::renderable
{
public:
    void setup(fay::render_device* device)
    {
        light_mesh = fay::create_raw_renderable(fay::Box, device);

        fay::shader_desc sd = fay::scan_shader_program("gfx/renderable.vs", "gfx/renderable.fs");
        sd.name = "light_shd";
        light_shd_id = device->create(sd);

        fay::pipeline_desc pd;
        {
            pd.name = "light_pipe";
            pd.stencil_enabled = false;

            light_pipe_id = device->create(pd);
        }
    }

    void update(glm::mat4 VP, glm::vec3 position_)
    {
        glm::mat4 model = glm::mat4(1.f);
        model = glm::translate(model, position_);

        this->MVP = VP * model;
    }

    void render(fay::command_list& cmd) override
    {
        cmd
            .apply_pipeline(light_pipe_id)
            .apply_shader(light_shd_id)
            .bind_uniform("MVP", MVP)
            .bind_uniform("bAlbedo", false)
            .draw(light_mesh.get());
    }

    bool on_input_event(const fay::single_input& io)
    {
        // if(active)
        scale_ -= glm::vec3(0.1f, 0.1f, 0.1f) * glm::vec3(io.wheel);
        if (scale_.x < 0.f)
            scale_ = glm::vec3(0.1f, 0.1f, 0.1f);
        else if (scale_.x > 1.f)
            scale_ = glm::vec3(1.f, 1.f, 1.f);

        speed_ -= io.wheel;
        if (speed_ <= 0.f)
            speed_ = 0.f;
        else if (speed_ >= 10.f)
            speed_ = 10.f;

        if (io.key['w']) position_.z -= io.delta_time * speed_;
        if (io.key['s']) position_.z += io.delta_time * speed_;
        if (io.key['a']) position_.x -= io.delta_time * speed_;
        if (io.key['d']) position_.x += io.delta_time * speed_;
        if (io.left_down) position_.y += io.delta_time * speed_;
        if (io.right_down) position_.y -= io.delta_time * speed_;

        return true;
    }

private:
    fay::renderable_sp light_mesh;
    fay::shader_id light_shd_id;
    fay::pipeline_id light_pipe_id;

    glm::vec3 position_ = glm::vec3(15, 25, 0); // world space light position
    float speed_ = 2.f;
    glm::vec3 scale_{ 0.5f, 0.5f, 0.5f };
    glm::mat4 MVP;
};
*/

} // namespace fay