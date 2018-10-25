#include "fay/core/config.h"
#include "fay/core/fay.h"
#include "fay/render/device.h"
#include "fay/render/device_opengl33.h"
#include "fay/render/render.h"

namespace fay::render
{

static render_device_ptr device{ nullptr };

extern render_device_ptr create_device_opengl33(const config& cfg);

void init_api(const config& cfg)
{
	switch (cfg.render_backend_v)
	{
	case render_backend::none:
		break;
	case render_backend::opengl33:
		break;
	case render_backend::opengl45:
		break;
	case render_backend::d3d11:
		break;
	default:
		break;
	}
}


} // namespace fay