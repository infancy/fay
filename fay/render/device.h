#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_RENDER_DEVICE_H
#define FAY_RENDER_DEVICE_H

#include "fay/core/config.h"
#include "fay/core/fay.h"
#include "fay/core/utility.h"
#include "fay/render/backend.h"
#include "fay/render/define.h"
#include "fay/render/pool.h"

namespace fay
{

// command translate, validation layer
class render_device
{
public:
    render_device() {} 
    render_device(const render_desc& desc);
    ~render_device() = default;

    // TODO: move to device.cpp
    render_backend_type type() const { return context_.type; }
    std::string name() const { context_.name; }

    // resource creation, updating and destruction
    template<typename Descriptor>
    auto create(const Descriptor& desc)
    {
        create_check(desc); 
        auto did = desc_.insert(desc);

        auto pid = backend_->create(desc);
        
        DCHECK(pid == did);
        return pid;
        // TODO???: cache the buffer_desc
    }
    /*
    texture_id  create(const  texture_desc& desc)
    {
        create_check(desc);
        return backend_->create(desc);
    }
    shader_id   create(const   shader_desc& desc)
    {
        create_check(desc);
        return backend_->create(desc);
    }
    pipeline_id create(const pipeline_desc& desc)
    {
        create_check(desc);
        return backend_->create(desc);
    }
    frame_id    create(const    frame_desc& desc)
    {
        create_check(desc);
        return backend_->create(desc);
    }
    */

    void update(buffer_id id, const void* data, int size);
    void update(texture_id id, const void* data);

    template<typename Handle>
    void destroy(Handle id)
    {
        DCHECK(desc_.have(id));
        backend_->destroy(id);
        desc_.erase(id);
    }

    /*
    void destroy(texture_id id);
    void destroy(shader_id id);
    void destroy(pipeline_id id);
    void destroy(frame_id id);
    */

/*
    const   buffer_desc* query_desc(  buffer_id id) { return nullptr; }
    const  texture_desc* query_desc( texture_id id) { return nullptr; }
    const   shader_desc* query_desc(  shader_id id) { return nullptr; }
    const pipeline_desc* query_desc(pipeline_id id) { return nullptr; }
    const    frame_desc* query_desc(   frame_id id) { return nullptr; }


    // get resource state (initial, alloc, valid, failed)
    resource_state query_state(buffer_id id);
    resource_state query_state(texture_id id);
    resource_state query_state(shader_id id);
    resource_state query_state(pipeline_id id);
    resource_state query_state(frame_id id);
*/

    template<typename Handle>
    bool query_valid(Handle id)
    {
        return desc_.have(id);
    }

    // render


    void execute(command_list cmds)
    {
        submit(std::move(cmds));
        execute();
    }

    void submit(command_list cmds)
    {
        command_queue_.emplace_back(cmds);
    }
    void execute()
    {
        // clear_color.targets.max < frame.rts.size()

        // clear_viewport, clear_scissor -> -(y+h)+frame.width

        for (const auto& cmds : command_queue_)
            execute_command_list(cmds);

        command_queue_.clear();
    }

private:
    // 
    void create_check(const   buffer_desc& desc)
    {
        DCHECK(!(desc.usage == resource_usage::immutable && desc.data == nullptr));

        DCHECK(!(desc.type == buffer_type::vertex && desc.instance_rate != 0));
        DCHECK(!(desc.type == buffer_type::instance && desc.instance_rate <= 0));
    }
    void create_check(const  texture_desc& desc)
    {

    }
    void create_check(const   shader_desc& desc)
    {

    }
    void create_check(const pipeline_desc& desc)
    {

    }
    void create_check(const    frame_desc& desc)
    {

    }

    // command
    void begin_default_frame()
    {
        ctx_.is_offscreen = false;
        backend_->begin_frame(frame_id(0));
    }
    void begin_frame(frame_id id)
    {
        DCHECK(query_valid(id)) << "invalid id";

        if (ctx_.frm_id == id)
            return;

        ctx_.is_offscreen = true;
        ctx_.frm_id = id;
        ctx_.frm = desc_[id];

        backend_->begin_frame(id);
    }
    void end_frame()
    {
        backend_->end_frame();
    }

