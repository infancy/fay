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

#pragma region render

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
	none, opengl, opengl_dsa, d3d11, d3d12, vulkan, count
};
// TODO: FAY_ENUM_CLASS_OPERATOR( render_backend_type )

#pragma endregion

// -------------------------------------------------------------------------------------------------
// resource

#pragma region resource

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

inline uint32_t bytesize(pixel_format fmt) // rename: bsize???
{
    int sz{};

    switch (fmt)
    {
        case fay::pixel_format::none:    sz = -1; break;

        case fay::pixel_format::rgba32f: sz = 16; break;
        case fay::pixel_format::rgb32f:  sz = 12; break;
        case fay::pixel_format::rg32f:   sz = 8; break;
        case fay::pixel_format::r32f:    sz = 4; break;

        case fay::pixel_format::rgba16f: sz = 8; break;
        case fay::pixel_format::rgb16f:  sz = 6; break;
        case fay::pixel_format::rg16f:   sz = 4; break;
        case fay::pixel_format::r16f:    sz = 2; break;

        case fay::pixel_format::rgba8:   sz = 4; break;
        case fay::pixel_format::rgb8:    sz = 3; break;
        case fay::pixel_format::rg8:     sz = 2; break;
        case fay::pixel_format::r8:      sz = 1; break;

        case fay::pixel_format::rgba4:   sz = 2; break;

        case fay::pixel_format::rgb10_a2: sz = 4; break;
        case fay::pixel_format::rgb5_a1:  sz = 2; break;
        case fay::pixel_format::rgb565:   sz = 2; break;

        case fay::pixel_format::depth:        sz = 32; break;
        case fay::pixel_format::depthstencil: sz = 32; break;

        case fay::pixel_format::dxt1: sz = -1; break;
        case fay::pixel_format::dxt3: sz = -1; break;
        case fay::pixel_format::dxt5: sz = -1; break;

        case fay::pixel_format::pvrtc2_rgb:  sz = -1; break;
        case fay::pixel_format::pvrtc4_rgb:  sz = -1; break;
        case fay::pixel_format::pvrtc2_rgba: sz = -1; break;
        case fay::pixel_format::pvrtc4_rgba: sz = -1; break;

        case fay::pixel_format::etc2_rgb8:  sz = -1; break;
        case fay::pixel_format::etc2_srgb8: sz = -1; break;

        default: sz = -1; break;
    }

    return sz;
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

#pragma endregion

} // namespace fay
