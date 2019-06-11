#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>

#include "fay/app/input.h"
#include "fay/core/fay.h"
#include "fay/core/enum.h"
#include "fay/math/math.h"

namespace fay
{

// -------------------------------------------------------------------------------------------------

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

        if (io['w']) position.z += io.delta_time * speed_;
        if (io['s']) position.z -= io.delta_time * speed_;
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

// -------------------------------------------------------------------------------------------------

// class plane { vec3 point; vec3 normal; };

// -------------------------------------------------------------------------------------------------
// axis aligned bounding box

enum class box_corner : uint32_t
{
    // octant          (x, y, z)
    I    = 0b111, // 7 (+, +, +) max
    II   = 0b011, // 3 (-, +, +)
    III  = 0b001, // 1 (-, +, -)
    IV   = 0b101, // 5 (+, +, -)

    V    = 0b110, // 6 (+, -, +)
    VI   = 0b010, // 2 (-, -, +)
    VII  = 0b000, // 0 (-, -, -) min
    VIII = 0b100, // 4 (+, -, -)
};

enum class box_relation
{
    contain   = 0b001,
    intersect = 0b010,
    except    = 0b100,
    //associate = contain | intersect,
};

FAY_ENUM_CLASS_OPERATORS(box_relation)

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

    bounds3(float p0, float p1)
        : bounds3(glm::vec3(p0), glm::vec3(p1))
    {
    }

    bool is_vaild() const
    {
        return min_.x <= MaxFloat || max_.x >= MinFloat;
    }

    const glm::vec3& min() const { return min_; }
    const glm::vec3& max() const { return max_; }

    glm::vec3 center() const { return coord(glm::vec3(0.5f)); }

    float radius() const { return 0.5f * glm::distance(min_, max_); }

    // WARNNING: 0~7, not 1~8
    // TODO: enum class bound_corner { ... };
    glm::vec3 corner(box_corner corner) const
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

