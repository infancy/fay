#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "fay/app/input.h"
#include "fay/core/fay.h"
#include "fay/math/math.h"

namespace fay
{

// axis aligned bounding box

enum class bounds_corner : uint32_t 
{ 
 // octant           (x, y, z)
    I    = 0b111, // (+, +, +) max
    II   = 0b011, // (-, +, +)
    III  = 0b001, // (-, -, +)
    IV   = 0b101, // (+, -, +)

    V    = 0b110, // (+, +, -)
    VI   = 0b010, // (-, +, -)
    VII  = 0b000, // (-, -, -) min
    VIII = 0b100, // (+, -, -)
};

// TODO: move to math.transform
struct transform
{
    glm::vec3 position{ 0.f };
    //quat rotation{ 1.f, 0.f, 0.f, 0.f };
    glm::vec3 scale{ 1.f };

    float speed_ = 2.f;

    glm::mat4 model_matrix() const
    {
        glm::mat4 model{ 1.f };

        model = glm::translate(model, this->position);
        model = glm::scale(model, this->scale);

        return model;
    }

    bool on_input_event(const fay::single_input& io)
    {
        // if(active)

        if (io['w']) position.z -= io.delta_time * speed_;
        if (io['s']) position.z += io.delta_time * speed_;
        if (io['a']) position.x -= io.delta_time * speed_;
        if (io['d']) position.x += io.delta_time * speed_;
        if (io.left_down) position.y += io.delta_time * speed_;
        if (io.right_down) position.y -= io.delta_time * speed_;

        if (scale.x <= 1.f)
            scale -= glm::vec3(0.1f, 0.1f, 0.1f) * glm::vec3(io.wheel);
        else
            scale -= glm::vec3(1.f, 1.f, 1.f) * glm::vec3(io.wheel);

        if (scale.x < 0.f)
            scale = glm::vec3(0.1f, 0.1f, 0.1f);
        else if (scale.x > 10.f)
            scale = glm::vec3(10.f, 10.f, 10.f);
        //std::clamp(scale, glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(10.f, 10.f, 10.f));

        return true;
    }
};

class bounds2
{

};

// template <typename T>
class bounds3
{
public:
    bounds3() 
    {
        min_ = { MaxFloat, MaxFloat, MaxFloat };

        max_ = { MinFloat, MinFloat, MinFloat };
    }

    bounds3(glm::vec3 p)
        : min_(p), max_(p)
    {
    }

    bounds3(glm::vec3 p0, glm::vec3 p1)
    {
        min_ = mins(p0, p1);
        max_ = maxs(p0, p1);
    }

    const glm::vec3& min() const { return min_; }
    const glm::vec3& max() const { return max_; }

    glm::vec3 center() const { return coord(glm::vec3(0.5f)); }

    float radius() const { return 0.5f * glm::distance(min_, max_); }

    // WARNNING: 0~7, not 1~8
    // TODO: enum class bound_corner { ... };
    glm::vec3 corner(bounds_corner corner) const
    {
        uint32_t i = static_cast<uint32_t>(corner);
        DCHECK(i < 8);

        float x = (i & 0b100) ? max_.x : min_.x;
        float y = (i & 0b010) ? max_.y : min_.y;
        float z = (i & 0b001) ? max_.z : min_.z;
        return glm::vec3(x, y, z);
    }

    // WARNNING: if 0.f < p < 1.f, the return value is inside of the box
    glm::vec3 coord(glm::vec3 p) const
    {
        return min_ + (max_ - min_) * p; // return lerp(min_, max_, p);
    }

    void expand(const glm::vec3& p)
    {
        min_ = mins(min_, p);
        max_ = maxs(max_, p);
    }

    void expand(const bounds3& bounds3)
    {
        // glm::min, glm::max???
        min_ = mins(min_, bounds3.min_);
        max_ = maxs(max_, bounds3.max_);
    }

    bounds3 transform(const glm::mat4& m) const
    {
        bounds3 bounds;

        for (uint32_t i = 0; i < 8; i++)
        {
            glm::vec3 c = corner(bounds_corner(i));
            glm::vec4 t = m * glm::vec4(c, 1.0f);
            glm::vec3 v = { t.x, t.y, t.z };

            bounds.expand(v);
        }

        return bounds;
    }

private:
    glm::vec3 mins(const glm::vec3 v0, const glm::vec3 v1) const { return { std::min(v0.x, v1.x), std::min(v0.y, v1.y), std::min(v0.z, v1.z) }; }
    glm::vec3 maxs(const glm::vec3 v0, const glm::vec3 v1) const { return { std::max(v0.x, v1.x), std::max(v0.y, v1.y), std::max(v0.z, v1.z) }; }

private:
    glm::vec3 min_{};
    glm::vec3 max_{};
};



} // namespace fay
