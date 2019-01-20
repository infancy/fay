//#ifdef FAY_IN_WINDOWS

#include <sstream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "fay/core/range.h"
#include "fay/render/backend.h"
#include "fay/render/define.h"
#include "fay/render/native_type.h"
#include "fay/render/pool.h"

using namespace std::string_literals;

namespace fay // ::render // ::opengl
{

// -------------------------------------------------------------------------------------------------
// helper macros

// static FAY_FORCE_INLINE void check_errors() { CHECK(glGetError() == GL_NO_ERROR); }
// TODO: in debug mode forces turned on error check, while in release it is optional.
#define glcheck_errors() CHECK(glGetError() == GL_NO_ERROR)

// -------------------------------------------------------------------------------------------------
// helper functions

namespace backend_opengl_func
{
    void gl_enabled(GLenum cap, bool flag)
    {
        if (flag)
            glEnable(cap);
        else
            glDisable(cap);
    }



    // -------------------------------------------------------------------------------------------------
    // helper functions
    GLenum pixel_internal_format(pixel_format fmt)
    {
        switch (fmt)
        {
            case pixel_format::none:
                return 0;

            case pixel_format::rgba32f:
                return GL_RGBA32F;
            case pixel_format::rgba16f:
                return GL_RGBA16F;

            case pixel_format::rgba8:
                return GL_RGBA8;
            case pixel_format::rgba4:
                return GL_RGBA4;

            case pixel_format::rgb5_a1:
                return GL_RGB5_A1;
            case pixel_format::rgb10_a2:
                return GL_RGB10_A2;

            case pixel_format::rgb8:
                return GL_RGB8;
            case pixel_format::rgb565:
                return GL_RGB565; // return GL_RGB5;

            case pixel_format::rg8:
                return GL_RG8;

            case pixel_format::r32f:
                return GL_R32F;
            case pixel_format::r16f:
                return GL_R16F;
            case pixel_format::r8:
                return GL_R8;

            case pixel_format::depth:
                return GL_DEPTH_COMPONENT16; // TODO: or 24, 32
            case pixel_format::depthstencil:
                return GL_DEPTH24_STENCIL8;

            case pixel_format::etc2_rgb8:
                return GL_COMPRESSED_RGB8_ETC2;
            case pixel_format::etc2_srgb8:
                return GL_COMPRESSED_SRGB8_ETC2;

            case pixel_format::dxt1:
            case pixel_format::dxt3:
            case pixel_format::dxt5:
            case pixel_format::pvrtc2_rgb:
            case pixel_format::pvrtc4_rgb:
            case pixel_format::pvrtc2_rgba:
            case pixel_format::pvrtc4_rgba:
            default:
                LOG(ERROR) << "shouldn't be here";
                return 0;
        }
    }
    GLenum pixel_external_format(pixel_format fmt)
    {
        switch (fmt)
        {
            case pixel_format::none:
                return 0;
            case pixel_format::rgba32f:
            case pixel_format::rgba16f:
            case pixel_format::rgba8:
            case pixel_format::rgba4:
            case pixel_format::rgb10_a2:
            case pixel_format::rgb5_a1:
                return GL_RGBA;
            case pixel_format::rgb8:
            case pixel_format::rgb565:
                return GL_RGB;
            case pixel_format::rg8:
                return GL_RG;
            case pixel_format::r32f:
            case pixel_format::r16f:
            case pixel_format::r8:
                return GL_RED;
            case pixel_format::depth:
                return GL_DEPTH_COMPONENT;
            case pixel_format::depthstencil:
                return GL_DEPTH_STENCIL;
            case pixel_format::etc2_rgb8:
                return GL_COMPRESSED_RGB8_ETC2;
            case pixel_format::etc2_srgb8:
                return GL_COMPRESSED_SRGB8_ETC2;
            case pixel_format::dxt1:
            case pixel_format::dxt3:
            case pixel_format::dxt5:
            case pixel_format::pvrtc2_rgb:
            case pixel_format::pvrtc4_rgb:
            case pixel_format::pvrtc2_rgba:
            case pixel_format::pvrtc4_rgba:
            default:
                LOG(ERROR) << "shouldn't be here";
                return 0;
        }
    }
    GLenum pixel_external_type(pixel_format fmt)
    {
        switch (fmt)
        {
            case pixel_format::rgba32f:
            case pixel_format::r32f:
                return GL_FLOAT;
            case pixel_format::rgba16f:
            case pixel_format::r16f:
                return GL_HALF_FLOAT;
            case pixel_format::rgba8:
            case pixel_format::rgb8:
            case pixel_format::rg8:
            case pixel_format::r8:
                return GL_UNSIGNED_BYTE;
            case pixel_format::rgb10_a2:
                return GL_UNSIGNED_INT_2_10_10_10_REV;
            case pixel_format::rgb5_a1:
                return GL_UNSIGNED_SHORT_5_5_5_1;
            case pixel_format::rgb565:
                return GL_UNSIGNED_SHORT_5_6_5;
            case pixel_format::rgba4:
                return GL_UNSIGNED_SHORT_4_4_4_4;
            case pixel_format::depth:
                // FIXME???
                return GL_UNSIGNED_SHORT;
            case pixel_format::depthstencil:
                // FIXME???
                return GL_UNSIGNED_INT_24_8;
            default:
                LOG(ERROR) << "shouldn't be here";
                return 0;
        }
    }
    // return internal_format, external_format, external_type
    std::array<GLenum, 3> gl_pixel_format(pixel_format fmt)
    {
        GLenum in_fmt = pixel_internal_format(fmt);
        GLenum ex_fmt = pixel_external_format(fmt);
        GLenum ex_type = pixel_external_type(fmt);
        return { in_fmt, ex_fmt, ex_type };
    }
}
using namespace backend_opengl_func;

namespace backend_opengl_type
{
    // -------------------------------------------------------------------------------------------------
    // helper types

