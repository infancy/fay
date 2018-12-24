#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_CORE_DEFINE_H
#define FAY_CORE_DEFINE_H

namespace fay
{



// -------------------------------------------------------------------------------------------------
// graphics



// -------------------------------------------------------------------------------------------------
// math



enum class math_feature
{
	none, simd, count
};



// -------------------------------------------------------------------------------------------------
// render



enum class render_feature 
{
	//instancing,

    texture_compression_dxt,
	texture_compression_pvrtc,
	texture_compression_atc,
	texture_compression_etc2,

	//texture_float,
	//texture_half_float,

	origin_bottom_left,
    origin_top_left,

	msaa_render_targets,
	packed_vertex_format_10_2,
	multiple_render_target,

	//texture_three,
	//texture_array,

	count
};

enum class render_backend_type
{
	none, opengl, opengl_dsa, d3d11, count
};
// TODO: FAY_ENUM_CLASS_OPERATOR( render_backend_type )



// -------------------------------------------------------------------------------------------------
// resource

enum class pixel_format
{
    none,

    rgba32f,
    rgba16f,

    rgba8,
    rgba4,

    rgb10_a2,
    rgb5_a1,

    rgb8,
    rgb565,

    rg8,

    r32f, // float
    r16f,
    r8,   // byte

    depth,
    depthstencil,

    dxt1,
    dxt3,
    dxt5,

    pvrtc2_rgb,
    pvrtc4_rgb,
    pvrtc2_rgba,
    pvrtc4_rgba,

    etc2_rgb8,
    etc2_srgb8,
};

enum class texture_format
{
	none,
	diffuse, specular, ambient, emissive, height, alpha, parallax,
	normal, shininess, opacity, displace, lightmap, reflection, cubemap, shadowmap,
	unknown, 
	count
};

enum class model_format
{
    none, obj, fbx, gltf, blend, unknown, count
};

} // namespace fay

#endif // FAY_CORE_DEFINE_H
