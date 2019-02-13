#include "fay/gfx/renderable.h"
#include "fay/render/define.h"


namespace fay
{

command_list& command_list::draw(renderable* renderable)
{
    renderable->render(*this);

    return *this;
}

} // namespace fay