    struct vertex_attribute_gl
    {
        GLuint        index;      // 0, 1, 2, 3
        GLint         size;       // float3 : 3, byte4: 4
        GLenum        type;       // GL_FLOAT, GL_SIZE
        GLboolean     normalized; // used for integral type, map it to -1.0~1.0(0.0~1.0)
        // GLsizei       stride;  // storge in buffer.stsride
        const GLvoid* offset;     // (void*)0, (void*)(size * sizeof(type)), ...

        // these variables are just for debug when processing a pass_desc. TODO: better way
        attribute_usage  usage;
        attribute_format format;
        uint32_t         num;

        static GLenum attribute_type(attribute_format fmt)
        {
            switch (fmt)
            {
                case fay::attribute_format::float1:
                case fay::attribute_format::float2:
                case fay::attribute_format::float3:
                case fay::attribute_format::float4:
                case fay::attribute_format::floatx:
                    return GL_FLOAT;
                case fay::attribute_format::byte4:
                case fay::attribute_format::byte4x:
                    return GL_BYTE;
                case fay::attribute_format::ubyte4:
                case fay::attribute_format::ubyte4x:
                    return GL_UNSIGNED_BYTE;
                case fay::attribute_format::short2:
                case fay::attribute_format::short2x:
                case fay::attribute_format::short4:
                case fay::attribute_format::short4x:
                    return GL_SHORT;
                default:
                    LOG(ERROR) << "shouldn't be here";
                    return 0;
            }
        }

        static GLboolean need_normalized(attribute_format fmt)
        {
            switch (fmt)
            {
                case fay::attribute_format::byte4:
                case fay::attribute_format::byte4x:
                case fay::attribute_format::ubyte4:
                case fay::attribute_format::ubyte4x:
                case fay::attribute_format::short2:
                case fay::attribute_format::short2x:
                case fay::attribute_format::short4:
                case fay::attribute_format::short4x:
                    return GL_TRUE;
                default:
                    return GL_FALSE;
            }
        }
    };

    struct buffer // : public buffer_desc
    {
        // init by buffer_desc
        std::string name{};
        uint32_t    size{};
        // const void* data{};
        GLenum      type{};
        GLenum      usage{};

        // used for vertex buffer, instance buffer
        GLsizei     stride{};
        std::vector<vertex_attribute_gl> layout{ 0 };
        // only used for instance buffer
        GLuint instance_rate{ 0 };

        // then assign others
        GLuint gid{}; // union { GLuint vbo; GLuint ibo; };
        //resource_state state; // TODO: remove it???

        buffer() = default;
        buffer(const buffer_desc& desc) : layout(desc.layout.size())
        {
            name = desc.name;
            size = desc.size;
            stride = desc.stride;
            type = buffer_type_map.at(desc.type).opengl;
            usage = resource_usage_map.at(desc.usage).opengl;

            if (desc.type == buffer_type::vertex || desc.type == buffer_type::instance)
            {

                uint32_t offset = 0;
                for (uint32_t i = 0; i < desc.layout.size(); ++i)
                {
                    auto& da = desc.layout[i];
                    auto& a = layout[i];

                    a.usage = da.usage;
                    a.format = da.format;
                    a.num = da.num; // normally num is 1

                    a.index = i; // i_, ix, ic, ii
                    a.type = vertex_attribute_gl::attribute_type(da.format);
                    a.normalized = vertex_attribute_gl::need_normalized(da.format);

                    // TODO
                    auto[sz, byte] = attribute_format_map.at(da.format);
                    a.size = sz * da.num;
                    a.offset = (GLvoid*)offset;

                    offset += byte * da.num; // e.g. {..., floatx, 8} -> size: 1 * 8, offset: 4 * 8
                }
            }

            if (desc.type == buffer_type::instance)
                instance_rate = desc.instance_rate;
        }
    };

    struct texture
    {
        // # init by texture_desc
        std::string name{};

        uint32_t width{};
        uint32_t height{};
        uint32_t layer{}; // depth

        GLenum type;
        GLenum usage;

        // remove
        GLenum internal_format, external_format, external_type;

        GLenum min_filter;
        GLenum max_filter;
        uint32_t max_anisotropy;

        GLenum wrap_u;
        GLenum wrap_v;
        GLenum wrap_w;

        float min_lod;
        float max_lod;


        GLenum render_target;
        uint32_t rt_sample_count;

        // # then assign others
        GLuint tbo{};
        // if use MSAA
        GLuint msaa_rbo{};

        // if used as depth_stencil target or msaa color target
        GLuint ds_rbo{};

        texture() = default;
        texture(const texture_desc& desc)
        {
            name = desc.name;

            width  = desc.width;
            height = desc.height;
            layer  = desc.depth;

            type = texture_type_map.at(desc.type).opengl;
            usage = resource_usage_map.at(desc.usage).opengl;

            min_filter = filter_mode_map.at(desc.min_filter).opengl;
            max_filter = filter_mode_map.at(desc.max_filter).opengl;
            wrap_u = wrap_mode_map.at(desc.wrap_u).opengl;
            wrap_v = wrap_mode_map.at(desc.wrap_v).opengl;
            wrap_w = wrap_mode_map.at(desc.wrap_w).opengl;

            render_target = render_target_map.at(desc.as_render_target).opengl;
            rt_sample_count  = desc.rt_sample_count;
        }
    };

    struct shader
    {
        constexpr static GLenum vs_type = GL_VERTEX_SHADER;
        constexpr static GLenum gs_type = GL_GEOMETRY_SHADER;
        constexpr static GLenum fs_type = GL_FRAGMENT_SHADER;

        // init by shader_desc
        std::string name{};

        struct uniform_block
        {
            std::string name{};
            uint32_t size{}; // byte_size
            GLuint ubo{};
        };

        std::vector<uniform_block> uniforms;

        // then assign others
        GLuint gid{}; // program_id

        shader() = default;
        shader(const shader_desc& desc)
        {
            name = desc.name;
            
            uniforms.resize(desc.uniform_blocks.size());
            for (auto i : range(desc.uniform_blocks.size()))
            {
                uniforms[i].name = desc.uniform_blocks[i].name;
                uniforms[i].size = desc.uniform_blocks[i].size;
            }
        }
    };

