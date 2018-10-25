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
	render_backend render_backend_v{ render_backend::opengl33 };
};

inline config g_config_{};
inline const config& g_config{ g_config_ };

} // namespace fay

#endif // FAY_CORE_CONFIG_H
