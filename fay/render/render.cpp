#include "glad/glad.h"

#include "fay/core/config.h"
#include "fay/core/fay.h"
#include "fay/render/device.h"
#include "fay/render/render.h"

namespace fay::render
{

// ---------------------------------------
// helper functions

// return true if pixel format is a compressed format
bool is_compressed_pixel_format(pixel_format fmt) 
{
	return true;
}

// ---------------------------------------
// forward

static render_device_ptr device{ nullptr };

render_device_ptr create_device_opengl33(const config& cfg);

void init(const config& cfg)
{
	switch (cfg.render_backend_v)
	{
		case render_backend::none:
			break;
		case render_backend::opengl:
			break;
		case render_backend::opengl_dsa:
			break;
		case render_backend::d3d11:
			break;
		default:
			break;
	}
}

buffer_id   create(const   buffer_desc& desc)
{
    DCHECK(!(desc.usage == resource_usage::immutable && desc.data == nullptr));
    DCHECK(!(desc.type == buffer_type::vertex && desc.instance_rate != 0));
    DCHECK(!(desc.type == buffer_type::instance && desc.instance_rate <= 0));

    return device->create(desc);
}
texture_id  create(const  texture_desc& desc);
shader_id   create(const   shader_desc& desc)
{
    DCHECK(desc.vs != nullptr && desc.fs != nullptr);
    return device->create(desc);
}
pipeline_id create(const pipeline_desc& desc);
frame_id    create(const    frame_desc& desc);

void update(buffer_id  id, const void* data, int size);
void update(texture_id id, const void* data);

void destroy(  buffer_id id)
{
    DCHECK(id.id != 0);

}
void destroy( texture_id id);
void destroy(  shader_id id);
void destroy(pipeline_id id);
void destroy(   frame_id id);


} // namespace fay