    struct pipeline
    {
        // init by pipeline_desc
        std::string name{};

        //
        GLenum primitive_type{ GL_TRIANGLES };

        //
        bool alpha_to_coverage_enabled{ false };
        GLenum cull_mode{ GL_BACK };
        GLenum face_winding = { GL_CW };
        int rasteriza_sample_count{ 0 };
        float depth_bias{ 0.f };
        float depth_bias_slope_scale{ 0.f };
        float depth_bias_clamp{ 0.f };

        // depth-stencil state
        struct stencil_state
        {
            GLenum fail_op{ GL_KEEP };
            GLenum depth_fail_op{ GL_KEEP };
            GLenum pass_op{ GL_KEEP };
            GLenum compare_op{ GL_ALWAYS };
        };
        
        bool depth_enabled{ true };
        GLenum depth_compare_op{ GL_ALWAYS };

        bool stencil_enabled{ true };
        stencil_state stencil_front{};
        stencil_state stencil_back{};
        uint8_t stencil_test_mask{ 0 };
        uint8_t stencil_write_mask{ 0 };
        uint8_t stencil_ref{ 0 };

        //
        struct blend_state
        {
            GLenum src_factor{ GL_ONE };
            GLenum dst_factor{ GL_ZERO };
            GLenum blend_op{ GL_FUNC_ADD };
        };

        bool blend_enabled{ false };

        blend_state blend_rgb;
        blend_state blend_alpha;

        blend_mask blend_write_mask{ blend_mask::rgba };
        int color_attachment_count{ 1 };
        GLenum blend_color_format;
        GLenum blend_depth_format;
        std::array<float, 4> blend_color{ 0.f, 0.f, 0.f, 0.f };

        // then assign others

        pipeline() = default;
        pipeline(const pipeline_desc& desc)
        {
            name = desc.name;

            // primitive type
            primitive_type = primitive_type_map.at(desc.primitive_type).opengl;

            // rasterization state
            alpha_to_coverage_enabled = desc.alpha_to_coverage_enabled;
            cull_mode = cull_mode_map.at(desc.cull_mode).opengl;
            face_winding = (desc.face_winding == face_winding::cw) ? GL_CW : GL_CCW;
            rasteriza_sample_count = desc.rasteriza_sample_count;
            depth_bias = desc.depth_bias;
            depth_bias_slope_scale = desc.depth_bias_slope_scale;
            depth_bias_clamp = desc.depth_bias_clamp;

            // depth-stencil state
            depth_enabled = desc.depth_enabled;
            depth_compare_op = compare_op_map.at(desc.depth_compare_op).opengl;

            auto copy_stencil_state = [](pipeline::stencil_state dst, const pipeline_desc::stencil_state src)
            {
                dst.fail_op       = stencil_op_map.at(src.fail_op).opengl;
                dst.depth_fail_op = stencil_op_map.at(src.depth_fail_op).opengl;
                dst.pass_op       = stencil_op_map.at(src.pass_op).opengl;
                dst.compare_op    = compare_op_map.at(src.compare_op).opengl;
            };

            stencil_enabled = desc.stencil_enabled;

            copy_stencil_state(stencil_front, desc.stencil_front);
            copy_stencil_state(stencil_back, desc.stencil_back);

            stencil_test_mask = desc.stencil_test_mask;
            stencil_write_mask = desc.stencil_write_mask;
            stencil_ref = desc.stencil_ref;

            // alpha-blending state
            auto copy_blend_state = [](pipeline::blend_state dst, const pipeline_desc::blend_state src)
            {
                dst.src_factor = blend_factor_map.at(src.src_factor).opengl;
                dst.dst_factor = blend_factor_map.at(src.dst_factor).opengl;
                dst.blend_op       = blend_op_map.at(src.blend_op).opengl;
            };

            blend_enabled = desc.blend_enabled;
            
            copy_blend_state(blend_rgb, desc.blend_rgb);
            copy_blend_state(blend_alpha, desc.blend_alpha);

            color_attachment_count = desc.color_attachment_count;
            blend_color_format = pixel_external_format(desc.blend_color_format);
            blend_depth_format = pixel_external_format(desc.blend_depth_format);
            blend_color = desc.blend_color;
            blend_write_mask = desc.blend_write_mask;
        }
    };

    struct attachment
    {
        texture_id tex_pid{};
        uint32_t layer;
        uint32_t level;
        mutable GLuint msaa_resolve_fbo{}; // only used when attachment is MSAA render target.

        attachment() {}
        attachment(const attachment_desc& desc)
        {
            tex_pid = desc.tex_id;
            layer = desc.layer;
            level = desc.level;
        }
    };

    /*
    if not use MSAA
        fbo bind tbo0, tbo1...
    else             
        fbo bind rbo0, ...            rboN
                 |                    |
                 V                    V
         copy to attach0.mr_fbo.tbo0, attachN.mr_fbo.tboN
    */
    struct frame
    {
        // # init by frame_desc
        std::string name{};

        uint32_t width{};
        uint32_t height{};

        // TODO??? : std::vector<texture_id> render_targets{};
        std::vector<attachment> render_targets{};
        attachment depth_stencil{};

        // # then assign others
        GLuint fbo{};
        //GLuint msaa_resolve_fbos{};

        frame() = default;
        frame(const frame_desc& desc)
        {
            name = desc.name;

            width = desc.width;
            height = desc.height;

            for (const auto& ad : desc.render_targets)
                render_targets.emplace_back(ad);
            depth_stencil = desc.depth_stencil;
        }
    };

    struct uniform_visitor 
    {
        void bind(bool value)  const { glUniform1i(location, (int)value); }
        void bind(int value)   const { glUniform1i(location, value); }
        void bind(float value) const { glUniform1f(location, value); }

