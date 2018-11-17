#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_RENDER_DEFINE_H
#define FAY_RENDER_DEFINE_H

#include "fay/core/fay.h"

namespace fay::render
{

// enum { ... };
// TODO: const_t, const32_t, constant...
constexpr inline uint32_t Invalid_id               = 0;
constexpr inline uint32_t Num_inflight_frames      = 2;
constexpr inline uint32_t Max_color_attachments    = 4;
constexpr inline uint32_t Max_shader_stages        = 3;
constexpr inline uint32_t Pass_max_buffers  = 4;
constexpr inline uint32_t Pass_max_textures = 12;
constexpr inline uint32_t Max_shaderstage_uniforms = 4;	// uniform_blocks
constexpr inline uint32_t Max_uniform_members      = 16;
constexpr inline uint32_t Max_vertex_attributes    = 16;
constexpr inline uint32_t Max_mipmaps              = 16;
constexpr inline uint32_t Max_texturearray_layers  = 128;

#define FAY_ENUM_CLASS_OPERATORS(E)                                                                           \
inline E operator~(const E a)      { return static_cast<E>(~static_cast<int>(a)); }                           \
inline E operator&(const E a, const E b) { return static_cast<E>(static_cast<int>(a)& static_cast<int>(b)); } \
inline E operator|(const E a, const E b) { return static_cast<E>(static_cast<int>(a)| static_cast<int>(b)); } \
inline E& operator|=(E& a, const E b) { a = a | b; return a; };                                               \
inline E& operator&=(E& a, const E b) { a = a & b; return a; };                                               \
inline bool is_set(const E val, const E flag) { return (val & flag) != (E)0; }

// TODO
enum class resource_state
{
	usable,
	alloc,
	//initial,
	valid,
	invalid,
};

// A resource usage hint describing the update strategy of buffers and images.
enum class resource_usage
{
	immutable, // initialize at creation and then never changed, this is the default strategy
	dynamic,   // 
	stream,    // updated each frame
};

// buffer enum

enum class buffer_type
{
	vertex,
	//index16,
	index,
    instance,
};

enum class /*vertex_*/attribute_usage
{
    position, 
    normal,

    tangent,
    bitangent,

    indices,
    weights,

    color0,
    color1,
    color2,
    color3,

    texcoord0,
    texcoord1,
    texcoord2,
    texcoord3,
}; 

enum class /*vertex_*/attribute_format
{
    float1,
    float2,
    float3,
    float4,
    floatx,

    // WARNING: The following type in opengl  need to normalized
    byte4,
    byte4x,

    ubyte4,
    ubyte4x,

    short2,
    short2x,

    short4,
    short4x,

    // uint10_x2,
};

// texture enum

enum class texture_type
{
	two,
	cube,
	three,
	array,
	//data,	// ues 2d texture as array of data
	//array_data
};

enum class filter_mode
{
	anisotropy, // only for d3d11
	nearest,
	linear,
	nearest_mipmap_nearest,
	nearest_mipmap_linear,
	linear_mipmap_nearest,
	linear_mipmap_linear,
};

enum class wrap_mode
{
	repeat,
	clamp_to_edge,
	mirrored_repeat,
};

enum class cube_face
{
	pos_x,
	neg_x,
	pos_y,
	neg_y,
	pos_z,
	neg_z,
};

enum class pixel_format
{
    none,
    rgba8,
    rgb8,
    rgba4,
    r5g6b5,
    r5g5b5a1,
    r10g10b10a2,
    rgba32f,
    rgba16f,

    r32f,
    r16f,
    l8,

    dxt1,
    dxt3,
    dxt5,

    depth,
    depthstencil,

    pvrtc2_rgb,
    pvrtc4_rgb,
    pvrtc2_rgba,
    pvrtc4_rgba,
    etc2_rgb8,
    etc2_srgb8,
};

// shader enum

enum class shader_type
{
	vertex,
	geometry,
	fragment,
};

enum class uniform_type
{
	invalid,
	float1,
	float2,
	float3,
	float4,
	mat4,
};

// pipeline enum

enum class pipeline_type
{
	rasterization,
	raytracing,
	compute,
};

enum class primitive_type
{
	points,
	lines,
	line_strip,
	triangles,
	triangle_strip,
};

enum class face_winding
{
	ccw,
	cw,
};

enum class cull_mode
{
	none,
	front,
	back,
};

// used for depth and stencil tests
enum class compare_func
{
	never,

