#pragma once

#include "fay/core/fay.h"
#include "fay/render/define.h"

namespace fay
{

// TODO: FAY_NO_VTABLE
class render_backend
{
public:
    render_backend() = default; // remove
    render_backend(const render_desc& desc) : render_desc_(desc) 
    {
        // init default frame
        // init environment

        if (desc.anti_aliasing == anti_aliasing::MSAA)
            feature_.use_msaa = true; // rename: enabled_MSAA
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

    virtual void clear_color(glm::vec4 rgba, std::vector<uint32_t> targets) const = 0;
    virtual void clear_depth(float depth) const = 0;
    virtual void clear_stencil(uint32_t stencil) const = 0;

    virtual void set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
    virtual void set_scissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

    virtual void apply_pipeline(const pipeline_id id, std::array<bool, 4>) = 0;
    virtual void apply_shader(const shader_id id) = 0;

    virtual void bind_uniform(const char* name, command::uniform uniform) = 0;
    virtual void bind_uniform(uint32_t ub_index, const void* data, uint32_t size) = 0;

    virtual void bind_texture(const texture_id id, int tex_unit, const std::string& sampler) = 0;

    virtual void bind_index(const buffer_id id) = 0;
    virtual void bind_vertex(const buffer_id id, std::vector<size_t> attrs, std::vector<size_t> slots, size_t instance_rate) = 0;

    virtual void update(buffer_id id, const void* data, int size) = 0;
    virtual void update(texture_id id, const void* data) = 0;

    virtual void draw(uint32_t count, uint32_t first, uint32_t instance_count) = 0;
    virtual void draw_index(uint32_t count, uint32_t first, uint32_t instance_count) = 0;

protected:
    struct feature
    {
        bool use_msaa{};
    };

    feature feature_{};
    render_desc render_desc_{};
};

using render_backend_ptr = std::unique_ptr<render_backend>;

render_backend_ptr create_backend_opengl(const render_desc& desc);

} // namespace fay