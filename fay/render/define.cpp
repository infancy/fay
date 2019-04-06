#include "fay/gfx/renderable.h"
#include "fay/gfx/frame.h"
#include "fay/render/define.h"

namespace fay
{

command_list& command_list::begin_frame(const frame& frm)
{
    begin_frame(frm.view());

    return *this;
}

command_list& command_list::begin_frame(const frame& frm, pipeline_id pipe, shader_id shd)
{
    begin_frame(frm);
    clear();
    apply_state(pipe, shd);

    return *this;
}

command_list& command_list::draw(renderable* renderable)
{
    renderable->render(*this);

    return *this;
}

} // namespace fay