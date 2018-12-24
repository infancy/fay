#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_RENDER_RENDER_H
#define FAY_RENDER_RENDER_H

#include "fay/core/config.h"
#include "fay/render/define.h"

namespace fay // ::render
{

// common interface
// render_device, device_context, render_queue

// Provides both fine - grained and coarse - grained APIs ???

// setup and misc functions
void init(const config& cfg);
void reset(const config& cfg);
bool isvalid(void);
bool query_feature(render_feature feature);
void reset_state_cache(void);

// resource creation, updating and destruction
buffer_id   create(const   buffer_desc& desc);
texture_id  create(const  texture_desc& desc);
//uniform_id create(const uniform_desc& desc);
shader_id   create(const   shader_desc& desc);
pipeline_id create(const pipeline_desc& desc);
frame_id    create(const    frame_desc& desc);
//pass_id   create(const     pass_desc& desc);

void update(buffer_id  id, const void* data, int size);
void update(texture_id id, const void* data);
void update(shader_id  id, const void* data); // update uniform block
// void update(uniform_id id, ...)

// void transfer/read/feedback(texture_id, const void* data);

// map/unmap

void destroy(  buffer_id id);
void destroy( texture_id id);
void destroy(  shader_id id);
void destroy(pipeline_id id);
void destroy(   frame_id id);

// get resource state (initial, alloc, valid, failed)
resource_state query_state(  buffer_id id);
resource_state query_state( texture_id id);
resource_state query_state(  shader_id id);
resource_state query_state(pipeline_id id);
resource_state query_state(   frame_id id);

// void submit_graphics(pass_desc desc);
// void submit_compute(pass_desc desc);
// void submit_blit(pass_desc desc);

void execute(command_list cmd); // execute(cmd); execute(std::move(cmd));

void submit(command_list cmd);
void execute();

} // namespace fay

#endif // FAY_RENDER_RENDER_H