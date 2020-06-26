#pragma once

#include "fay/core/fay.h"
#include "fay/core/string.h"
#include "fay/render/define.h"
#include "fay/resource/text.h"

namespace fay
{

// TODO: remove
shader_desc scan_shader_program(const std::string& shader_name, std::string vs_filepath, std::string fs_filepath, render_backend_type backend_type);



shader_desc create_shader_desc(const std::string& shader_name, render_backend_type backend_type, std::string base_filepath);

// Parameter check
// map to RootSignature???
// check params with relevant shader in runtime, or generate shader_params by tools
struct shader_params
{

};

struct camera_shader_params : shader_params
{

};

struct light_shader_params : shader_params
{

};

} // namespace fay