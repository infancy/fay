#pragma once

#include <d3d11.h>
#include <d3d12.h>
#include <dxgi.h>
#include "glad/glad.h"

#include "fay/core/define.h"
#include "fay/core/enum.h"
#include "fay/render/define.h"

// https://blog.magnum.graphics/backstage/gfx-api-enum-mapping/

namespace fay
{

// template<typename T>
struct native_type
{
    GLenum opengl{};
	UINT d3d11{};
    UINT d3d12{};
};

struct native_type2
{
    UINT d3d12{};
    UINT vulkan{};
};

const inline enum_class_map<resource_usage, native_type>
resource_usage_map
{
    { resource_usage::immutable, { GL_STATIC_DRAW,  D3D11_USAGE_IMMUTABLE } },
    { resource_usage::dynamic,   { GL_DYNAMIC_DRAW, D3D11_USAGE_DYNAMIC } },
    { resource_usage::stream,    { GL_STREAM_DRAW,  D3D11_USAGE_DYNAMIC } },
};

const inline enum_class_map<buffer_type, native_type>
buffer_type_map
{
	{ buffer_type::vertex,    { GL_ARRAY_BUFFER,         D3D11_BIND_VERTEX_BUFFER, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER } },
	{ buffer_type::index,     { GL_ELEMENT_ARRAY_BUFFER, D3D11_BIND_INDEX_BUFFER,  D3D12_RESOURCE_STATE_INDEX_BUFFER } },
    { buffer_type::instance,  { GL_ARRAY_BUFFER,         D3D11_BIND_VERTEX_BUFFER, 0 } },

    { buffer_type::uniform_cbv,       { 0, 0, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER } },

    { buffer_type::storage_srv,       { 0, 0, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE } },
    { buffer_type::storage_uav,       { 0, 0, D3D12_RESOURCE_STATE_UNORDERED_ACCESS } },

    { buffer_type::uniform_texel_srv, { 0, 0, 0 } },
    { buffer_type::storage_texel_uav, { 0, 0, 0 } },

    { buffer_type::counter_uav,       { 0, 0, 0 } },

    { buffer_type::transfer_src_,     { 0, 0, D3D12_RESOURCE_STATE_COPY_SOURCE } },
    { buffer_type::transfer_dst_,     { 0, 0, D3D12_RESOURCE_STATE_COPY_DEST } },
};

const inline enum_class_map<pixel_format, native_type>
pixel_format_map
{
    { pixel_format::rgba32f,  { 0, DXGI_FORMAT_R32G32B32A32_FLOAT } },
    { pixel_format::rgb32f,   { 0, DXGI_FORMAT_R32G32B32_FLOAT } },
    { pixel_format::rg32f,    { 0, DXGI_FORMAT_R32G32_FLOAT } },
    { pixel_format::r32f,     { 0, DXGI_FORMAT_R32_FLOAT } },

    { pixel_format::rgba16f,  { 0, DXGI_FORMAT_R16G16B16A16_FLOAT } },
  //{ pixel_format::rgb16f,   { 0, DXGI_FORMAT_R16G16B16_FLOAT } },
    { pixel_format::rg16f,    { 0, DXGI_FORMAT_R16G16_FLOAT } },
    { pixel_format::r16f,     { 0, DXGI_FORMAT_R16_FLOAT } },

    { pixel_format::rgba8,    { 0, DXGI_FORMAT_R8G8B8A8_UNORM } },
  //{ pixel_format::rgb8,     { 0, DXGI_FORMAT_R8G8A8_UNORM } },
    { pixel_format::rg8,      { 0, DXGI_FORMAT_R8G8_UNORM } },
    { pixel_format::r8,       { 0, DXGI_FORMAT_R8_UNORM } },

    { pixel_format::rgba4,    { 0, DXGI_FORMAT_B4G4R4A4_UNORM } },

    { pixel_format::rgb10_a2, { 0, DXGI_FORMAT_R10G10B10A2_UNORM } },
    { pixel_format::rgb5_a1,  { 0, DXGI_FORMAT_B5G5R5A1_UNORM } },
    { pixel_format::rgb565,   { 0, DXGI_FORMAT_B5G6R5_UNORM } },

    { pixel_format::depth,        { 0, DXGI_FORMAT_D32_FLOAT} },
    { pixel_format::depthstencil, { 0, DXGI_FORMAT_D24_UNORM_S8_UINT} }
};

const inline enum_class_map<texture_type, native_type>
texture_type_map
{
    { texture_type::one,   { GL_TEXTURE_1D, 0 } },
	{ texture_type::two,   { GL_TEXTURE_2D, 0 } },
	{ texture_type::cube,  { GL_TEXTURE_CUBE_MAP, 0 } },
	{ texture_type::three, { GL_TEXTURE_3D, 0 } },
	{ texture_type::array, { GL_TEXTURE_2D_ARRAY, 0 } },
	//{ texture_type::texture_array_2d,   { 0, 0 } },
	//{ texture_type::texture_array_data, { 0, 0 } },
};

const inline enum_class_map<texture_usage_, native_type>
texture_usage_map
{
    { texture_usage_::undefined,                { 0, 0, 0 } },

    { texture_usage_::sampled_texture,            { 0, 0, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE } },
    { texture_usage_::storage_texture,            { 0, 0, D3D12_RESOURCE_STATE_UNORDERED_ACCESS } },

    { texture_usage_::color_attachment,         { 0, 0, D3D12_RESOURCE_STATE_RENDER_TARGET } },
    { texture_usage_::depth_stencil_attachment, { 0, 0, D3D12_RESOURCE_STATE_DEPTH_WRITE } },

    { texture_usage_::resolve_src_,             { 0, 0, D3D12_RESOURCE_STATE_RESOLVE_SOURCE } },
    { texture_usage_::resolve_dst_,             { 0, 0, D3D12_RESOURCE_STATE_RESOLVE_DEST } },

    { texture_usage_::transfer_src_,            { 0, 0, D3D12_RESOURCE_STATE_COPY_SOURCE } },
    { texture_usage_::transfer_dst_,            { 0, 0, D3D12_RESOURCE_STATE_COPY_DEST } },
};

const inline enum_class_map<render_target, native_type>
render_target_map
{
    { render_target::none,          { GL_NONE, 0 } },
    { render_target::color,         { GL_COLOR_ATTACHMENT0, 0 } },
    { render_target::depth,         { GL_DEPTH_ATTACHMENT, 0 } },
    { render_target::stencil,       { GL_STENCIL_ATTACHMENT, 0 } },
    { render_target::depth_stencil, { GL_DEPTH_STENCIL_ATTACHMENT, 0 } },
};

const inline enum_class_map<filter_mode, native_type>
filter_mode_map
{
    { filter_mode::anisotropy,             { GL_NONE,                   D3D11_FILTER_ANISOTROPIC,                     D3D12_FILTER_ANISOTROPIC } },

    { filter_mode::nearest,                { GL_NEAREST,                D3D11_FILTER_MIN_MAG_MIP_POINT,               D3D12_FILTER_MIN_MAG_MIP_POINT } },
    { filter_mode::nearest_mipmap_nearest, { GL_NEAREST_MIPMAP_NEAREST, D3D11_FILTER_MIN_MAG_MIP_POINT,               D3D12_FILTER_MIN_MAG_MIP_POINT } },
    { filter_mode::nearest_mipmap_linear,  { GL_NEAREST_MIPMAP_LINEAR,  D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR,        D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR } },

    { filter_mode::linear,                 { GL_LINEAR,                 D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT,        D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT } },
    { filter_mode::linear_mipmap_nearest,  { GL_LINEAR_MIPMAP_NEAREST,  D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT,        D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT } },
    { filter_mode::linear_mipmap_linear,   { GL_LINEAR_MIPMAP_LINEAR,   D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR, D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR } },
};

const inline enum_class_map<wrap_mode, native_type>
wrap_mode_map
{
    { wrap_mode::repeat,          { GL_REPEAT,          D3D11_TEXTURE_ADDRESS_WRAP, } },
    { wrap_mode::clamp_to_edge,   { GL_CLAMP_TO_EDGE,   D3D11_TEXTURE_ADDRESS_CLAMP, } },
    { wrap_mode::mirrored_repeat, { GL_MIRRORED_REPEAT, D3D11_TEXTURE_ADDRESS_MIRROR, } },
};

const inline enum_class_map<cube_face, native_type>
cube_face_map
{
    { cube_face::pos_x, { GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0 } },
    { cube_face::neg_x, { GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0 } },
    { cube_face::pos_y, { GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0 } },
    { cube_face::neg_y, { GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0 } },
    { cube_face::pos_z, { GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0 } },
    { cube_face::neg_z, { GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0 } },
};

const inline enum_class_map<shader_stage, native_type>
shader_type_map
{
	{ shader_stage::vertex,   { GL_VERTEX_SHADER, 0 } },
	{ shader_stage::geometry, { GL_GEOMETRY_SHADER, 0 } },
	{ shader_stage::fragment, { GL_FRAGMENT_SHADER, 0 } },

    { shader_stage::mesh,     { 0, 0 } },
};

const inline enum_class_map<primitive_type, native_type>
primitive_type_map
{
	{ primitive_type::points,         { GL_POINTS,         D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,     D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT } },
	{ primitive_type::lines,          { GL_LINES,          D3D11_PRIMITIVE_TOPOLOGY_LINELIST,      D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE } },
	{ primitive_type::line_strip,     { GL_LINE_STRIP,     D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,     D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE } },
	{ primitive_type::triangles,      { GL_TRIANGLES,      D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,  D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE } },
	{ primitive_type::triangle_strip, { GL_TRIANGLE_STRIP, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE } },
};

const inline enum_class_map<cull_mode, native_type>
cull_mode_map
{
	{ cull_mode::none,  { GL_NONE,  D3D11_CULL_NONE,  D3D12_CULL_MODE_NONE } },
	{ cull_mode::front, { GL_FRONT, D3D11_CULL_FRONT, D3D12_CULL_MODE_FRONT } },
	{ cull_mode::back,  { GL_BACK,  D3D11_CULL_BACK,  D3D12_CULL_MODE_BACK } },
};

const inline enum_class_map<fill_mode, native_type>
fill_mode_map
{
    { fill_mode::none,      { 0, 0, 0 } },
    { fill_mode::wireframe, { 0, 0, D3D12_FILL_MODE_WIREFRAME } },
    { fill_mode::solid,     { 0, 0, D3D12_FILL_MODE_SOLID } },
};

const inline enum_class_map<blend_factor, native_type>
blend_factor_map
{
    { blend_factor::zero,                  { GL_ZERO,                     D3D11_BLEND_ZERO,             D3D12_BLEND_ZERO } },
    { blend_factor::one,                   { GL_ONE,                      D3D11_BLEND_ONE,              D3D12_BLEND_ONE } },

    { blend_factor::src_color,             { GL_SRC_COLOR,                D3D11_BLEND_SRC_COLOR,        D3D12_BLEND_SRC_COLOR } },
    { blend_factor::one_minus_src_color,   { GL_ONE_MINUS_SRC_COLOR,      D3D11_BLEND_INV_SRC_COLOR,    D3D12_BLEND_INV_SRC_COLOR } },
    { blend_factor::src_alpha,             { GL_SRC_ALPHA,                D3D11_BLEND_SRC_ALPHA,        D3D12_BLEND_SRC_ALPHA } },
    { blend_factor::one_minus_src_alpha,   { GL_ONE_MINUS_SRC_ALPHA,      D3D11_BLEND_INV_SRC_ALPHA,    D3D12_BLEND_INV_SRC_ALPHA } },

    { blend_factor::dst_color,             { GL_DST_COLOR,                D3D11_BLEND_DEST_COLOR,       D3D12_BLEND_DEST_COLOR } },
    { blend_factor::one_minus_dst_color,   { GL_ONE_MINUS_DST_COLOR,      D3D11_BLEND_INV_DEST_COLOR,   D3D12_BLEND_INV_DEST_COLOR } },
    { blend_factor::dst_alpha,             { GL_DST_ALPHA,                D3D11_BLEND_DEST_ALPHA,       D3D12_BLEND_DEST_ALPHA } },
    { blend_factor::one_minus_dst_alpha,   { GL_ONE_MINUS_DST_ALPHA,      D3D11_BLEND_INV_DEST_ALPHA,   D3D12_BLEND_INV_DEST_ALPHA } },

    { blend_factor::src_alpha_saturated,   { GL_SRC_ALPHA_SATURATE,       D3D11_BLEND_SRC_ALPHA_SAT,    D3D12_BLEND_SRC_ALPHA_SAT } },

    { blend_factor::blend_color,           { GL_CONSTANT_COLOR,           D3D11_BLEND_BLEND_FACTOR,     D3D12_BLEND_BLEND_FACTOR } },
    { blend_factor::one_minus_blend_color, { GL_ONE_MINUS_CONSTANT_COLOR, D3D11_BLEND_INV_BLEND_FACTOR, D3D12_BLEND_INV_BLEND_FACTOR } },
    { blend_factor::blend_alpha,           { GL_CONSTANT_ALPHA,           D3D11_BLEND_BLEND_FACTOR,     D3D12_BLEND_BLEND_FACTOR  } },
    { blend_factor::one_minus_blend_alpha, { GL_ONE_MINUS_CONSTANT_ALPHA, D3D11_BLEND_INV_BLEND_FACTOR, D3D12_BLEND_INV_BLEND_FACTOR } },
};

const inline enum_class_map<blend_op, native_type>
blend_op_map
{
    { blend_op::add,              { GL_FUNC_ADD,              D3D11_BLEND_OP_ADD,          D3D12_BLEND_OP_ADD } },
    { blend_op::subtract,         { GL_FUNC_SUBTRACT,         D3D11_BLEND_OP_SUBTRACT,     D3D12_BLEND_OP_SUBTRACT } },
    { blend_op::reverse_subtract, { GL_FUNC_REVERSE_SUBTRACT, D3D11_BLEND_OP_REV_SUBTRACT, D3D12_BLEND_OP_REV_SUBTRACT } },
};

const inline enum_class_map<stencil_op, native_type>
stencil_op_map
{
	{ stencil_op::keep,       { GL_KEEP,      D3D11_STENCIL_OP_KEEP,     D3D12_STENCIL_OP_KEEP } },
	{ stencil_op::zero,       { GL_ZERO,      D3D11_STENCIL_OP_ZERO,     D3D12_STENCIL_OP_ZERO } },
	{ stencil_op::replace,    { GL_REPLACE,   D3D11_STENCIL_OP_REPLACE,  D3D12_STENCIL_OP_REPLACE } },
	{ stencil_op::incr_clamp, { GL_INCR,      D3D11_STENCIL_OP_INCR_SAT, D3D12_STENCIL_OP_INCR_SAT } },
	{ stencil_op::decr_clamp, { GL_DECR,      D3D11_STENCIL_OP_DECR_SAT, D3D12_STENCIL_OP_DECR_SAT } },
	{ stencil_op::invert,     { GL_INVERT,    D3D11_STENCIL_OP_INVERT,   D3D12_STENCIL_OP_INVERT } },
	{ stencil_op::incr_wrap,  { GL_INCR_WRAP, D3D11_STENCIL_OP_INCR,     D3D12_STENCIL_OP_INCR } },
	{ stencil_op::decr_wrap,  { GL_DECR_WRAP, D3D11_STENCIL_OP_DECR,     D3D12_STENCIL_OP_DECR } },
};

const inline enum_class_map<compare_op, native_type>
compare_op_map
{
    { compare_op::never,         { GL_NEVER,    D3D11_COMPARISON_NEVER,         D3D12_COMPARISON_FUNC_NEVER } },
    { compare_op::less,          { GL_LESS,     D3D11_COMPARISON_LESS,          D3D12_COMPARISON_FUNC_LESS } },
    { compare_op::less_equal,    { GL_LEQUAL,   D3D11_COMPARISON_LESS_EQUAL,    D3D12_COMPARISON_FUNC_LESS_EQUAL } },
    { compare_op::equal,         { GL_EQUAL,    D3D11_COMPARISON_EQUAL,         D3D12_COMPARISON_FUNC_EQUAL } },
    { compare_op::not_equal,     { GL_NOTEQUAL, D3D11_COMPARISON_NOT_EQUAL,     D3D12_COMPARISON_FUNC_NOT_EQUAL } },
    { compare_op::greater,       { GL_GREATER,  D3D11_COMPARISON_GREATER,       D3D12_COMPARISON_FUNC_GREATER } },
    { compare_op::greater_equal, { GL_GEQUAL,   D3D11_COMPARISON_GREATER_EQUAL, D3D12_COMPARISON_FUNC_GREATER_EQUAL } },
    { compare_op::always,        { GL_ALWAYS,   D3D11_COMPARISON_ALWAYS,        D3D12_COMPARISON_FUNC_ALWAYS } },
};


// =================================================================================================
// format

constexpr inline bool is_compressed_pixel_format(pixel_format fmt) 
{
    switch (fmt) 
    {
        case pixel_format::dxt1:
        case pixel_format::dxt3:
        case pixel_format::dxt5:
        case pixel_format::pvrtc2_rgb:
        case pixel_format::pvrtc4_rgb:
        case pixel_format::pvrtc2_rgba:
        case pixel_format::pvrtc4_rgba:
        case pixel_format::etc2_rgb8:
        case pixel_format::etc2_srgb8:
            return true;
        default:
            return false;
    }
}

constexpr /*inline*/ static bool is_dpeth_stencil_pixel_format(pixel_format fmt)
{
    switch (fmt)
    {
        case pixel_format::depth:
        case pixel_format::depthstencil:
            return true;
        default:
            return false;
    }
}

} // namespace fay