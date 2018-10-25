#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_RENDER_DEFINE_H
#define FAY_RENDER_DEFINE_H

#include "fay/core/fay.h"

namespace fay::render
{

// buffer, texture, shader, uniform, PSO
// device, context, effect, pass

struct buffer_desc
{

};

struct texture_desc
{

};

struct shader_desc
{

};

struct pipeline_desc
{

};

struct pass_desc
{

};

struct context_desc
{

};

struct buffer_id   { uint32_t id; };
struct texture_id  { uint32_t id; };
struct shader_id   { uint32_t id; };
struct pipeline_id { uint32_t id; };
struct pass_id     { uint32_t id; };


} // namespace fay

#endif // FAY_RENDER_DEFINE_H