    void clear_color(glm::vec4 rgba, std::vector<uint32_t> targets) const
    {
        auto is_clamp = [](float val)
        {
            return (val >= 0.f) && (val <= 1.f);
        };

        // unified approach
        auto view_count = ctx_.is_offscreen ? ctx_.frm.render_targets.size() : 1;
        
        if (targets.empty())
        {
            targets.resize(view_count);
            std::iota(targets.begin(), targets.end(), 0);
        }

        DCHECK(is_clamp(rgba.r) && is_clamp(rgba.g) && is_clamp(rgba.b) && is_clamp(rgba.a));
        DCHECK(targets.size() <= view_count);
        DCHECK(all_of(targets, [view_count](uint32_t idx)
        {
            return idx >= 0 && idx < view_count;
        })) << "have some index out of range";

        backend_->clear_color(rgba, targets);
    }
    void clear_depth(float depth) const
    {
        DCHECK(is_clamp(depth, 0.f, 1.f));

        backend_->clear_depth(depth);
    }
    void clear_stencil(uint32_t stencil) const
    {
        DCHECK(is_clamp(stencil, 0x00u, 0xffu));

        backend_->clear_stencil(stencil);
    }

    void set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        // TOCHECK

        // TODO

        backend_->set_viewport(x, y, width, height);
    }
    void set_scissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        // TOCHECK

        //if(ctx_.type == render_backend_type::opengl)
        //    y = // (_sg_gl.cur_pass_height - (y + h)) : y;

        backend_->set_scissor(x, y, width, height);
    }

    void apply_pipeline(const pipeline_id id);
    void apply_shader(const shader_id id)
    {
        DCHECK(query_valid(id)) << "invalid id";

        ctx_.shd_id = id;
        ctx_.shd = desc_[id];

        backend_->apply_shader(id);
    }

    void bind_uniform(const std::string& name, command::uniform uniform)
    {
        backend_->bind_uniform(name.c_str(), uniform);
    }
    void bind_uniform(const std::string& name, const void* data, uint32_t size)
    {
        auto idx = index(ctx_.shd.uniform_blocks, [name, size](auto&& ub)
        {
            return (ub.name == name) && ub.size == size;
        });
        DCHECK(idx.has_value()) << "invaild uniform_block";

        backend_->bind_uniform(idx.value(), data, size);
    }

    void bind_texture(const texture_id id, uint32_t tex_unit, const std::string& sampler)
    {
        DCHECK(query_valid(id)) << "invalid id";
        DCHECK(
            any_of(ctx_.shd.fs_samplers, std::equal_to{}, sampler) || 
            any_of(ctx_.shd.vs_samplers, std::equal_to{}, sampler));

        backend_->bind_texture(id, tex_unit, sampler);
    }

    void bind_texture(const std::vector<texture_id>& textures)
    {
        const auto& texs = textures;
        const auto& samplers = ctx_.shd.vs_samplers;
        DCHECK(texs.size() <= samplers.size()) << "too many textures";

        // TODO: check texs[i] and samplers[i] is matching

        // TODO: WARNNING
        for (auto i : range(texs.size()))
            bind_texture(texs[i], ctx_.tex_unit++, samplers[i]);
    }

    void bind_index(const buffer_id id)
    {
        DCHECK(query_valid(id)) << "invalid id";

        ctx_.index_count = desc_[id].size; // WARNNING: ???
        backend_->bind_index(id);
    }
    void bind_buffer(const buffer_id id, const std::vector<attribute_usage>& attrs, uint32_t instance_rate);

    //void update(buffer_id id, const void* data, uint32_t size);
    //void update(texture_id id, const void* data, uint32_t size);

    void draw(uint32_t count, uint32_t first, uint32_t instance_count)
    {
        DCHECK((count >= 0) && (first >= 0) && (instance_count > 0));

        if (count == 0)
            count = ctx_.vertex_count;

        backend_->draw(count, first, instance_count);
    }
    void draw_index(uint32_t count, uint32_t first, uint32_t instance_count)
    {
        DCHECK((count >= 0) && (first >= 0) && (instance_count > 0));

        if (count == 0)
            count = ctx_.index_count;

        backend_->draw_index(count, first, instance_count);
    }

    // execute
    void execute_command_list(const command_list& cmds);

    void execute_command(const command& cmd);

private:
    struct context
    {
        std::string name{};
        render_backend_type type{};
        //pipeline_id default_pipe{};
    };

    struct command_list_context
    {
        uint32_t tex_unit{};
        uint32_t vertex_count{};
        uint32_t index_count{};

        shader_id   shd_id{};
        pipeline_id pipe_id{};
        frame_id    frm_id{};

        shader_desc   shd{};
        pipeline_desc pipe{};

        bool is_offscreen{};
        frame_desc    frm{};

        //bool first_vertex_buffer{ true };
    };

    context context_{};

    command_list_context ctx_{};
    std::vector<command_list> command_queue_{};

    // window_ptr window_{}; event
    render_backend_ptr backend_{};
    resource_pool<buffer_desc, texture_desc, shader_desc, pipeline_desc, frame_desc> desc_{}; // TODO: rename render_pool
};

using render_device_ptr = std::unique_ptr<render_device>;

} // namespace fay

#endif // FAY_RENDER_DEVICE_H