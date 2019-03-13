#pragma once

#include "fay/core/fay.h"
#include "fay/core/range.h"
#include "fay/render/define.h"
#include "fay/render/native_type.h"
#include "fay/render/pool.h"
#include "fay/render/shader.h"

namespace fay
{

// TODO: OCCLUSION

// TODO: FAY_NO_VTABLE
class render_backend
{
public:
    render_backend() = default; // remove
    render_backend(const render_desc& desc) : renderd_(desc) 
    {
        // init default frame
        // init environment

        if (renderd_.anti_aliasing == anti_aliasing::MSAA)
            renderd_.enable_msaa = true;
    }
    virtual ~render_backend() = default;

    // resource creation, updating and destruction
    virtual   buffer_id create(const   buffer_desc& desc) = 0;
    virtual  texture_id create(const  texture_desc& desc) = 0;
    virtual   shader_id create(const   shader_desc& desc) = 0;
    virtual pipeline_id create(const pipeline_desc& desc) = 0;
    virtual    frame_id create(const    frame_desc& desc) = 0;

    virtual void destroy(  buffer_id id) = 0;
    virtual void destroy( texture_id id) = 0;
    virtual void destroy(  shader_id id) = 0;
    virtual void destroy(pipeline_id id) = 0;
    virtual void destroy(   frame_id id) = 0;

    // render

    // WARNNING: use 0 as default frame(rather than invalid value) by limitations of command_list
    virtual void begin_frame(frame_id id) = 0;
    virtual void end_frame() = 0;

    virtual void clear_color(glm::vec4 rgba, std::vector<uint> targets) const = 0;
    virtual void clear_depth(float depth) const = 0;
    virtual void clear_stencil(uint stencil) const = 0;

    virtual void set_viewport(uint x, uint y, uint width, uint height) = 0;
    virtual void set_scissor(uint x, uint y, uint width, uint height) = 0;

    virtual void apply_pipeline(const pipeline_id id, std::array<bool, 4>) = 0;
    virtual void apply_shader(const shader_id id) = 0;

    // TODO: remove
    virtual void bind_uniform(const char* name, command::uniform uniform) = 0;

    virtual void bind_uniform(uint ub_index, const void* data, uint size, shader_stage stage = shader_stage::none) = 0;
    virtual void bind_texture(const texture_id id, int tex_unit, const std::string& sampler, shader_stage stage = shader_stage::none) = 0;

    virtual void bind_index(const buffer_id id) = 0;
    virtual void bind_vertex(const buffer_id id, std::vector<size_t> attrs, std::vector<size_t> slots, size_t instance_rate) = 0;

    virtual void update(buffer_id id, const void* data, int size) = 0;
    virtual void update(texture_id id, const void* data) = 0;

    virtual void draw(uint count, uint first, uint instance_count) = 0;
    virtual void draw_index(uint count, uint first, uint instance_count) = 0;

protected:
    render_desc renderd_{};
    // TODO: desc_resource_pool desc_pool_{};
};

using render_backend_ptr = std::unique_ptr<render_backend>;

render_backend_ptr create_backend_opengl(const render_desc& desc);
render_backend_ptr create_backend_d3d11(const render_desc& desc);

render_backend_ptr create_render_backend(const render_desc& desc)
{
    switch (desc.backend)
    {
        case render_backend_type::opengl:
            return create_backend_opengl(desc);
        case render_backend_type::d3d11:
            return create_backend_d3d11(desc);
        case render_backend_type::none:
        default:
            LOG(ERROR) << "render_device: no render_backend";
            return nullptr;
    }
}

} // namespace fay