        void bind(const glm::vec2& value) const { glUniform2fv(location, 1, &value[0]); }
        void bind(const glm::vec3& value) const { glUniform3fv(location, 1, &value[0]); }
        void bind(const glm::vec4& value) const { glUniform4fv(location, 1, &value[0]); }

        // glUniformMatrix*(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
        void bind(const glm::mat2& mat) const { glUniformMatrix2fv(location, 1, GL_FALSE, &mat[0][0]); }
        void bind(const glm::mat3& mat) const { glUniformMatrix3fv(location, 1, GL_FALSE, &mat[0][0]); }
        void bind(const glm::mat4& mat) const { glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]); }

        GLint location;
    };
}
using namespace backend_opengl_type;



class backend_opengl : public render_backend
{
public:
    backend_opengl(const render_desc& desc) 
        : render_backend(desc)
        , window_(static_cast<GLFWwindow*>(desc.glfw_window))
    {

        // TODO: add to render_desc
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);
        glEnable(GL_SCISSOR_TEST);
        glEnable(GL_PROGRAM_POINT_SIZE); // The point size can be modify in vertex shader

        glGenVertexArrays(1, &context_.vao);
        glBindVertexArray(context_.vao);

        // TODO: render_desc.default_frame
        // default frame is created by glfw_window

        // TODO: global msaa???
        if (feature_.use_msaa)
        {
            GLint para{};
            glGetIntegerv(GL_SAMPLE_BUFFERS, &para);
            if (para > 0)
            {
                glEnable(GL_MULTISAMPLE);
            }
            else
            {
                LOG(ERROR) << "device_opengl doesn't support MSAA";
            }
        }

