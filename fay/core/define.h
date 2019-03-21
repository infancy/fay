#pragma once
#include <cstdint>

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

// TODO: struct pixel_format { ... };
enum class pixel_format
{
    none,

    rgba32f,
    rgb32f,
    rg32f,
    r32f,

    rgba16f,
    rgb16f,
    rg16f,
    r16f,

    rgba8,
    rgb8,
    rg8,
    r8,

    rgba4,

    rgb10_a2,
    rgb5_a1,
    rgb565,

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

inline uint32_t bytesize(pixel_format fmt)
{
    switch (fmt)
    {
        case fay::pixel_format::none:    return -1;

        case fay::pixel_format::rgba32f: return 16;
        case fay::pixel_format::rgb32f:  return 12;
        case fay::pixel_format::rg32f:   return 8;
        case fay::pixel_format::r32f:    return 4;

        case fay::pixel_format::rgba16f: return 8;
        case fay::pixel_format::rgb16f:  return 6;
        case fay::pixel_format::rg16f:   return 4;
        case fay::pixel_format::r16f:    return 2;

        case fay::pixel_format::rgba8:   return 4;
        case fay::pixel_format::rgb8:    return 3;
        case fay::pixel_format::rg8:     return 2;
        case fay::pixel_format::r8:      return 1;

        case fay::pixel_format::rgba4:   return 2;

        case fay::pixel_format::rgb10_a2: return 4;
        case fay::pixel_format::rgb5_a1:  return 2;
        case fay::pixel_format::rgb565:   return 2;

        case fay::pixel_format::depth:        return 32;
        case fay::pixel_format::depthstencil: return 32;

        case fay::pixel_format::dxt1: return -1;
        case fay::pixel_format::dxt3: return -1;
        case fay::pixel_format::dxt5: return -1;

        case fay::pixel_format::pvrtc2_rgb:  return -1;
        case fay::pixel_format::pvrtc4_rgb:  return -1;
        case fay::pixel_format::pvrtc2_rgba: return -1;
        case fay::pixel_format::pvrtc4_rgba: return -1;

        case fay::pixel_format::etc2_rgb8:  return -1;
        case fay::pixel_format::etc2_srgb8: return -1;

        default: return -1;
    }
}

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
