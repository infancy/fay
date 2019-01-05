#pragma once

#include <glm/glm.hpp>

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