        glcheck_errors();
    }
    ~backend_opengl()
    {
        glDeleteVertexArrays(1, &context_.vao);
    }

	// resource creation, updating and destruction
	buffer_id   create(const   buffer_desc& desc) override
	{
        glcheck_errors();

        // or id = buffer_pool.insert(besc), generate buffer by itself ???
        buffer_id pid = pool_.insert(desc); // id in the pool
        buffer& buf = pool_[pid];

        // TODO : dynamic, stream have more than one 

        glGenBuffers(1, &buf.gid); // vbo or ibo
        glBindBuffer(buf.type, buf.gid);
        glBufferData(buf.type, buf.size * buf.stride, nullptr, buf.usage);

        if (buf.usage == GL_STATIC_DRAW)
        {
            DCHECK(desc.data != nullptr);
            glBufferSubData(buf.type, 0, buf.size * buf.stride, desc.data);
        }

        glcheck_errors();
        return pid;
	}
	texture_id  create(const  texture_desc& desc) override
	{
        glcheck_errors();

        texture_id pid = pool_.insert(desc); // id in the pool
        texture& tex = pool_[pid];

        if (!is_dpeth_stencil_pixel_format(desc.pixel_format))
        {
            glGenTextures(1, &tex.tbo);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(tex.type, tex.tbo);

            if (tex.type != GL_TEXTURE_2D_MULTISAMPLE)
            {
                glTexParameteri(tex.type, GL_TEXTURE_MIN_FILTER, tex.min_filter);
                glTexParameteri(tex.type, GL_TEXTURE_MAG_FILTER, tex.max_filter);
                // glTexParameteri(tex.type, GL_TEXTURE_MAX_ANISOTROPY_EXT, tex.max_anisotropy);

                glTexParameteri(tex.type, GL_TEXTURE_WRAP_S, tex.wrap_u);
                glTexParameteri(tex.type, GL_TEXTURE_WRAP_T, tex.wrap_v);
                if (tex.type == GL_TEXTURE_3D)
                // if (tex.type == GL_PROXY_TEXTURE_3D || tex.type == GL_TEXTURE_CUBE_MAP)
                    glTexParameteri(tex.type, GL_TEXTURE_WRAP_R, tex.wrap_w);

                //glTexParameterf(tex.type, GL_TEXTURE_MIN_LOD, std::clamp(tex.min_lod, 0.f, 1000.f));
                //glTexParameterf(tex.type, GL_TEXTURE_MAX_LOD, std::clamp(tex.max_lod, 0.f, 1000.f));
            }
            else
            {
                // TODO...
            }
            glcheck_errors();

            bool is_compressed = is_compressed_pixel_format(desc.pixel_format);
            auto[in_fmt, ex_fmt, ex_type] = gl_pixel_format(desc.pixel_format);

            if (tex.type == GL_TEXTURE_2D || tex.type == GL_TEXTURE_CUBE_MAP)
            {
                uint32_t nums = tex.type == GL_TEXTURE_2D ? 1 : 6;

                for (uint32_t i = 0; i < nums; ++i)
                {
                    GLenum target = tex.type == GL_TEXTURE_2D ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;

                    if (is_compressed)
                        glCompressedTexImage2D(target, 0, in_fmt,
                            desc.width, desc.height, 0, desc.size, desc.data[i]); // TODO: even data is nullptr, it is not problem.
                    else
                        glTexImage2D(target, 0, in_fmt,
                            desc.width, desc.height, 0, ex_fmt, ex_type, desc.data[i]);
                }
            }
            else if (tex.type == GL_TEXTURE_2D_ARRAY || tex.type == GL_TEXTURE_3D)
            {
                // WARNNING: data or data3d, only one is available
                if (is_compressed)
                    glCompressedTexImage3D(tex.type, 0, in_fmt,
                        desc.width, desc.height, desc.depth, 0, desc.size, desc.data.front()); // TODO: even data3d is nullptr, it is not problem.
                else
                    glTexImage3D(tex.type, 0, in_fmt,
                        desc.width, desc.height, desc.depth, 0, ex_fmt, ex_type, desc.data.front());

                if (!desc.data.empty())
                    update(pid, nullptr); // TODO
            }
            else
            {
                LOG(ERROR) << "shouldn't be here";
            }
            glcheck_errors();

            // TODO: koi, generate mipmap by itself.
            if (desc.mipmap)
            {
                glGenerateMipmap(tex.type);
                glTexParameteri(tex.type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            }
            glcheck_errors();

            if ((desc.as_render_target == render_target::color) &&
                (render_desc_.anti_aliasing == anti_aliasing::MSAA))
            {
                glGenRenderbuffers(1, &tex.msaa_rbo);
                glBindRenderbuffer(GL_RENDERBUFFER, tex.msaa_rbo);
                glRenderbufferStorageMultisample(GL_RENDERBUFFER, tex.rt_sample_count, 
                    pixel_internal_format(desc.pixel_format), tex.width, tex.height);
            }
        }
        else // depth_stencil
        {
            glGenRenderbuffers(1, &tex.ds_rbo);
            glBindRenderbuffer(GL_RENDERBUFFER, tex.ds_rbo);
            
            // TODO : GL_DEPTH24_STENCIL8
            //GLenum gl_depth_format = _sg_gl_depth_attachment_format(img->pixel_format);

            if (render_desc_.anti_aliasing == anti_aliasing::MSAA)
            {
                glRenderbufferStorageMultisample(GL_RENDERBUFFER, tex.rt_sample_count, GL_DEPTH24_STENCIL8, tex.width, tex.height);
            }
            else
            {
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, tex.width, tex.height);
            }
        }

        glcheck_errors();
        return pid;
	}
	shader_id   create(const   shader_desc& desc) override
	{
        glcheck_errors();

        shader_id pid = pool_.insert(desc); // id in the pool
        shader& shader = pool_[pid];

        std::vector<GLuint> ids; // shader_id

        ids.push_back(compile_shader(shader.name, shader::vs_type, desc.vs.c_str()));
        ids.push_back(compile_shader(shader.name, shader::fs_type, desc.fs.c_str()));

        if (desc.gs.length() > 0)
            ids.push_back(compile_shader(shader.name, shader::gs_type, desc.gs.c_str()));

        shader.gid = link_shader(shader.name, ids); // program_id
        glcheck_errors();

        // create uniform blocks
        // GL_DYNAMIC_DRAW
        for (auto& ub : shader.uniforms)
        {
            glGenBuffers(1, &ub.ubo);
            glBindBuffer(GL_UNIFORM_BUFFER, ub.ubo);
            glBufferData(GL_UNIFORM_BUFFER, ub.size, NULL, GL_DYNAMIC_DRAW); // TODO: or GL_STREAM_DRAW??? 
        }

        glcheck_errors();
        return pid;
	}
	pipeline_id create(const pipeline_desc& desc) override
	{
        glcheck_errors();

        // Since there is no actual operation to create a pipeline state object, why should it be created? 
        // In order to check if there is a problem with the combination of objects of pipeline in advance.

        pipeline_id pid = pool_.insert(desc); // id in the pool
        //pipeline& pipe = pool_[pid];

        // noop

        glcheck_errors();
        return pid;
	}
	frame_id    create(const    frame_desc& desc) override
	{
        glcheck_errors();

        frame_id pid = pool_.insert(desc); // id in the pool
        frame& frm = pool_[pid];

        glGenFramebuffers(1, &frm.fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, frm.fbo);

        // depth_stencil target

        if (desc.depth_stencil.tex_id)
        {
            const auto& tex = pool_[frm.depth_stencil.tex_pid];

            glFramebufferRenderbuffer(GL_FRAMEBUFFER, tex.render_target, GL_RENDERBUFFER, tex.ds_rbo);
        }

        // render target

        auto bind_render_target_to_texture = [this](int target_i, const attachment& attach)
        {
            const auto& tex = pool_[attach.tex_pid];

            switch (tex.type)
            {
                case GL_TEXTURE_2D:
                    // glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + target_i, attach.tbo, attach.mipmap_level);
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + target_i, GL_TEXTURE_2D, tex.tbo, attach.level);
                    break;

                case GL_TEXTURE_CUBE_MAP:
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + target_i, GL_TEXTURE_CUBE_MAP_POSITIVE_X + attach.layer, tex.tbo, attach.level);
                    break;

                default:
                    // texture3D or texture2d_array
                    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + target_i, tex.tbo, attach.level, attach.layer);
                    break;
            }
        };

        const auto& rts = frm.render_targets;

        if (!(render_desc_.anti_aliasing == anti_aliasing::MSAA))
        { // attach texture
            for (int i = 0; i < rts.size(); ++i)
            {
                bind_render_target_to_texture(i, rts[i]);
            }
        }
        else
        { // attach msaa render buffer
            for (int i = 0; i < rts.size(); ++i)
            {
                const auto& attach = rts[i];
                const auto& tex = pool_[attach.tex_pid];
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_RENDERBUFFER, tex.msaa_rbo);
            }

            // below: create MSAA resolve framebuffers to read data from tex.rbo to tex.tbo
        }        
        // check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            LOG(ERROR) << "Framebuffer completeness check failed!\n";
        }

        // if ues MSAA, create MSAA resolve framebuffers to read data from tex.msaa_rbo to tex.tbo
        if (render_desc_.anti_aliasing == anti_aliasing::MSAA)
        {
            for (int i = 0; i < rts.size(); ++i)
            {
                const auto& attach = rts[i];

                glGenFramebuffers(1, &attach.msaa_resolve_fbo);
                glBindFramebuffer(GL_FRAMEBUFFER, attach.msaa_resolve_fbo);
                bind_render_target_to_texture(0, attach);

                if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                {
                    LOG(ERROR) << "MSAA resolve framebuffer " << i << " completeness check failed\n";
                }
            }
        }

        glcheck_errors();
        return pid;
	}

	void destroy(  buffer_id id) override
	{
        auto gid = pool_[id].gid;
        // https://stackoverflow.com/questions/27937285/when-should-i-call-gldeletebuffers
        // void glDeleteBuffers(GLsizei n, const GLuint* buffers);
        // glDeleteBuffers(1, &gid);
        pool_.erase(id);
	}
	void destroy( texture_id id) override
	{
        auto gid = pool_[id].tbo;
        pool_.erase(id);
	}
	void destroy(  shader_id id) override
	{
        auto gid = pool_[id].gid;
        // glDeleteProgram(gid);
        pool_.erase(id);
	}
	void destroy(pipeline_id id) override
	{
        pool_.erase(id);
	}
	void destroy(   frame_id id) override
	{
        pool_.erase(id);
	}

	// render
    void begin_frame(frame_id id) override
    {
        ctx_ = command_list_context{}; // stateless

        glcheck_errors();

        // clear_state

        if (id.value != 0) // offscreen frame
        {
            ctx_.is_offscreen = true;
            ctx_.frm = pool_[id];
            glBindFramebuffer(GL_FRAMEBUFFER, ctx_.frm.fbo);

            // TODO!!!
            // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
            unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
            DCHECK(ctx_.frm.render_targets.size() < 4);
            glDrawBuffers(ctx_.frm.render_targets.size(), attachments);
        }
        else // default frame
        {
            ctx_.is_offscreen = false;
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        glcheck_errors();

        log0_ << ("frame    : "s + (ctx_.is_offscreen ? ctx_.frm.name : "0") + '\n');
    }
    void end_frame() override
    {
        glcheck_errors();

        // if use MSAA in offscreen render, copy data from tex.rbo to tex.tbo
        if (ctx_.is_offscreen && feature_.use_msaa)
        {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, ctx_.frm.fbo);

            const int w = ctx_.frm.width;
            const int h = ctx_.frm.height;

            for (uint32_t i = 0; i < ctx_.frm.render_targets.size(); ++i)
            {
                glReadBuffer(GL_COLOR_ATTACHMENT0 + i);

                const auto& attach = ctx_.frm.render_targets[i];
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, attach.msaa_resolve_fbo);
                // WARNNING???: that's ok
                //const GLenum gl_att = GL_COLOR_ATTACHMENT0;
                //glDrawBuffers(1, &gl_att);
                glDrawBuffer(GL_COLOR_ATTACHMENT0);
                glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
            }

            glcheck_errors();
        }

        glcheck_errors();

        // log info
    #ifdef FAY_DEBUG_
        std::cout << "\nrender backend: opengl\n\n";
        std::cout << log0_.rdbuf();
        std::cout << '\n';
    #endif // FAY_DEBUG

        log0_.clear();
    }

    void clear_color(glm::vec4 rgba, std::vector<uint32_t> targets) const override
    {
        glcheck_errors();

        if (ctx_.is_offscreen)
        {
            //glViewport(0, 0, ctx_.frm.width, ctx_.frm.height);
            glViewport(0, 0, ctx_.frm.width, ctx_.frm.height);
            glScissor(0, 0, ctx_.frm.width, ctx_.frm.height);

            for (auto i : targets)
            {
                DCHECK(ctx_.frm.render_targets[i].tex_pid);
                glClearBufferfv(GL_COLOR, i, &rgba[0]);
            }
        }
        else
        {
            int width{}, height{};
            glfwGetFramebufferSize(window_, &width, &height);

            //glViewport(0, 0, width, height);
            glViewport(0, 0, width, height);
            glScissor(0, 0, width, height);
            glClearBufferfv(GL_COLOR, 0, &rgba[0]);
        }

        glcheck_errors();
    }
    void clear_depth(float depth) const override
    {
        glcheck_errors();

        if (ctx_.is_offscreen)
        {
            glViewport(0, 0, ctx_.frm.width, ctx_.frm.height);
            glScissor(0, 0, ctx_.frm.width, ctx_.frm.height);

            DCHECK(ctx_.frm.depth_stencil.tex_pid);
        }
        else
        {
            int width{}, height{};
            glfwGetFramebufferSize(window_, &width, &height);

            glViewport(0, 0, width, height); // TODO: remove
            glScissor(0, 0, width, height);
        }
        glClearBufferfv(GL_DEPTH, 0, &depth);

        glcheck_errors();
    }
    void clear_stencil(uint32_t stencil) const override
    {
        glcheck_errors();

        if (ctx_.is_offscreen)
        {
            glViewport(0, 0, ctx_.frm.width, ctx_.frm.height);
            glScissor(0, 0, ctx_.frm.width, ctx_.frm.height);

            DCHECK(ctx_.frm.depth_stencil.tex_pid);
        }
        else
        {
            int width{}, height{};
            glfwGetFramebufferSize(window_, &width, &height);

            glViewport(0, 0, width, height);
            glScissor(0, 0, width, height);
        }
        glcheck_errors();
        const GLint gl_stencil = stencil;
        // glClearNamedFramebufferiv(id_, GL_STENCIL, 0, &stencil);
        // glClearBufferuiv(GL_STENCIL, 0, &gl_stencil);
        glClearBufferiv(GL_STENCIL, 0, &gl_stencil);

        glcheck_errors();
    }

    void set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override
    {
        glcheck_errors();

        glViewport(x, y, width, height);

        glcheck_errors();
    }
    void set_scissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override
    {
        glcheck_errors();

        glScissor(x, y, width, height);

        glcheck_errors();
    }

    void apply_pipeline(const pipeline_id id, std::array<bool, 4> flags) override
    {
        glcheck_errors();

        ctx_.pipe = pool_[id];
        const auto& pipe = ctx_.pipe;

        // primitive type
        if (flags[0])
        {
            glcheck_errors();
        }

        // rasterization state
        if (flags[1])
        {
            // TODO: gl_enabled(GL_CULL_FACE, pipe.cull_mode != GL_NONE);
            if(pipe.cull_mode == GL_NONE)
                glDisable(GL_CULL_FACE);
            else
            {
                glEnable(GL_CULL_FACE);
                glCullFace(pipe.cull_mode);
            }

            glFrontFace(pipe.face_winding);

            gl_enabled(GL_SAMPLE_ALPHA_TO_COVERAGE, pipe.alpha_to_coverage_enabled);
            gl_enabled(GL_MULTISAMPLE, pipe.rasteriza_sample_count > 1);

            // glEnable(GL_POLYGON_OFFSET_FILL);
            // glPolygonOffset(pipe.depth_bias_slope_scale, pipe.depth_bias);

            glcheck_errors();
        }

        // depth-stencil state
        if (flags[2])
        {
            // glEnable(GL_DEPTH_TEST); always open the depth test???
            glDepthFunc(pipe.depth_compare_op);
            // void glDepthMask(GLboolean flag);
            glDepthMask(pipe.depth_enabled);

            glcheck_errors();

            // gl_enabled(GL_STENCIL_TEST, pipe.stencil_enabled);
            // auto apply_stencil_state = [](const pipeline::stencil_state ss, GLenum face)   
            for (auto i : range(2))
            {
                const auto& ss = (i == 0) ? pipe.stencil_front : pipe.stencil_back;
                GLenum face = (i == 0) ? GL_FRONT : GL_BACK;

                // mask: ANDed with **both the reference value and the stored stencil value** before the test
                glStencilFuncSeparate(face, ss.compare_op, pipe.stencil_ref, pipe.stencil_test_mask);
                glStencilOpSeparate(face, ss.fail_op, ss.depth_fail_op, ss.pass_op);
            }
            // void glStencilMask(	GLuint mask);
            if(pipe.stencil_enabled)
                glStencilMask(0);
            else
                glStencilMask(pipe.stencil_write_mask);

            glcheck_errors();
        }

        // alpha-blending state
        if (flags[3])
        {
            gl_enabled(GL_BLEND, pipe.blend_enabled);

            auto[r, g, b, a] = pipe.blend_color;
            glBlendColor(r, g, b, a);

            glBlendFuncSeparate(
                  pipe.blend_rgb.src_factor,   pipe.blend_rgb.dst_factor,
                pipe.blend_alpha.src_factor, pipe.blend_alpha.dst_factor);

            glBlendEquationSeparate(pipe.blend_rgb.blend_op, pipe.blend_alpha.blend_op);

            // TODO: Simplified way
            glColorMask(
                enum_have(pipe.blend_write_mask, blend_mask::red),
                enum_have(pipe.blend_write_mask, blend_mask::green),
                enum_have(pipe.blend_write_mask, blend_mask::blue),
                enum_have(pipe.blend_write_mask, blend_mask::alpha));

            glcheck_errors();
        }

        glcheck_errors();
        log0_ << ("pipeline : "s + ctx_.pipe.name + '\n');
    }
    void apply_shader(const shader_id id) override
    {
        glcheck_errors();

        ctx_.shd = pool_[id];
        glUseProgram(ctx_.shd.gid);

        glcheck_errors();
        log0_ << ("shader   : "s + ctx_.shd.name + "\n");
    }


    // TODO: rename uniform_block
    void bind_uniform(uint32_t ub_index, const void* data, uint32_t size) override
    {
        glcheck_errors();

        GLuint sho = ctx_.shd.gid;
        GLuint ubo = ctx_.shd.uniforms[ub_index].ubo;
        uint32_t bind_point = ub_index;
        const char* ub_name = ctx_.shd.uniforms[ub_index].name.c_str();

        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);

        glUniformBlockBinding(sho, glGetUniformBlockIndex(sho, ub_name), bind_point);
        glBindBufferBase(GL_UNIFORM_BUFFER, bind_point, ubo);

        glcheck_errors();
        log0_ << ("uniform block : "s + ub_name + '\n');
    }

    void bind_uniform(const char* name, command::uniform uniform) override
    {
        glcheck_errors();

        uniform_visitor visitor{ glGetUniformLocation(ctx_.shd.gid, name) };

        std::visit([visitor](auto&& t) { visitor.bind(t); }, uniform);

        glcheck_errors();
        log0_ << ("uniform  : "s + std::to_string(uniform.index()) + '\n');
    }

    void bind_index(const buffer_id id) override
    {
        ctx_.index_id = id;

        glcheck_errors();

        const auto& index = pool_[id];
        glBindBuffer(index.type, index.gid);

        glcheck_errors();
        log0_ << ("index    : "s + (ctx_.index_id.value == 0 ? "null" : pool_[ctx_.index_id].name) + '\n');
    }
    void bind_vertex(const buffer_id id, std::vector<uint32_t> attrs, std::vector<uint32_t> slots, uint32_t instance_rate) override
    {
        ctx_.buf_ids.push_back(id);

        glcheck_errors();

        // set vertex/instance buffer and vertex layout
        // TODO: cache vertex attritube binding

        const auto& buf = pool_[id];
        const auto& layout = buf.layout;

        // TODO: move to device
        // TODO: remove
        if (attrs.size() == 0) // use all vertex attribute
        {
            auto attrs_num = layout.size();

            attrs.resize(attrs_num);
            std:iota(attrs.begin(), attrs.end(), 0u);

            slots = attrs;
        }

        glBindBuffer(buf.type, buf.gid);
        for (int i = 0; i < buf.layout.size(); ++i)
        {
            auto& attr = layout[attrs[i]];
            auto  slot = slots[i];

            glEnableVertexAttribArray(slot);
            glVertexAttribPointer(slot, attr.size, attr.type, attr.normalized, buf.stride, attr.offset);
            if (instance_rate > 0)
                glVertexAttribDivisor(slot, instance_rate);
        }

        glcheck_errors();
        log0_ << ("buffer   : "s + pool_[id].name + '\n');
    }
    void bind_texture(const texture_id id, int tex_unit, const std::string& sampler) override
    {
        ctx_.tex_ids.push_back(id);

        glcheck_errors();

        glActiveTexture(GL_TEXTURE0 + tex_unit);	    // active Nth texture unit

        // TODO: cache location
        glUniform1i(glGetUniformLocation(ctx_.shd.gid, sampler.c_str()), tex_unit); // bind sampler to tex_unit

        const auto& tex = pool_[id];
        glBindTexture(tex.type, tex.tbo);	// bind texture to tex_unit

        glcheck_errors();
        log0_ << ("texture  : "s + pool_[id].name + '\n');
    }

    void update(buffer_id id, const void* data, int size) override
    {


        glcheck_errors();
    }
    void update(texture_id id, const void* data) override
    {
        glcheck_errors();
    }



    void draw(uint32_t count, uint32_t first, uint32_t instance_count) override
    {
        glcheck_errors();

        if (instance_count == 1)
        {
            glDrawArrays(ctx_.pipe.primitive_type, first, count);
        }
        else
        {
            glDrawArraysInstanced(ctx_.pipe.primitive_type, first, count, instance_count);
        }

        glcheck_errors();
        log0_ << "draw\n\n";
    }
    void draw_index(uint32_t count, uint32_t first, uint32_t instance_count) override
    {
        glcheck_errors();

        const GLvoid* indices = (const GLvoid*)(GLintptr)(first * sizeof(GLuint)); // first_element*4 + base_offset

        if (instance_count == 1)
            glDrawElements(ctx_.pipe.primitive_type, count, GL_UNSIGNED_INT, indices);
        else
            glDrawElementsInstanced(ctx_.pipe.primitive_type, count, GL_UNSIGNED_INT, indices, instance_count);

        glcheck_errors();
        log0_ << "draw_index\n\n";
    }

    /*
    void execute(command_list desc) override
    {
        cache_.passes.emplace_back(std::move(desc));
        draw();
    }

	void submit(command_list desc) override
	{
        cache_.passes.emplace_back(std::move(desc));
	}

    void execute()
    {
        // set
        // complie
        // execute: draw/compute/blit
    }

    void draw() override
    {
        // TODO: how to clear
        // default: clear or not clear
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // TODO: encode + sort
        for (const auto& pass : cache_.passes)
        {


            // check vertex layout

            // print stats

            

            // set texture

            // set shader
            auto shader = pool_[pass.shd_id];
            glUseProgram(shader.gid);
            glcheck_errors();

            // set pipeline
            auto pipe = pool_[pass.pipe_id];

            // set frame
            // TODO: device::device
            auto frm = pool_[pass.frm_id];
            auto fbo = frm.fbo;
            if (fbo != ctx_.framebuffer)
            {
                ctx_.framebuffer = fbo;
                // glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&gl_orig_fb);
                glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            }

            // draw
            if (pass.index.id != 0)
            {
                // set index buffer
                auto index_buf = pool_[pass.index];
                glBindBuffer(index_buf.type, index_buf.gid);
                // TODO
                glDrawElements(pipe.primitive_type, index_buf.size, GL_UNSIGNED_INT, 0);
            }
            else
            {
                auto vertex_buf_id = pass.buffers[0];
                glDrawArrays(pipe.primitive_type, 0, pool_[vertex_buf_id].size);
            }
            glcheck_errors();
        }
    }
    */