    glm::vec3 coord(float x, float y, float z) const
    {
        return coord({ x, y, z });
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
            glm::vec3 c = corner(box_corner(i));
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

// -------------------------------------------------------------------------------------------------

class frustum
{
public:
    // from Granite/math/frustum.cpp/Frustum::build_planes
    frustum(const glm::mat4& view_projection)
    {
        using namespace glm;

        inv_view_projection_ = glm::inverse(view_projection);

        // (D3D) NDC box corner point(top/bottom, left/right, near/far)
        static const vec4 trf(+1.f, +1.f, +1.f, 1.f);
        static const vec4 tlf(-1.f, +1.f, +1.f, 1.f);
        static const vec4 tln(-1.f, +1.f, +0.f, 1.f);
        static const vec4 trn(+1.f, +1.f, +0.f, 1.f);
        static const vec4 brf(+1.f, -1.f, +1.f, 1.f);
        static const vec4 blf(-1.f, -1.f, +1.f, 1.f);
        static const vec4 bln(-1.f, -1.f, +0.f, 1.f);
        static const vec4 brn(+1.f, -1.f, +0.f, 1.f);
        static const vec4 center(0.f, 0.f, 0.5f, 1.f);

        // normalize
        // const auto project = [](const vec4 &v) { return vec3(v.x / v.w, v.y / v.w, v.z / v.w); };

        //                                                                      v1
        //                                                                    /  |
        // transform to world space                                         /    |
        //                 v2----- v1           |y             v2         v0     |
        //    |y          /|      /|            |             / |         |      |
        //    |          v3------v0|            |            v3 |         |      |
        //    |     z    | |     | |            |     z      |  |         |      |
        //    .------    | |v6---|-|v5          .------      |  |v6       |      |
        //   /           |/      |/            /             | /          |     v5
        //  / x          v7------v4  NDC      / x     proj   v7           |   /
        //                                                                | /
        //                                                                v4
        vec3 TRF = corners_[0] = project(inv_view_projection_ * trf);
        vec3 TLF = corners_[1] = project(inv_view_projection_ * tlf);
        vec3 TLN = corners_[2] = project(inv_view_projection_ * tln);
        vec3 TRN = corners_[3] = project(inv_view_projection_ * trn);
        vec3 BRF = corners_[4] = project(inv_view_projection_ * brf);
        vec3 BLF = corners_[5] = project(inv_view_projection_ * blf);
        vec3 BLN = corners_[6] = project(inv_view_projection_ * bln);
        vec3 BRN = corners_[7] = project(inv_view_projection_ * brn);

        // the normal of each plane, pointing to the internal
        vec3 l = normalize(cross(BLF - BLN, TLN - BLN));
        vec3 r = normalize(cross(TRF - TRN, BRN - TRN));
        vec3 n = normalize(cross(BLN - BRN, TRN - BRN));
        vec3 f = normalize(cross(TRF - BRF, BLF - BRF));
        vec3 t = normalize(cross(TLN - TRN, TRF - TRN));
        vec3 b = normalize(cross(BRF - BRN, BLN - BRN));

        // 
        planes_[0] = vec4(l, -dot(l, BLN));
        planes_[1] = vec4(r, -dot(r, TRN));
        planes_[2] = vec4(n, -dot(n, BRN));
        planes_[3] = vec4(f, -dot(f, BRF));
        planes_[4] = vec4(t, -dot(t, TRN));
        planes_[5] = vec4(b, -dot(b, BRN));

        // 
        vec4 Center = inv_view_projection_ * center;
        // checks winding order
        for (auto &p : planes_)
            if (dot(Center, p) < 0.f)
                p = -p;
    }

    //bool extra_culling(const bounds3& bounds, box_relation relation = box_relation::contain) const

    // treat the bounds3 as a bounds_sphere
    // bool rough_expect(const bounds3& bounds) const

    bool expect(const bounds3& bounds) const
    {
        using namespace glm;

        for (auto &plane : planes_)
        {
            bool all_outside = true;

            for (size_t i = 0; i < 8; ++i)
            {
                if (dot(vec4(bounds.corner(box_corner(i)), 1.f), plane) >= 0.f)
                {
                    all_outside = false;
                    break;
                }
            }

            // only one is true, it's true
            if (all_outside)
                return true;
        }

        return false;
    }

    bool contain(const bounds3& bounds) const
    {
        using namespace glm;

        for (auto &plane : planes_)
        {
            bool has_outside = false;

            for (size_t i = 0; i < 8; ++i)
            {
                if (dot(vec4(bounds.corner(box_corner(i)), 1.f), plane) < 0.f)
                {
                    has_outside = true;
                    break;
                }
            }

            // only one is false, it's false
            if (has_outside)
                return false;
        }

        return true;
    }

    box_relation relation(const bounds3& bounds) const
    {
        if (expect(bounds))
            return box_relation::except;
        else if (contain(bounds))
            return box_relation::contain;
        else
            return box_relation::intersect;
    }

    // TODO: remove box_corner
    glm::vec3 corner(box_corner corner) const
    {
        static size_t map[8] =
        {
         // 1, 2, 3, 4,   5, 6, 7, 8

         // 0, 1, 2, 3,   4, 5, 6, 7
         // 7, 3, 1, 5,   6, 2, 0, 4
         // |
         // +----------------------+
         //                        |
         // 0, 1, 2, 3,   4, 5, 6, 7
            6, 2, 5, 1,   7, 3, 4, 0
        };

        uint32_t i = static_cast<uint32_t>(corner);
        DCHECK(i < 8);
        return corners_[ map[i] ];
    }

    // WARNNING: if 0.f < p < 1.f, the return value is inside of the box
    glm::vec3 coord(float x, float y, float z) const
    {
        // WARNNING: D3D NDC space
        glm::vec4 clip = glm::vec4(2.f * x - 1.f, 2.f * y - 1.f, z, 1.f);
        return project(inv_view_projection_ * clip);
    }

    glm::vec3 coord(glm::vec3 p) const
    {
        return coord(p.x, p.y, p.z);
    }

    glm::vec3 center() const
    {
        glm::vec3 p(0.5f);
        glm::vec3 near_center = glm::lerp(corner(box_corner::IV), corner(box_corner::VII), p);
        glm::vec3 far_center = glm::lerp(corner(box_corner::I), corner(box_corner::VI), p);
        return glm::lerp(near_center, far_center, p);
    }

    bounds3 bounds() const
    {
        bounds3 bounds(corner(box_corner::I), corner(box_corner::II));
        bounds.expand(corner(box_corner::III));
        bounds.expand(corner(box_corner::IV));
        bounds.expand(corner(box_corner::V));
        bounds.expand(corner(box_corner::VI));
        bounds.expand(corner(box_corner::VII));
        bounds.expand(corner(box_corner::VIII));

        return bounds;
    }

    // mainly used in CSM
    // 'ts':interpolation values, like 0.0, 0.05, 0.15, 1.0
    /*
    std::vector<std::array<glm::vec3, 8>> section_bounds(const std::vector<float> ts) const
    {
        DCHECK(ts.size() >= 2);

        std::vector<std::array<glm::vec3, 4>> faces(ts.size());
        faces.front() = { corners_[2], corners_[3], corners_[6], corners_[7] };
        faces.back()  = { corners_[0], corners_[1], corners_[4], corners_[5] };

        auto _near = faces.front();
        auto _far  = faces.back();

        for (size_t i = 1; i < ts.size() - 1; ++i)
        {
            auto& face = faces[i];

            for (size_t j = 0; j < 4; ++j)
                face[j] = glm::lerp(_near[j], _far[j], ts[i]);
        }

        auto sz = ts.size() - 1;
        std::vector<std::array<glm::vec3, 8>> corners(sz);

        for (size_t i = 0; i < sz; ++i)
        {
            auto _near = faces[i], _far = faces[i + 1];
            corners[i] = { _far[0], _far[1], _near[2], _near[3], _far[4], _far[5], _near[6], _near[7] };
        }

        return corners;
    }
    */

    //glm::vec4 rough_bounding_sphere() const;
    //glm::vec4 bounding_sphere() const;


    std::array<glm::vec3, 8> corners() const
    {
        return corners_;
    }

    std::array<glm::vec4, 6> planes() const
    {
        return planes_;
    }

private:
    // normalize
    glm::vec3 project(const glm::vec4& v) const
    {
        return glm::vec3(v.x / v.w, v.y / v.w, v.z / v.w);
    };

private:
    struct frustum_plane
    {
        glm::vec3   xyz;    ///< Camera frustum plane position
        float       negW;   ///< Camera frustum plane, sign of the coordinates
        glm::vec3   sign;   ///< Camera frustum plane position
    } planes__[6];

    std::array<glm::vec3, 8> corners_{};
    std::array<glm::vec4, 6> planes_{};
    glm::mat4 inv_view_projection_;
};

} // namespace fay
