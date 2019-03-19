#pragma once

#include "fay/core/fay.h"
#include "fay/core/string.h"
#include "fay/render/define.h"
#include "fay/resource/text.h"

namespace fay
{

shader_desc scan_shader_program(const std::string shader_name, std::string vs_filepath, std::string fs_filepath, render_backend_type backend_type);

} // namespace fay