	less,
	less_equal,

	equal,
	not_equal,

	greater,
	greater_equal,

	always,
};

enum class stencil_op
{
	keep,
	zero,
	replace,
	incr_clamp,
	decr_clamp,
	invert,
	incr_wrap,
	decr_wrap,
};

enum class blend_factor
{
	zero,
	one,

	src_color,
	one_minus_src_color,
	src_alpha,
	one_minus_src_alpha,

	dst_color,
	one_minus_dst_color,
	dst_alpha,
	one_minus_dst_alpha,

	src_alpha_saturated,

	blend_color,
	one_minus_blend_color,
	blend_alpha,
	one_minus_blend_alpha,
};

enum class blend_op
{
	add,
	subtract,
	reverse_subtract,
};

enum class color_mask
{
	none = (0x10),     /* special value for 'all channels disabled */
	r = (1 << 0),
	g = (1 << 1),
	b = (1 << 2),
	a = (1 << 3),
	rgb = 0x7,	// r & g & b
	rgba = 0xf, // r& g & b & a, or rgb & a
};
// TODO: enum op

// pass enum

// tood: remove
enum class action
{
	action_default,
	action_clear,
	action_load,
	action_dontcare,
};

// ------------------------------------------------

// buffer, texture, shader, uniform, pso
// device, context, effect, pass

// TODO: struct no ctor init, default ctor
#define FAY_RENDER_TYPE_ID( type )                  \
struct type##_id                                    \
{                                                   \
    uint32_t id;                                    \
    explicit type##_id(uint32_t i = 0) : id{ i } {} \
};

FAY_RENDER_TYPE_ID(buffer)
FAY_RENDER_TYPE_ID(texture)
FAY_RENDER_TYPE_ID(shader)
FAY_RENDER_TYPE_ID(pipeline)
FAY_RENDER_TYPE_ID(frame)

#undef FAY_RENDER_TYPE_ID

struct vertex_attribute
{
    attribute_usage  usage;
    attribute_format format;
    uint32_t         num;

    vertex_attribute() {}
    vertex_attribute(attribute_usage usage, attribute_format format, uint32_t num = 1)
    {
        this->usage  = usage;
        this->format = format;
        this->num    = num;
    }

    bool operator==(const vertex_attribute& va) const
    {
        return usage == va.usage && format == va.format && num == va.num;
    }
};

/*
class vertex_layout
{
public:
    vertex_layout() {}
    vertex_layout(const std::initializer_list<vertex_attribute> il) : attrs(il) {}

    uint32_t size() { return (uint32_t)attrs.size(); }
    bool operator==(const vertex_layout&& layout) const { return attrs == layout.attrs; }

public:
    static inline vertex_layout p3n3t2;

private:

    std::vector<vertex_attribute> attrs{};
};
*/
using vertex_layout = std::vector<vertex_attribute>;

// vertex buffer, index buffer, instance buffer
struct buffer_desc
{
    std::string_view name { "defult" };

    uint32_t    size     { 0 }; // vertex/index nums
    GLsizei     stride   { 4 }; // WARNNING: fay::render can't check if this value is right or not.
    const void* data     { nullptr }; // data's length is size * stride
    buffer_type type     { buffer_type::index };
    resource_usage usage { resource_usage::immutable };

    // used for vertex buffer, instance buffer
    vertex_layout layout{ 0 };

    // only used for instance buffer
    // instance buffer update update data per instance(or more), instead of updating it per vertex.
    uint32_t instance_rate{ 0 };

	/* gl specific */
	//uint32_t gl_buffers[num_inflight_frames];
	/* metal specific */
	//const void* mtl_buffers[num_inflight_frames];
	/* d3d11 specific */
	//const void* d3d11_buffer;
};

struct texture_desc
{
    std::string_view name{ "defult" };
	uint32_t width;
	uint32_t height;
	pixel_format pixel_format;
	//image_content content;
	texture_type type;
	resource_usage usage;
	union 
	{
		int depth;
		int layers;
	};
	int num_mipmaps;

	filter_mode min_filter;
	filter_mode max_filter;
	wrap_mode wrap_u;
	wrap_mode wrap_v;
	wrap_mode wrap_w;

