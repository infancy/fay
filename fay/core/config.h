#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_CORE_CONFIG_H
#define FAY_CORE_CONFIG_H

#include "fay/core/define.h"

namespace fay //::config
{

struct config
{
	render_backend_type render_backend_type_v{ render_backend_type::opengl };
};

inline config g_config_{};
inline const config& g_config{ g_config_ };

} // namespace fay

#endif // FAY_CORE_CONFIG_H
