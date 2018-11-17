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

enum class render_backend
{
	none, opengl, opengl_dsa, d3d11, count
};
// TODO: FAY_ENUM_CLASS_OPERATOR( render_backend )



// -------------------------------------------------------------------------------------------------
// resource



enum class model_format
{
	none, obj, fbx, gltf, blend, unknown, count
};

enum class texture_format
{
	none,
	diffuse, specular, ambient, emissive, height, alpha, parallax,
	normal, shininess, opacity, displace, lightmap, reflection, cubemap, shadowmap,
	unknown, 
	count
};



} // namespace fay

#endif // FAY_CORE_DEFINE_H
