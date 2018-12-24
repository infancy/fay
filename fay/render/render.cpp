#include "glad/glad.h"

#include "fay/core/config.h"
#include "fay/core/fay.h"
#include "fay/render/device.h"
#include "fay/render/render.h"

namespace fay // ::render
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

static render_device_ptr device_{ nullptr };

render_device_ptr create_device_opengl(const config& cfg);

void init(const config& cfg)
{
	
}

buffer_id   create(const   buffer_desc& desc) { return device_->create(desc); }
texture_id  create(const  texture_desc& desc) { return device_->create(desc); }
shader_id   create(const   shader_desc& desc) { return device_->create(desc); }
pipeline_id create(const pipeline_desc& desc) { return device_->create(desc); }
frame_id    create(const    frame_desc& desc) { return device_->create(desc); }

void update(buffer_id  id, const void* data, int size);
void update(texture_id id, const void* data);

void destroy(  buffer_id id)
{

}
void destroy( texture_id id);
void destroy(  shader_id id);
void destroy(pipeline_id id);
void destroy(   frame_id id);


} // namespace fay