	bool as_render_target;
	int rt_sample_count;

	uint32_t max_anisotropy;
	float min_lod;
	float max_lod;
	/* gl specific */
	//uint32_t gl_textures[num_inflight_frames];
	/* metal specific */
	//const void* mtl_textures[num_inflight_frames];
	/* d3d11 specific */
	//const void* d3d11_texture;

	/*
	texture_desc(texture_type type)
	{
		switch(type)
		{
			...
		}
	}
	*/
};

// shader sources(not filepath) + uniform blocks + texutres 
struct shader_desc
{
    std::string_view name{ "defult" };

    const char* vs{};
    const char* gs{};
    const char* fs{};

    vertex_layout layout {}; // TODO: auto generate it
	// uniforms
	// textures
};

struct pipeline_desc	// pipeline state object
{
    std::string_view name{ "defult" };

	// vertex layout and attribure
    // TODO???: set vertex layout in pipeline_desc

	// primitive type
	primitive_type primitive_type{ primitive_type::triangles };

	// shader
    // TODO???: set program in pipeline_desc

	// rasterization state
	bool alpha_to_coverage_enabled { false };
	cull_mode cull_mode            { cull_mode::back };
	face_winding face_winding      { face_winding::cw }; // used in D3D( and metal).
	int rasteriza_sample_count     { 0 };
	float depth_bias               { 0.f };
	float depth_bias_slope_scale   { 0.f };
	float depth_bias_clamp         { 0.f };

	// alpha-blending state
	bool blending_enabled            { false };

	blend_factor src_factor_rgb      { blend_factor::one };
	blend_factor dst_factor_rgb      { blend_factor::zero };
	blend_op blend_op_rgb            { blend_op::add };

	blend_factor src_factor_alpha    { blend_factor::one };
	blend_factor dst_factor_alpha    { blend_factor::zero };
	blend_op blend_op_alpha          { blend_op::add };

	color_mask color_write_mask      { color_mask::rgba };
	int color_attachment_count;
	pixel_format blend_color_format;
	pixel_format blend_depth_format;
	std::array<float, 4> blend_color { 0.f, 0.f, 0.f, 0.f };

	// depth-stencil state
	struct stencil_state
	{
		stencil_op fail_op          { stencil_op::keep };
		stencil_op depth_fail_op    { stencil_op::keep };
		stencil_op pass_op          { stencil_op::keep };
		compare_func compare_func   { compare_func::always }; // TODO: samename
	};

	bool depth_write_enabled        { false }; // ???
	compare_func depth_compare_func { compare_func::always };

	bool stencil_enabled            { false };
	stencil_state stencil_front     { };
	stencil_state stencil_back      { };
	uint8_t stencil_read_mask       { 0 };
	uint8_t stencil_write_mask      { 0 };
	uint8_t stencil_ref             { 0 };
};

struct attachment_desc_
{
    texture_id texture;
    int mip_level;
    union
    {
        int face;
        int layer;
        int slice;
    };
};

struct frame_desc
{
    std::string_view name{ "defult" };

    std::array<attachment_desc_, Max_color_attachments> color_attachments;
    attachment_desc_ depth_stencil_attachment;
    // clear_desc: none, color, depth, color_depth
};
// ---------------------------

struct device_desc
{
	size_t buffer_pool_size  { 128 };
	size_t texture_pool_size { 128 };
	size_t shader_pool_size  { 32 };
	size_t pipeline_pool_size{ 64 };
	size_t pass_pool_size    { 16 };
	size_t context_pool_size { 16 };

	// d3d11-specific
	const void* d3d11_device{};
	const void* d3d11_device_context{};
	const void* (*d3d11_render_target_view_cb)() {};
	const void* (*d3d11_depth_stencil_view_cb)() {};
};

// clear_strategy

// data + state + action
struct pass_desc
{
    std::string_view name{ "defult" };

    buffer_id index{ 0 };
    // buffer_id vertex{ 0 }; array<buffer_id, ...> instances;
    std::array<buffer_id,  Pass_max_buffers> buffers{}; // buffers[0] is default vertex buffer
    std::array<texture_id, Pass_max_textures> textures{};

	shader_id shd_id{ 0 };
	pipeline_id pipe_id{ 0 };
    // frame_id frm_id;
};

} // namespace fay

#endif // fay_render_define_h