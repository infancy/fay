#pragma once

#include "fay/core/fay.h"
#include "fay/gfx/renderable.h"
#include "fay/gfx/material.h"

namespace fay
{



class static_mesh : public renderable
{
public:
    static_mesh(fay::render_device_ptr& device, const resource_mesh& res, material_sptr mat)
    {
        name_ = res.name;

        buffer_desc bd; 
        {
            bd.name = res.name + "_vbo";
            bd.size = res.size; // rename: num
            bd.stride = 32; // TODO: do it by helper functions;
            bd.data = res.vertices.data();
            bd.type = buffer_type::vertex;

            bd.layout = res.layout;
        }
        fay::buffer_desc id(fay::buffer_type::index); 
        {
            id.name = res.name + "_ibo";
            id.size = res.indices.size();
            id.data = res.indices.data();
        }
        vbo = device->create(bd);
        ibo = device->create(id);

        primitive_ = res.primitive_;
        mat_ = mat;
    }

    void render(command_list& cmd) override
    {
        // TODO: shader, pipeline
        cmd
            .bind_texture_unit(mat_->textures[0], 0, "Diffuse")
            .bind_index(ibo)
            .bind_vertex(vbo)
            .draw_index();
    }

protected:
    std::string name_{};

    buffer_id vbo;
    // buffer_id vbo_attributes;
    buffer_id ibo;

    uint32_t ibo_offset = 0;
    int32_t vertex_offset = 0;
    uint32_t count = 0;
    uint32_t position_stride = 0;
    uint32_t attribute_stride = 0;

    primitive_type primitive_;

    material_sptr mat_; // todo: csptr

};

} // namespace fay
