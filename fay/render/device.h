#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_RENDER_DEVICE_H
#define FAY_RENDER_DEVICE_H

#include "fay/core/config.h"
#include "fay/core/fay.h"
#include "fay/render/define.h"

namespace fay::render
{

class render_device
{
public:
    virtual ~render_device() = default;

    // TODO: move to device.cpp
    render_backend type() const { return render_backend_; }
    std::string name() const { name_; }

	// resource creation, updating and destruction
	virtual   buffer_id create(const   buffer_desc& desc) = 0;
	virtual  texture_id create(const  texture_desc& desc) = 0;
	virtual   shader_id create(const   shader_desc& desc) = 0;
	virtual pipeline_id create(const pipeline_desc& desc) = 0;
	virtual    frame_id create(const    frame_desc& desc) = 0;

	virtual void update( buffer_id id, const void* data, int size) = 0;
	virtual void update(texture_id id, const void* data) = 0;

	virtual void destroy(  buffer_id id) = 0;
	virtual void destroy( texture_id id) = 0;
	virtual void destroy(  shader_id id) = 0;
	virtual void destroy(pipeline_id id) = 0;
	virtual void destroy(   frame_id id) = 0;

	// get resource state (initial, alloc, valid, failed)
	virtual resource_state query_state(  buffer_id id) = 0;
	virtual resource_state query_state( texture_id id) = 0;
	virtual resource_state query_state(  shader_id id) = 0;
	virtual resource_state query_state(pipeline_id id) = 0;
	virtual resource_state query_state(   frame_id id) = 0;

	// render
	virtual void submit(pass_desc desc) = 0;
	virtual void draw() = 0;
	virtual void draw(pass_desc desc) = 0;

protected:
    render_backend render_backend_{};
    std::string name_{};
};

using render_device_ptr = std::unique_ptr<render_device>;

render_device_ptr create_device_opengl(const config& cfg);

} // namespace fay

#endif // FAY_RENDER_DEVICE_H