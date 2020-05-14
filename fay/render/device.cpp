#include "fay/core/range.h"
#include "fay/core/utility.h"
#include "fay/render/device.h"

namespace fay
{

render_device::render_device(const render_desc& desc) :
    backend_{ create_render_backend(desc) },
    pool_{ backend_->get_render_desc_pool() }
{
    context_.name = desc.name;
    context_.type = desc.backend;

    // TODO: create other with "id=0"
    ctx_.pipe_id = create(pipeline_desc());
    ctx_.pipe = pool_.get(ctx_.pipe_id);
    // make device have a old_pipe
    backend_->apply_pipeline(ctx_.pipe_id, {true, true, true, true});

    //execute(command_list()
    //    .begin_default_frame() // TODO: other backend can bind default without init???
    //    .apply_pipeline(context_.default_pipe)
    //    .end_frame());
}

void render_device::apply_pipeline(const pipeline_id id)
{
    DCHECK(query_valid(id)) << "invalid id";

    // !!!!!!!!!!!!!!!!!!!!
    // TODO: how to deal with "command_list_ctx clear every new frame and pipe_id is assigned 0"
    if (ctx_.pipe_id == id)
        return;

    const auto old = ctx_.pipe; // const auto& old = ctx_.pipe;
    ctx_.pipe_id = id;
    ctx_.pipe = pool_.get(ctx_.pipe_id);
    const auto& now = ctx_.pipe;
    
    std::array<bool, 4> flags{};

    if (old.primitive_type != now.primitive_type)
    {
        flags[0] = true;
    }

    if ((old.alpha_to_coverage_enabled != now.alpha_to_coverage_enabled) ||
        (old.cull_mode                 != now.cull_mode) ||
        (old.face_winding              != now.face_winding) ||
        (old.rasteriza_sample_count    != now.rasteriza_sample_count) ||
        (old.depth_bias                != now.depth_bias) ||
        (old.depth_bias_slope_scale    != now.depth_bias_slope_scale) ||
        (old.depth_bias_clamp          != now.depth_bias_clamp))
    {
        flags[1] = true;
    }

    if ((old.depth_enabled       != now.depth_enabled) ||
        (old.depth_compare_op    != now.depth_compare_op) ||
        (old.stencil_enabled     != now.stencil_enabled) ||
        (old.stencil_front       != now.stencil_front) ||
        (old.stencil_back        != now.stencil_back) ||
        (old.stencil_test_mask   != now.stencil_test_mask) ||
        (old.stencil_write_mask  != now.stencil_write_mask) ||
        (old.stencil_ref         != now.stencil_ref))
    {
        flags[2] = true;
    }

    if ((old.blend_enabled          != now.blend_enabled) ||
        (old.blend_rgb              != now.blend_rgb) ||
        (old.blend_alpha            != now.blend_alpha) ||
        (old.blend_write_mask       != now.blend_write_mask) ||
        (old.color_attachment_count != now.color_attachment_count) ||
        (old.blend_color_format     != now.blend_color_format) ||
        (old.blend_depth_format     != now.blend_depth_format) ||
        (old.blend_color            != now.blend_color)) // TODO???: float compare
    {
        flags[3] = true;
    }

    //if (any_of(flags, std::equal_to{}, true))
    backend_->apply_pipeline(id, flags);
}

void render_device::bind_buffer(const buffer_id id, const std::vector<attribute_usage>& usages, size_t instance_rate)
{
    DCHECK(query_valid(id)) << "invalid id";

    auto desc = pool_.get(id);
    DCHECK(
        (desc.type == buffer_type::vertex && instance_rate == 0) ||
        (desc.type == buffer_type::instance /*&& instance_rate * desc.size <= ctx_.vertex_count*/)); // TODO???: draw_instance before bind_instance

    if(desc.type == buffer_type::vertex)
        ctx_.vertex_count = desc.size;

    std::vector<size_t> attrs_, slots_;

    const auto& buf_layout = pool_.get(id).layout;
    const auto& shd_layout = ctx_.shd.layout;

    if (usages.size() == 0)
    {
        DCHECK(buf_layout.size() <= shd_layout.size());
        int num = buf_layout.size();

        attrs_.resize(num);
        slots_.resize(num);

        for (auto i : range(num))
        {
            DCHECK(buf_layout[i] == shd_layout[i]) << "not same vertex attribute";
            attrs_[i] = i;
            slots_[i] = i;
        }
    }
    else
    {
        DCHECK(usages.size() < ctx_.shd.layout.size());

        for (auto usage : usages)
        {
            auto idx0 = index(buf_layout, [usage](const vertex_attribute& attr)
            {
                return attr.usage() == usage;
            });
            DCHECK(idx0.has_value()) << "invaild vertex attribute";

            auto idx1 = index(shd_layout, [usage](const vertex_attribute& attr)
            {
                return attr.usage() == usage;
            });
            DCHECK(idx1.has_value()) << "invaild vertex attribute";

            DCHECK(buf_layout[idx0.value()] == shd_layout[idx1.value()]) << "not same vertex attribute";

            attrs_.push_back(idx0.value());
            slots_.push_back(idx1.value());

            // WARNING: not complete
            if (usage == attribute_usage::instance_model)
            {
                for (size_t i : range(1, 4))
                {
                    attrs_.push_back(idx0.value() + i);
                    slots_.push_back(idx1.value() + i);
                }
            }
        }
    }

    backend_->bind_vertex(id, attrs_, slots_, instance_rate);
    return;
}



void render_device::execute_command_list(const command_list& cmds)
{
    const auto& cs = cmds.commands_();

    DCHECK(cs.size() > 2);

    DCHECK(
        ((cs.front().type_ == command_type::begin_default_frame) || (cs.front().type_ == command_type::begin_frame)) &&
        (cs.back().type_ == command_type::end_frame))
        << "first cmd must be begin_xx_frame and last cmd must be end_frame";

    DCHECK(std::none_of(++cs.cbegin(), --cs.cend(), [](const command& cmd) 
    {
        return 
            (cmd.type_ == command_type::begin_default_frame) ||
            (cmd.type_ == command_type::begin_frame) ||
            (cmd.type_ == command_type::end_frame);
    })) << "cmds in the middle can't be begin_xx_frame or end_frame";

    // TOCHECK

    // TODO: better way
    ctx_ = {};
    ctx_.pipe_id = pipeline_id(1);
    ctx_.pipe = pool_.get(ctx_.pipe_id);

    for (const auto& cmd : cs)
        execute_command(cmd);
}

void render_device::execute_command(const command& cmd)
{
    switch (cmd.type_)
    {
        case command_type::begin_default_frame:

            begin_default_frame();
            break;

        case command_type::begin_frame:

            begin_frame(cmd.frm_);
            break;

        case command_type::end_frame:

            end_frame();
            break;

        case command_type::clear_color:

            clear_color(cmd.float4_, cmd.uints_);
            break;

        case command_type::clear_depth:

            clear_depth(cmd.float_);
            break;

        case command_type::clear_stencil:

            clear_stencil(cmd.uint_);
            break;

        case command_type::set_viewport:

            set_viewport(cmd.uint4_.x, cmd.uint4_.y, cmd.uint4_.z, cmd.uint4_.w);
            break;

        case command_type::set_scissor:

            set_scissor(cmd.uint4_.x, cmd.uint4_.y, cmd.uint4_.z, cmd.uint4_.w);
            break;

        case command_type::apply_shader:

            apply_shader(cmd.shd_);
            break;

        case command_type::apply_pipeline:

            apply_pipeline(cmd.pip_);
            break;

        case command_type::bind_index:

            bind_index(cmd.buf_);
            break;

        case command_type::bind_buffer:

            bind_buffer(cmd.buf_, cmd.attrs_, cmd.uint_);
            break;

        case command_type::bind_named_texture:

            bind_texture(cmd.tex_, cmd.str_);
            break;

        case command_type::bind_textures:

            bind_textures(cmd.texs_, static_cast<shader_stage>(cmd.uint_));
            break;

        case command_type::bind_uniform:

            bind_uniform(cmd.str_, cmd.uniform_);
            break;

        case command_type::bind_uniform_block:

            bind_uniform(cmd.str_, cmd.uniform_block_.data(), cmd.uniform_block_.size());
            break;

        case command_type::draw:

            draw(cmd.uint4_.x, cmd.uint4_.y, cmd.uint4_.z);
            break;

        case command_type::draw_index:

            draw_index(cmd.uint4_.x, cmd.uint4_.y, cmd.uint4_.z);
            break;

        default:

            LOG(ERROR) << "shouldn't be here";
            break;
    }
}

} // namespace fay