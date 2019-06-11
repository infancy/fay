#pragma once

#include "fay/core/fay.h"
#include "fay/gfx/renderable.h"
#include "fay/gfx/material.h"

namespace fay
{

// only verties and indices, without material
class raw_mesh : public renderable
{
public:
    raw_mesh(fay::render_device* device, const resource_mesh& res)
    {
        name_ = res.name;

        buffer_desc bd;
        {
            bd.name = res.name + "_vbo";
            bd.size = res.size; // rename: num
            bd.stride = res.layout.stride(); // TODO: do it by helper functions;
            bd.data = res.vertices.data();
            bd.type = buffer_type::vertex;

            bd.layout = res.layout;
        }
        vbo = device->create(bd);

        fay::buffer_desc id(res.name + "_ibo", res.indices.size(), res.indices.data(), fay::buffer_type::index);
        ibo = device->create(id);

        primitive_ = res.primitive_;
    }

    void render(command_list& cmd) override
    {
        cmd
            //.bind_uniform("bAlbedo", bAlbedo)
            .bind_index(ibo)
            .bind_vertex(vbo)
            .draw_index();
    }

protected:
    std::string name_{};

    buffer_id vbo;
    buffer_id ibo;
    primitive_type primitive_;
};

class static_mesh : public renderable
{
public:
    static_mesh(fay::render_device* device, const resource_mesh& res, material_sp mat)
    {
        name_ = res.name;

        buffer_desc bd; 
        {
            bd.name = res.name + "_vbo";
            bd.size = res.size; // rename: num
            bd.stride = res.layout.stride(); // TODO: do it by helper functions;
            bd.data = res.vertices.data();
            bd.type = buffer_type::vertex;

            bd.layout = res.layout;
        }
        vbo = device->create(bd);

        fay::buffer_desc id(res.name + "_ibo", res.indices.size(), res.indices.data(), fay::buffer_type::index);
        ibo = device->create(id);

        primitive_ = res.primitive_;
        mat_ = mat;
    }

    void render(command_list& cmd) override
    {
        // TODO: shader, pipeline

        bool bAlbedo = mat_->textures[0].value > 0;

        cmd
            // TODO: span_view
            .try_bind_texture(mat_->textures[0], "Albedo")
            .bind_uniform("albedo", mat_->albedo)
            .bind_uniform("bAlbedo", bAlbedo)
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

    material_sp mat_; // todo: csptr
};

class array_mesh : public renderable
{
public:
    array_mesh() = default;

    array_mesh(const std::vector<renderable_sp>& list, std::vector<size_t> indices = {})
    {
        if (indices.empty())
        {
            //list_ = list;
            for (auto& renderable : list)
                list_.push_back(renderable);
        }
        else
        {
            for (auto index : indices)
                list_.push_back(list[index]);
        }
    }

    void render(command_list& cmd) override
    {
        for (auto& renderable : list_)
            renderable->render(cmd);
    }

private:
    std::list<renderable_sp> list_;
};

} // namespace fay
