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



// setup and misc functions
void init_api(const config& cfg);
//void reset_api(const config& cfg);
void shutdown(void);
bool isvalid(void);
bool query_feature(feature feature);
void reset_state_cache(void);



// resource creation, updating and destruction

buffer_id   create_buffer(const buffer_desc& desc);
texture_id  create_texture(const texture_desc& desc);
shader_id   create_shader(const shader_desc& desc);
pipeline_id create_pipeline(const pipeline_desc& desc);
pass_id     create_pass(const pass_desc& desc);

void update_buffer(buffer buf, const void* data_ptr, int data_size);
void update_texture(texture img, const texture_content* data);

void destroy_buffer(buffer_id buf);
void destroy_texture(texture_id tex);
void destroy_shader(shader_id shd);
void destroy_pipeline(pipeline_id pip);
void destroy_pass(pass_id pass);



/*
// get resource state (initial, alloc, valid, failed)
resource_state query_buffer_state(buffer buf);
resource_state query_texture_state(texture img);
resource_state query_shader_state(shader shd);
resource_state query_pipeline_state(pipeline pip);
resource_state query_pass_state(pass pass);



// rendering functions
void begin_default_pass(const pass_action* pass_action, int width, int height);
void begin_pass(pass pass, const pass_action* pass_action);
void apply_viewport(int x, int y, int width, int height, bool origin_top_left);
void apply_scissor_rect(int x, int y, int width, int height, bool origin_top_left);
void apply_draw_state(const draw_state* ds);
void apply_uniform_block(shader_stage stage, int ub_index, const void* data, int num_bytes);
void draw(int base_element, int num_elements, int num_instances);
void end_pass(void);
void commit(void);



// separate resource allocation and initialization (for async setup)
buffer alloc_buffer(void);
texture alloc_texture(void);
shader alloc_shader(void);
pipeline alloc_pipeline(void);
pass alloc_pass(void);
void init_buffer(buffer buf_id, const buffer_desc* desc);
void init_texture(texture img_id, const texture_desc* desc);
void init_shader(shader shd_id, const shader_desc* desc);
void init_pipeline(pipeline pip_id, const pipeline_desc* desc);
void init_pass(pass pass_id, const pass_desc* desc);
void fail_buffer(buffer buf_id);
void fail_texture(texture img_id);
void fail_shader(shader shd_id);
void fail_pipeline(pipeline pip_id);
void fail_pass(pass pass_id);

// rendering contexts (optional)
context setup_context(void);
void activate_context(context ctx_id);
void discard_context(context ctx_id);
*/

} // namespace fay

#endif // FAY_RENDER_RENDER_H