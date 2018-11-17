#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_RENDER_RENDER_H
#define FAY_RENDER_RENDER_H

#include "fay/core/config.h"
#include "fay/render/define.h"

namespace fay::render
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
shader_id   create(const   shader_desc& desc);
pipeline_id create(const pipeline_desc& desc);
frame_id    create(const    frame_desc& desc);

void update(buffer_id  id, const void* data, int size);
void update(texture_id id, const void* data);

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

/* rendering functions */
extern void begin_default_pass(const action* pass_action, int width, int height);
extern void begin_pass(frame_id pass, const action* pass_action);
extern void apply_viewport(int x, int y, int width, int height, bool origin_top_left);
extern void apply_scissor_rect(int x, int y, int width, int height, bool origin_top_left);
extern void apply_draw_state(const pass_desc* ds);
extern void apply_uniform_block(shader_type stage, int ub_index, const void* data, int num_bytes);
extern void draw(int base_element, int num_elements, int num_instances);
extern void end_pass(void);
extern void commit(void);

} // namespace fay

#endif // FAY_RENDER_RENDER_H