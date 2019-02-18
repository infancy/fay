#pragma once

#include <variant>

#include "glm/glm.hpp"

#include "fay/core/enum.h"
#include "fay/core/define.h"
#include "fay/core/fay.h"
#include "fay/core/memory.h"
#include "fay/core/range.h"

namespace fay
{

// enum { ... };
// TODO: const_t, const32_t, constant...
constexpr inline uint32_t InvalidId              = 0;
constexpr inline uint32_t NumInflightFrames      = 2;
constexpr inline uint32_t MaxColorAttachments    = 4;
constexpr inline uint32_t MaxShaderStages        = 3;
constexpr inline uint32_t PassMaxBuffers         = 4;
constexpr inline uint32_t PassMaxTextures        = 12;
constexpr inline uint32_t MaxShaderstageUniforms = 4;	// uniform_blocks
constexpr inline uint32_t MaxUniformMembers      = 16;
constexpr inline uint32_t MaxVertexAttributes    = 16;
constexpr inline uint32_t MaxMipmaps             = 16;
constexpr inline uint32_t MaxTextureArrayLayers  = 128;

// -------------------------------------------------------------------------------------------------
// render features

enum class anti_aliasing
{
    none,
    //SSAA, // Super Sample Anti-Aliasing
    MSAA, // Multi Sample Anti-Aliasing, hardware built-in AA, will reduce some frame rates

    // Post Processing AA(PPAA)
    FXAA, // Fast approXimate Anti-Aliasing
    SMAA, // Sub-pixel Morphological Anti-Aliasing
    //TXAA,
};

// -------------------------------------------------------------------------------------------------
// render types

// A resource usage hint describing the update strategy of buffers and images.
enum class resource_usage
{
    // none,
	immutable, // initialize at creation and then never changed, this is the default strategy
	dynamic,   // 
	stream,    // updated each frame
};

// TODO
enum class resource_state
{
	usable,
	alloc,
	//initial,
	valid,
	invalid,
};

// buffer enum

enum class buffer_type
{
    // none,
	vertex,
	//index16,
	index,
    instance,
};

enum class /*vertex_*/attribute_usage
{
    // position2d,
    position, 
    normal,

    tangent,
    bitangent,

    bone_index,
    bone_weight,

    color0,
    color1,
    color2,
    color3,

    texcoord0,
    texcoord1,
    texcoord2,
    texcoord3,

    // instance
    instance_model,

    unknown0,
    unknown1,
    unknown2,
    unknown3,
}; 

enum class /*vertex_*/attribute_format
{
    float1,
    float2,
    float3,
    float4,
    // floatx, could use float1 * 16 or float4 * 4

    // WARNING: The following type in opengl maybe need to normalized
    byte4,
    ubyte4,

    short2,
    short4,

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

enum class texture_usage
{
    albedo,
    metallic_roughness,
    normal,
    occlusion,
    emissive, 
    
    unknown0,
    unknown1,
    unknown2,
    unknown3,
};

enum class render_target
{
    none          = 0b0000,
    color         = 0b0001,
    depth         = 0b0010,
    stencil       = 0b0100,
    depth_stencil = 0b1000,
    DepthStencil  = depth | stencil | depth_stencil
};
FAY_ENUM_CLASS_OPERATORS(render_target)

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

// core/define/
// enum class pixel_format

// shader enum

enum class shader_stage
{
	vertex,
	geometry,
	fragment,

    mesh,
};

enum class uniform_type
{
	invalid,

    bvec1, // bool
    bvec2,
    bvec3,
    bvec4,

    ivec1, // int
    ivec2,
    ivec3,
    ivec4,

    uvec1, // uint
    uvec2,
    uvec3,
    uvec4,

    vec1, // float
    vec2,
    vec3,
    vec4,
    mat2,
    mat3,
	mat4,
    mat2x2,
    mat2x3,
    mat2x4,
    mat3x2,
    mat3x3,
    mat3x4,
    mat4x2,
    mat4x3,
    mat4x4,