private:

    // create shader
    GLuint compile_shader(const std::string shader_name, GLenum type, const char* src)
    {
        auto id = glCreateShader(type);
        glShaderSource(id, 1, &src, NULL);
        glCompileShader(id);

        // check compile errors
        check_shader_errors(shader_name, id, type);
            
        return id;
    }
    GLuint link_shader(const std::string shader_name, std::vector<GLuint> ids)
    {
        GLuint program_id = glCreateProgram();

        for(auto id : ids)
            glAttachShader(program_id, id);

        glLinkProgram(program_id);

        // check link errors
        check_shader_errors(shader_name, program_id /*, 0*/);

        for (auto id : ids)
            glDeleteShader(id);

        return program_id;
    }
    void check_shader_errors(const std::string shader_name, GLuint id, GLenum type = 0)
    {
        static std::unordered_map<GLenum, const char*> name
        {
            { GL_VERTEX_SHADER,   "VERTEX_SHADER"   },
            { GL_GEOMETRY_SHADER, "GEOMETRY_SHADER" },
            { GL_FRAGMENT_SHADER, "FRAGMENT_SHADER" },
        };

        GLint state;
        constexpr GLsizei max_length = 1024;
        GLchar error_info[max_length];

        if (type != 0)
        {
            glGetShaderiv(id, GL_COMPILE_STATUS, &state);
            if (state == GL_FALSE)
            {
                glGetShaderInfoLog(id, max_length, NULL, error_info);
                LOG(ERROR) 
                    << "ERROR::SHADER_COMPILATION_ERROR" << "\n"
                    << "name: " << shader_name << " type: " << name[type] << "error: \n"
                    << error_info;
            }
        }
        else
        {
            glGetProgramiv(id, GL_LINK_STATUS, &state);
            if (state == GL_FALSE)
            {
                glGetProgramInfoLog(id, max_length, NULL, error_info);
                LOG(ERROR) 
                    << "ERROR::PROGRAM_LINKING_ERROR" << "\n"
                    << "name: " << shader_name << " type: " << "PROGRAM" << "error: \n"
                    << error_info;
            }
        }
    }

    // vertex attribure



    // execute commands
    void execute(const command& cmd)
    {

    }

private:
	struct context
	{
        GLuint vao{}; // todo: opengl3.3+ must have a default VAO
	};

    struct command_list_context
    {
        buffer_id index_id{};
        std::vector<buffer_id> buf_ids{};
        std::vector<texture_id> tex_ids{};
        //shader_id shd_id;
        //pipeline_id pipe_id{};
        //frame_id frm_id{};

        shader shd{};
        pipeline pipe{};

        bool is_offscreen{};
        frame frm{};
    };

    // TODO: multiwindow: one device + multictx   or   multidevice
    // one ctx bind to one window(multiwindow needs multicontext, multi-vao)
    /*const*/ GLFWwindow* window_{};

    std::stringstream log0_;
    command_list_context ctx_{};

    context context_{};
    resource_pool<buffer, texture, shader, pipeline, frame> pool_{};
};

// TODO: _ptr -> _up, _uptr _sp _wp
render_backend_ptr create_backend_opengl(const render_desc& desc)
{
    return std::make_unique<backend_opengl>(desc);
}

// class device_opengl_dsa : public render_device

// reference:
// https://github.com/acdemiralp/gl

} // namespace fay