    // double
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
	ccw, // Counter-ClockWise order
	cw,  // ClockWise order
};

enum class cull_mode
{
	none,
	front,
	back,
};

// used for depth and stencil tests
enum class compare_op
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

enum class blend_mask
{
	none  = 0x10,  // special value for all channels disabled
	red   = 1 << 0,
	green = 1 << 1,
	blue  = 1 << 2,
	alpha = 1 << 3,
	rgb   = red | green | blue,
	rgba  = red | green | blue | alpha, // all channels enabled
};
FAY_ENUM_CLASS_OPERATORS(blend_mask)

// pass enum

// ...

enum class pass_type
{
    none     = 0x00,
    graphics = 1 << 0,
    compute  = 1 << 1,
    blit     = 1 << 2,
    all = graphics | compute | blit,
};

// ------------------------------------------------

// buffer, texture, shader, uniform, pso
// device, context, effect, pass

// TODO: struct no ctor init, default ctor
// TODO: uint32_T -> size_t
// TODO: bool() -> is_valid()
// TODO: value -> private: value
#define FAY_RENDER_TYPE_ID( type )                                          \
struct type##_id                                                            \
{                                                                           \
    uint32_t value;                                                         \
    explicit type##_id(uint32_t i = 0) : value{ i } {}                      \
    explicit operator bool() const { return value != 0u; }                  \
    bool operator==(type##_id right) const { return value == right.value; } \
    bool operator!=(type##_id right) const { return !operator==(right); }   \
};

FAY_RENDER_TYPE_ID(buffer)
FAY_RENDER_TYPE_ID(texture)
FAY_RENDER_TYPE_ID(shader)
FAY_RENDER_TYPE_ID(pipeline)
FAY_RENDER_TYPE_ID(frame)

#undef FAY_RENDER_TYPE_ID

// a matrix instance : { fay::attribute_usage::instance_matrix,  fay::attribute_format::floatx, 16 }
class vertex_attribute
{
public:
    friend class vertex_layout;

    vertex_attribute() {}
    vertex_attribute(attribute_usage usage, attribute_format format, size_t num = 1, size_t index = 0)
    {
        this->usage_  = usage;
        this->format_ = format;
        this->index_  = index;
        this->num_    = num;
    }

    attribute_usage  usage()  const { return usage_; }
    attribute_format format() const { return format_; }
    size_t           index()  const { return index_; }

    bool operator==(const vertex_attribute& va) const
    {
        return usage_ == va.usage_ && format_ == va.format_ && index_ == va.index_ && num_ == va.num_;
    }

private:
    attribute_usage  usage_{};
    attribute_format format_{};
    size_t           index_{};

    size_t           num_{};
};

struct attribute_detail // rename: attribute_num_bytesize
{
    size_t num;
    size_t size; // num * sizeof(T)
};

// nums, bytes of attribute_format
const /*inline*/ static enum_class_map<attribute_format, attribute_detail>
attribute_format_map
{
    { attribute_format::float1,  {1,  4} },
    { attribute_format::float2,  {2,  8} },
    { attribute_format::float3,  {3, 12} },
    { attribute_format::float4,  {4, 16} },

    { attribute_format::byte4,   {4, 4} },
    { attribute_format::ubyte4,  {4, 4} },

    { attribute_format::short2,  {2, 4} },
    { attribute_format::short4,  {4, 8} },

    //{ uint10_x2, {1, 4} },
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

// TODO: better way
class vertex_layout : public std::vector<vertex_attribute> //extends_sequence<std::vector<vertex_attribute>>
{
public:
    //using extends_sequence<std::vector<vertex_attribute>>::extends_sequence;
    using std::vector<vertex_attribute>::vector;

    vertex_layout(std::initializer_list<vertex_attribute> il)
    {
        for (const auto& attr : il)
        {
            if (attr.num_ == 1)
            {
                push_back({ attr.usage(), attr.format(), 1, 0 });
            }
            else
            {
                for (size_t i : range(attr.num_))
                    push_back({ attr.usage(), attr.format(), attr.num_, i });
            }
        }
    }

    size_t stride() const
    {
        size_t size{};

        for (const auto& attr : *this)
        {
            size += attribute_format_map.at(attr.format()).size;
        }

        return size;
    }
};

// vertex buffer, index buffer, instance buffer
struct buffer_desc
{
    std::string name { "defult" };

    uint32_t    size     {}; // vertex/index nums
    const void* data     {}; // data's length is size * layout.stride()
    // TODO: remove
    uint32_t    stride{}; // byte sizes of single element. WARNNING: fay can't check if this value is right or not.

    buffer_type type     {};
    resource_usage usage { resource_usage::immutable };

    // used for vertex buffer, instance buffer
    // TODO: improve
    vertex_layout layout{};

    // only used for instance buffer
    // instance buffer update data per instance(or more), instead of updating per vertex.
    // TODO: remove it
    // uint32_t instance_rate{};

    buffer_desc() = default;
    buffer_desc(buffer_type type)
    {
        this->type = type;

        switch (type)
        {
            case fay::buffer_type::vertex:
                break;
            case fay::buffer_type::index:
                stride = 4;
                break;
            case fay::buffer_type::instance:
                break;
            default:
                break;
        }
    }
};

struct texture_desc
{
    std::string name{ "defult" };

	uint32_t width{};
	uint32_t height{};
    uint32_t depth{}; // layers, used in texture3d or texture_array
    pixel_format pixel_format{}; // rename: format

    uint32_t                 size{}; // used for compressed texture
    std::vector<const void*> data{};

    texture_type type{};
    texture_usage _usage{}; // TODO: rename
	resource_usage usage{ resource_usage::immutable }; // update_rate rate
    // resource_state state{ resource_state::empty };

    filter_mode min_filter{ filter_mode::linear };
	filter_mode max_filter{ filter_mode::linear };
    uint32_t max_anisotropy{ 1 }; // 1 ~ 16
    bool mipmap{ true };
    // int num_mipmaps; // How many levels of mipmaps are generated

    wrap_mode wrap_u{ wrap_mode::repeat };
	wrap_mode wrap_v{ wrap_mode::repeat };
	wrap_mode wrap_w{ wrap_mode::repeat }; // used in texture_type::three

    float min_lod{ 0.f }; // -1000 ~ +1000 or 0 ~ 1000 ???
    float max_lod{ 1000.f }; // max_float

    render_target as_render_target{ render_target::color }; // used as render target or depth_stencil target is depended by pixel_format
    uint32_t rt_sample_count{ 1 }; // only used when texture is used as render_target or depth_stencil target

    texture_desc() = default;
    texture_desc(texture_type type)
    {
        this->type = type;

        switch (type)
        {
            case fay::texture_type::two:
                break;
            case fay::texture_type::cube:
            {
                // pos_x, neg_x, pos_y, neg_y, pos_z, neg_z
                wrap_u = wrap_mode::clamp_to_edge;
                wrap_v = wrap_mode::clamp_to_edge;
                break;
            }
            case fay::texture_type::three:
                break;
            case fay::texture_type::array:
                break;
            default:
                break;
        }
    }
};

// shader sources(not filepath) + uniform blocks + texutres 
struct shader_desc
{
    struct uniform_block
    {
        std::string name{};
        uint32_t size{}; // byte_size
        std::vector<uniform_type> types;

        bool operator==(const uniform_block& that) const
        {
            return
                (name == that.name) &&
                (size == that.size) &&
                (types == that.types);
        }
    };

    struct sampler
    {
        std::string name{};
        texture_type type{};

        bool operator==(const sampler& that) const
        {
            return
                (name == that.name) &&
                (type == that.type);
        }
    };

    std::string name{ "defult" };

    std::string vs{};
    std::string gs{};
    std::string fs{};

    std::vector<std::string> vertex_names{};
    vertex_layout layout {};

    // TODO: uniform
    // std::vector<uniform> uniforms;

    std::vector<uniform_block> uniform_blocks;

    // TODO: std::span_view
    size_t vs_samplers_sz{};
    size_t fs_samplers_sz{};
    std::vector<sampler> samplers{};
    //std::vector<sampler> vs_samplers{};
    //std::vector<sampler> fs_samplers{};
};

/*
    low level api: pipeline state object
*/
struct pipeline_desc
{
    std::string name{ "default" };

	// vertex layout and attribure
    // DOIT: set vertex layout in pipeline_desc

	// primitive type
	primitive_type primitive_type{ primitive_type::triangles };

	// shader
    // TODO???: set program in pipeline_desc

	// rasterization state
    cull_mode cull_mode{ cull_mode::back };
    face_winding face_winding{ face_winding::ccw };

	bool alpha_to_coverage_enabled { false };
	int rasteriza_sample_count     { 1 }; // if sc > 1, use multisample

    // bool polygon_offset_enabled
    // WARNNING: Not enabled
	float depth_bias               { 0.f };
	float depth_bias_slope_scale   { 0.f };
	float depth_bias_clamp         { 0.f }; // used on d3d11 

    // depth-stencil state
    struct stencil_state
    {
        compare_op compare_op{ compare_op::always };

        stencil_op fail_op{ stencil_op::keep }; // stencil_fail(don't care depth test result???)
        stencil_op depth_fail_op{ stencil_op::keep }; // stencil pass but depth fail
        stencil_op pass_op{ stencil_op::keep };

        // std::as_const
        bool operator==(const stencil_state& that) const
        {
            return
                (fail_op       == that.fail_op) &&
                (depth_fail_op == that.depth_fail_op) &&
                (pass_op       == that.pass_op) &&
                (compare_op  == that.compare_op);
        }
        bool operator!=(const stencil_state& that) const
        {
            return !operator==(that);
        }
    };

    bool depth_enabled{ true }; // depth_mask_write_enabled
    compare_op depth_compare_op{ compare_op::less_equal };
    
    bool stencil_enabled{ true }; // WARNNING: default open stencil // TODO: def close it

    stencil_state stencil_front{ };
    stencil_state stencil_back{ };
    uint8_t stencil_ref{ 0 }; // reference value
    uint8_t stencil_test_mask{ 0xff };

    uint8_t stencil_write_mask{ 0xff };

	// alpha-blending state
    struct blend_state
    {
        blend_factor src_factor{ blend_factor::one };
        blend_factor dst_factor{ blend_factor::zero };
        blend_op blend_op{ blend_op::add };

        bool operator==(const blend_state& that) const
        {
            return
                (src_factor == that.src_factor) &&
                (dst_factor == that.dst_factor) &&
                (blend_op   == that.blend_op);
        }
        bool operator!=(const blend_state& that) const
        {
            return !operator==(that);
        }
    };

	bool blend_enabled{ false };

    blend_state blend_rgb {};
    blend_state blend_alpha {};
    std::array<float, 4> blend_color{ 0.f, 0.f, 0.f, 0.f }; // if use constant color & alpha

     // WARNNING: used in debug check
    int color_attachment_count { 1 };
    pixel_format blend_color_format { pixel_format::rgb8 };
	pixel_format blend_depth_format { pixel_format::r8 };

    blend_mask blend_write_mask{ blend_mask::rgba };
};

struct attachment_desc
{
    texture_id tex_id{};
    // union { int face; int layer; int slice; };
    uint32_t layer{}; // texture::two(0), texture::cube(0~5), texture::array/three(0~depth-1)
    uint32_t level{}; // mipmap_level
};

/*
    low level api: (vulkan)render pass / (metal)RenderPassDesc
*/
struct frame_desc
{
    std::string name{ "defult" };

    uint32_t width{};
    uint32_t height{};

    std::vector<attachment_desc> render_targets{}; // color_attachments
    attachment_desc depth_stencil{};
};

// ---------------------------

struct render_desc
{
    // render_device
    std::string name{ "default" };
    render_backend_type render_backend_type{ render_backend_type::opengl };

	/*
    size_t   buffer_pool_size{ 128 };
	size_t  texture_pool_size{ 128 };
	size_t   shader_pool_size{  32 };
	size_t pipeline_pool_size{  64 };
	size_t     pass_pool_size{  16 };
	size_t  context_pool_size{  16 };
    */

    // feature
    anti_aliasing anti_aliasing{};

    // opengl-specific
    // TODO: check backend_type & window/device/context
    // struct GLFWwindow;
    void* glfw_window;

	// d3d11-specific
	const void* d3d11_device{};
	const void* d3d11_device_context{};
	const void* (*d3d11_render_target_view_cb)() {};
	const void* (*d3d11_depth_stencil_view_cb)() {};

    // **-specific
};

// clear_op

struct state_desc
{
    std::string name{ "defult" };

    shader_id shd_id{};
    pipeline_id pipe_id{};
    // frame_id frm_id{}; // use 0 as the framebuffer by default 
};

/*
    low level api: descriptor table/set
*/
struct resource_set
{
    std::string name{ "defult" };

    buffer_id index{};
    // buffer_id vertex{ 0 }; array<buffer_id, ...> instances;
    std::vector<buffer_id> buffers{}; // buffers[0] is default vertex buffer
    std::vector<texture_id> textures{};
    // texture sampler
};

enum class command_type
{
    none,

    begin_default_frame,
    begin_frame,
    end_frame,

    clear_color,
    clear_depth,
    clear_stencil,
    //clear_frame,

    set_viewport,
    set_scissor,
    // set_view

    apply_shader,
    apply_pipeline,
    // apply_state,

    bind_index,
    bind_buffer, 
    bind_named_texture,
    bind_textures,
    bind_uniform,
    bind_uniform_block,
    // bind_resource_set,

    update_buffer,
    update_texture,

    draw,
    draw_index,

    readback, // blit
    // copy_data, update_resource, apply_state(binding_state)
};

struct command // command/encoder
{
    using uniform = std::variant<bool, int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat2, glm::mat3, glm::mat4>;

    //std::string_view name{ "defult" };

    command_type type_{};

    // std::any, std::variant, union para { ... }
    int          int_{};
    uint32_t    uint_{};
    float      float_{};

    buffer_id   buf_{};
    texture_id  tex_{};
    shader_id   shd_{};
    pipeline_id pip_{};
    frame_id    frm_{};

    glm::uvec4 uint4_{};
    glm::vec4 float4_{};
    //glm::mat4 mat4_{};

    uniform uniform_;
    memory uniform_block_;

    std::string str_{};
    std::vector<uint32_t> uints_{};
    std::vector<texture_id> texs_{};
    std::vector<attribute_usage> attrs_{};

    command(command_type type) : type_(type) {}

    /*
    #ifdef FAY_DEBUG
        int command_priority; "clear" must less than "draw"
    #endif
    */
};

/*
    low level api: command buffer/list
*/

class renderable;
class material;

// TODO: rebuild(not reuse it)
class command_list
{
public:
    // std::vector<state_desc> states;
    // std::vector<resource_desc> resources;
    // std::vector<command_desc> commands;

    command_list() {}
    command_list(std::string name) : name_(std::move(name)) {}

    // -------------------------------------------------------------------------------------------------

    command_list& begin_frame(frame_id id)
    {
        auto& cmd = add_command(command_type::begin_frame);
        cmd.frm_ = id;

        return *this;
    }
    command_list& begin_default_frame(/*uint32_t x, uint32_t y, uint32_t w, uint32_t h*/)
    {
        add_command(command_type::begin_default_frame);

        return *this;
    }
    command_list& end_frame()
    {
        add_command(command_type::end_frame);

        return *this;
    }
    // command_list& bind_frame(frame_id id, uint32_t flags, std::vector<glm::vec4> colors, float depth, int stencil);

    // WARNNING: clear all color targets if 'targets' is empty.
    command_list& clear_color(glm::vec4 rgba = { 0.f, 0.f, 0.f, 1.f }, std::vector<uint32_t> targets = {})
    {
        auto& cmd = add_command(command_type::clear_color);
        cmd.float4_ = rgba;
        cmd.uints_ = targets;

        // return add_commoand(command_type::begin_frame, [](command& cmd) { cmd.float4_ = rgba; cmd.uints_ = targets; });

        return *this;
    }
    command_list& clear_depth(float depth = 1.f)
    {
        auto& cmd = add_command(command_type::clear_depth);
        cmd.float_ = depth;

        return *this;
    }
    command_list& clear_stencil(uint32_t stencil = 0)
    {
        auto& cmd = add_command(command_type::clear_stencil);
        cmd.uint_ = stencil;

        return *this;
    }

    command_list& clear_frame()
    {
        clear_color();
        clear_depth();
        clear_stencil();

        return *this;
    }
    
    // WARNNING: fay use top_left as origin(rather then bottom_left in opengl)
    command_list& set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        auto& cmd = add_command(command_type::set_viewport);
        cmd.uint4_ = { x, y, width, height };

        return *this;
    }
    command_list& set_scissor( uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        auto& cmd = add_command(command_type::set_scissor);
        cmd.uint4_ = { x, y, width, height };

        return *this;
    }

    command_list& apply_shader(const shader_id id)
    {
        auto& cmd = add_command(command_type::apply_shader);
        cmd.shd_ = id;

        return *this;
    }
    command_list& apply_pipeline(const pipeline_id id)
    {
        auto& cmd = add_command(command_type::apply_pipeline);
        cmd.pip_ = id;

        return *this;
    }
    //command_list& apply_state(const shader_id id, const pipeline_id id);

    command_list& begin_default(const pipeline_id pipe_id, const shader_id shd_id)
    {
        begin_default_frame();
        clear_frame();
        apply_pipeline(pipe_id);
        apply_shader(shd_id);

        return *this;
    }

    // -------------------------------------------------------------------------------------------------

    command_list& bind_texture(const texture_id id, const std::string sampler)
    {
        auto& cmd = add_command(command_type::bind_named_texture);
        cmd.tex_ = id;
        cmd.str_ = sampler;

        return *this;
    }

    // !!! you could write device->bind_textures(0, "xxx"); device will ignore this cmd if id is 0.
    command_list& try_bind_texture(const texture_id id, const std::string sampler)
    {
        if (id)
            bind_texture(id, sampler);

        return *this;
    }

    // !!! default bind all textures to **fragment shader**
    // !!! you could write device->bind_textures({id0, id1, 0, id3, 0, id5}); device will ignore 0.
    command_list& bind_textures(std::vector<texture_id> textures, shader_stage stage = shader_stage::fragment)
    {
        auto& cmd = add_command(command_type::bind_textures);
        cmd.texs_ = std::move(textures);
        cmd.uint_ = enum_cast(stage);

        return *this;
    }

    // -------------------------------------------------------------------------------------------------

    command_list& bind_index(const buffer_id id)
    {
        auto& cmd = add_command(command_type::bind_index);
        cmd.buf_ = id;

        return *this;
    }

    // WARNNING: only bind all buffers to **vertex shader**
    // command_list& bind_buffer(const std::vector<buffer_id> buffers);

    // WARNNING: if attrs is {}, it will use all attrs of buffer
    command_list& bind_vertex(const buffer_id id, std::vector<attribute_usage> attrs = {}) {   return bind_buffer(id, attrs, 0); }
    command_list& bind_instance(const buffer_id id, std::vector<attribute_usage> attrs = {}) { return bind_buffer(id, attrs, 1); }



    command_list& bind_uniform(std::string name, command::uniform uniform)
    {
        auto& cmd = add_command(command_type::bind_uniform);
        cmd.str_ = std::move(name);
        cmd.uniform_ = std::move(uniform);

        return *this;
    }
    command_list& bind_uniform_block(std::string ub_name, memory mem)
    {
        auto& cmd = add_command(command_type::bind_uniform_block);
        cmd.str_ = std::move(ub_name);
        cmd.uniform_block_ = std::move(mem);

        return *this;
    }

    //command_list& bind_vertex(const buffer_id id, std::vector<uint32_t> attrs = {}, std::vector<uint32_t> slots = {});
    //command_list& bind_instance(const buffer_id id, std::vector<uint32_t> attrs = {}, std::vector<uint32_t> slots = {});
    //command_list& bind_resource(const buffer_id id, const std::vector<buffer_id> buffers, const std::vector<texture_id> textures);

    // TODO: bind_vertex_attribute, allocate_vertex_data

    // command_list& update_buffer(const buffer_id id, const void* data, uint32_t size); // cache the data
    // command_list& update_texture(const texture_id id, const void* data, uint32_t size);

    // WARNNING: if count is 0, it will be computed by device automatically
    // WARNNING: if instance_count is 0, use 'draw', else use 'draw_instance'
    command_list& draw(uint32_t count = 0, uint32_t first = 0, uint32_t instance_count = 1)
    {
        auto& cmd = add_command(command_type::draw);
        cmd.uint4_ = { count, first, instance_count, 0 };

        return *this;
    }
    command_list& draw_index(uint32_t count = 0, uint32_t first = 0, uint32_t instance_count = 1)
    {
        auto& cmd = add_command(command_type::draw_index);
        cmd.uint4_ = { count, first, instance_count, 0 };

        return *this;
    }

    /*
    template<typename Renderable>
    command_list& draw(Renderable* renderable)
    {
        renderable->render(*this);

        return *this;
    }
    */
    command_list& draw(renderable* renderable);

    // command_list& draw_indirect();
    // command_list& draw_indexed_indirect();
    // command_list& compute();
    // command_list& blit(frame_id f1, frame_id f2); // transfer

    /* const */std::string_view name() const { return name_; }
    const std::vector<command>& commands_() const { return cmds_; }

private:
    command& add_command(command_type type)
    {
        cmds_.emplace_back(type);
        return cmds_.back();
    }

    command_list& bind_buffer(const buffer_id id, std::vector<attribute_usage> attrs, uint32_t instance_rate)
    {
        auto& cmd = add_command(command_type::bind_buffer);
        cmd.buf_ = id;
        cmd.attrs_ = std::move(attrs);
        cmd.uint_ = instance_rate;

        return *this;
    }

private:
    std::string name_{ "defult" };
    // pass_type type{};

    std::vector<command> cmds_{};
};

} // namespace fay