// #include "fay/core/fay.h"
#include "fay/render/backend.h"

#ifdef FAY_IN_WINDOWS

// remove
#include "glad/glad.h"

#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#include <comdef.h>
#include <d3d11.h>
//#include <d3d11shader.h>
#include <d3dcompiler.h>
#include <dxgi.h>
//#include <d3d11_4.h>
//#include <dxgi1_6.h>
#include <windows.h>
#include <windowsx.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
//#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")
//#pragma comment(lib, "winmm.lib")


using namespace std::string_literals;

#define FAY_SMART_COM_PTR(_a) _COM_SMARTPTR_TYPEDEF(_a, __uuidof(_a))

#ifdef FAY_DEBUG
    #ifndef D3D_CHECK
    #define D3D_CHECK(func)	\
	    { \
		    HRESULT hr = (func); \
		    if(FAILED(hr)) \
		    { \
                char hr_msg[512]; \
                FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, hr, 0, hr_msg, ARRAYSIZE(hr_msg), nullptr); \
                std::string error_msg = "\nError!\nline: "s + std::to_string(__LINE__) + "\nhr: " + std::to_string(hr) + "\nfunc: " + #func + "\nerror msg: " + hr_msg; \
                LOG(ERROR) << error_msg; \
		    } \
	    }
    #endif
#else
    #ifndef D3D_CHECK
    #define D3D_CHECK(func) (func)
    #endif 
#endif

#ifdef FAY_DEBUG
#ifndef D3D_CHECK2
#define D3D_CHECK2(func, ptr) \
	    { \
		    HRESULT hr = (func); \
		    if(FAILED(hr) || !ptr) \
		    { \
                char hr_msg[512]; \
                FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, hr, 0, hr_msg, ARRAYSIZE(hr_msg), nullptr); \
                std::string error_msg = "\nError!\nline: "s + std::to_string(__LINE__) + "\nhr: " + std::to_string(hr) + "\nfunc: " + #func + "\nerror msg: " + hr_msg; \
                LOG(ERROR) << error_msg; \
		    } \
	    }
#endif
#else
#ifndef D3D_CHECK2
#define D3D_CHECK2(func) (func)
#endif 
#endif

namespace fay // TODO fay::d3d11
{

// D3D

FAY_SMART_COM_PTR(IUnknown);

FAY_SMART_COM_PTR(ID3D11Device);
FAY_SMART_COM_PTR(ID3D11DeviceContext);
FAY_SMART_COM_PTR(IDXGISwapChain);

FAY_SMART_COM_PTR(ID3D11Texture2D);
FAY_SMART_COM_PTR(ID3D11Texture2D);
FAY_SMART_COM_PTR(ID3D11RenderTargetView);
FAY_SMART_COM_PTR(ID3D11DepthStencilView);

namespace backend_d3d11_func
{

D3D_FEATURE_LEVEL d3d_feature_level(uint32_t major, uint32_t minor)
{
    switch (major)
    {
        case 12:
            switch (minor)
            {
                case 0: return D3D_FEATURE_LEVEL_12_0;
                case 1: return D3D_FEATURE_LEVEL_12_1;
            }
        case 11:
            switch (minor)
            {
                case 0: return D3D_FEATURE_LEVEL_11_0;
                case 1: return D3D_FEATURE_LEVEL_11_1;
            }
        case 10:
            switch (minor)
            {
                case 0: return D3D_FEATURE_LEVEL_10_0;
                case 1: return D3D_FEATURE_LEVEL_10_1;
            }
        case  9:
            switch (minor)
            {
                case 1: return D3D_FEATURE_LEVEL_9_1;
                case 2: return D3D_FEATURE_LEVEL_9_2;
                case 3: return D3D_FEATURE_LEVEL_9_3;
            }
        default:
            LOG(ERROR);
            return (D3D_FEATURE_LEVEL)0;
    }
}

}
using namespace backend_d3d11_func;

namespace backend_d3d11_type
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
};

struct buffer // : public buffer_desc
{
    // init by buffer_desc
    std::string name{};
    uint    size{};
    // const void* data{};
    GLenum      type{};
    GLenum      usage{};

    // used for vertex buffer, instance buffer
    GLsizei     stride{};
    std::vector<vertex_attribute_gl> layout{};

    // then assign others
    GLuint gid{}; // union { GLuint vbo; GLuint ibo; };
    //resource_state state; // TODO: remove it???

    buffer() = default;
    buffer(const buffer_desc& desc) : layout(desc.layout.size())
    {
        name = desc.name;
        size = desc.size;
        stride = desc.stride;
        type = buffer_type_map.at(desc.type).d3d11;
        usage = resource_usage_map.at(desc.usage).d3d11;

        if (desc.type == buffer_type::vertex || desc.type == buffer_type::instance)
        {

            uint offset = 0;
            for (uint i = 0; i < desc.layout.size(); ++i)
            {
                auto& da = desc.layout[i];
                auto& a = layout[i];

                auto[num, byte] = attribute_format_map.at(da.format());
                a.size = num;

                a.offset = (GLvoid*)offset;
                offset += byte;
            }
        }
    }
};

struct texture
{
    // # init by texture_desc
    std::string name{};

    uint width{};
    uint height{};
    uint layer{}; // depth

    GLenum type;
    GLenum usage;

    // remove
    GLenum internal_format, external_format, external_type;

    GLenum min_filter;
    GLenum max_filter;
    uint max_anisotropy;

    GLenum wrap_u;
    GLenum wrap_v;
    GLenum wrap_w;

    float min_lod;
    float max_lod;


    GLenum render_target;
    uint rt_sample_count;

    // # then assign others
    GLuint tbo{};
    // if use MSAA
    GLuint msaa_rbo{};

    // if used as depth_stencil target or msaa color target
    //GLuint ds_rbo{};

    texture() = default;
    texture(const texture_desc& desc)
    {
        name = desc.name;

        width = desc.width;
        height = desc.height;
        layer = desc.depth;

        type = texture_type_map.at(desc.type).d3d11;
        usage = resource_usage_map.at(desc.usage).d3d11;

        min_filter = filter_mode_map.at(desc.min_filter).d3d11;
        max_filter = filter_mode_map.at(desc.max_filter).d3d11;
        wrap_u = wrap_mode_map.at(desc.wrap_u).d3d11;
        wrap_v = wrap_mode_map.at(desc.wrap_v).d3d11;
        wrap_w = wrap_mode_map.at(desc.wrap_w).d3d11;

        render_target = render_target_map.at(desc.as_render_target).d3d11;
        rt_sample_count = desc.rt_sample_count;
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
        uint size{}; // byte_size
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
        primitive_type = primitive_type_map.at(desc.primitive_type).d3d11;

        // rasterization state
        alpha_to_coverage_enabled = desc.alpha_to_coverage_enabled;
        cull_mode = cull_mode_map.at(desc.cull_mode).d3d11;
        face_winding = (desc.face_winding == face_winding::cw) ? GL_CW : GL_CCW;
        rasteriza_sample_count = desc.rasteriza_sample_count;
        depth_bias = desc.depth_bias;
        depth_bias_slope_scale = desc.depth_bias_slope_scale;
        depth_bias_clamp = desc.depth_bias_clamp;

        // depth-stencil state
        depth_enabled = desc.depth_enabled;
        depth_compare_op = compare_op_map.at(desc.depth_compare_op).d3d11;

        auto copy_stencil_state = [](pipeline::stencil_state dst, const pipeline_desc::stencil_state src)
        {
            dst.fail_op = stencil_op_map.at(src.fail_op).d3d11;
            dst.depth_fail_op = stencil_op_map.at(src.depth_fail_op).d3d11;
            dst.pass_op = stencil_op_map.at(src.pass_op).d3d11;
            dst.compare_op = compare_op_map.at(src.compare_op).d3d11;
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
            dst.src_factor = blend_factor_map.at(src.src_factor).d3d11;
            dst.dst_factor = blend_factor_map.at(src.dst_factor).d3d11;
            dst.blend_op = blend_op_map.at(src.blend_op).d3d11;
        };

        blend_enabled = desc.blend_enabled;

        copy_blend_state(blend_rgb, desc.blend_rgb);
        copy_blend_state(blend_alpha, desc.blend_alpha);

        color_attachment_count = desc.color_attachment_count;
        //blend_color_format = pixel_external_format(desc.blend_color_format);
        //blend_depth_format = pixel_external_format(desc.blend_depth_format);
        blend_color = desc.blend_color;
        blend_write_mask = desc.blend_write_mask;
    }
};

struct attachment
{
    texture_id tex_pid{};
    uint layer;
    uint level;
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

    uint width{};
    uint height{};

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
using namespace backend_d3d11_type;



class backend_d3d11 : public render_backend
{
public:
    backend_d3d11(const render_desc& desc)
        : render_backend(desc)
    {
        create_device();
        create_render_target();
    }
    ~backend_d3d11()
    {
    }

    buffer_id   create(const   buffer_desc& desc) override
    {
        // or id = buffer_pool.insert(besc), generate buffer by itself ???
        buffer_id pid = pool_.insert(desc); // id in the pool
        buffer& buf = pool_[pid];

        return pid;
    }
    texture_id  create(const  texture_desc& desc) override
    {
        texture_id pid = pool_.insert(desc); // id in the pool
        texture& tex = pool_[pid];


        return pid;
    }
    shader_id   create(const   shader_desc& desc) override
    {
        shader_id pid = pool_.insert(desc); // id in the pool
        shader& shader = pool_[pid];

        return pid;
    }
    pipeline_id create(const pipeline_desc& desc) override
    {
        pipeline_id pid = pool_.insert(desc); // id in the pool
        //pipeline& pipe = pool_[pid];


        return pid;
    }
    frame_id    create(const    frame_desc& desc) override
    {
        frame_id pid = pool_.insert(desc); // id in the pool
        frame& frm = pool_[pid];

        return pid;
    }

    void destroy(buffer_id id) override
    {
        auto gid = pool_[id].gid;
        // https://stackoverflow.com/questions/27937285/when-should-i-call-gldeletebuffers
        pool_.erase(id);
    }
    void destroy(texture_id id) override
    {
        auto gid = pool_[id].tbo;
        pool_.erase(id);
    }
    void destroy(shader_id id) override
    {
        auto gid = pool_[id].gid;
        // glDeleteProgram(gid);
        pool_.erase(id);
    }
    void destroy(pipeline_id id) override
    {
        pool_.erase(id);
    }
    void destroy(frame_id id) override
    {
        pool_.erase(id);
    }

    // render
    void begin_frame(frame_id id) override
    {

        log_ << ("frame    : "s + (cmd_.is_offscreen ? cmd_.frm.name : "0") + '\n');
    }
    void end_frame() override
    {

        // log info
    #ifdef FAY_DEBUG_
        std::cout << "\nrender backend: d3d11\n\n";
        std::cout << log_.rdbuf();
        std::cout << '\n';
    #endif // FAY_DEBUG

        log_.clear();
    }

    void clear_color(glm::vec4 rgba, std::vector<uint> targets) const override
    {
    }
    void clear_depth(float depth) const override
    {
    }
    void clear_stencil(uint stencil) const override
    {
    }

    void set_viewport(uint x, uint y, uint width, uint height) override
    {
    }
    void set_scissor(uint x, uint y, uint width, uint height) override
    {
    }

    void apply_pipeline(const pipeline_id id, std::array<bool, 4> flags) override
    {
        log_ << ("pipeline : "s + cmd_.pipe.name + '\n');
    }
    void apply_shader(const shader_id id) override
    {
        log_ << ("shader   : "s + cmd_.shd.name + "\n");
    }


    // TODO: rename uniform_block
    void bind_uniform(uint ub_index, const void* data, uint size) override
    {
        log_ << ("uniform block : "s + "TODO" + '\n');
    }

    void bind_uniform(const char* name, command::uniform uniform) override
    {
        log_ << ("uniform  : "s + std::to_string(uniform.index()) + '\n');
    }

    void bind_index(const buffer_id id) override
    {
        //log_ << ("index    : "s + (cmd_.index_id.value == 0 ? "null" : pool_[cmd_.index_id].name) + '\n');
    }
    void bind_vertex(const buffer_id id, std::vector<size_t> attrs, std::vector<size_t> slots, size_t instance_rate) override
    {
        log_ << ("buffer   : "s + pool_[id].name + '\n');
    }
    void bind_texture(const texture_id id, int tex_unit, const std::string& sampler) override
    {
        log_ << ("texture  : "s + pool_[id].name + '\n');
    }

    void update(buffer_id id, const void* data, int size) override
    {
    }
    void update(texture_id id, const void* data) override
    {
    }



    void draw(uint count, uint first, uint instance_count) override
    {
        log_ << "draw\n\n";
    }
    void draw_index(uint count, uint first, uint instance_count) override
    {
        log_ << "draw_index\n\n";
    }

private:
    bool create_device()
    {
        if (renderd_.enable_msaa)
        {
            ctx_.sample_desc.Count = renderd_.multiple_sample_count;
            ctx_.sample_desc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
        }
        else
        {
            ctx_.sample_desc.Count = 1;
            ctx_.sample_desc.Quality = 0;
        }

        UINT create_flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
    #ifdef FAY_DEBUG
        create_flags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif
        DXGI_SWAP_CHAIN_DESC scd;
        {
            ZeroMemory(&scd, sizeof(scd));
            scd.BufferDesc.Width = renderd_.width;
            scd.BufferDesc.Height = renderd_.height;
            scd.BufferDesc.RefreshRate.Numerator = 60;
            scd.BufferDesc.RefreshRate.Denominator = 1;
            scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
            scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
            scd.SampleDesc = ctx_.sample_desc;
            scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            scd.BufferCount = 1;
            scd.OutputWindow = static_cast<HWND>(renderd_.d3d_handle);
            scd.Windowed = TRUE;
            scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
            scd.Flags = 0;
        }
        D3D_FEATURE_LEVEL feature_level;
        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr,                       // pAdapter (use default)
            D3D_DRIVER_TYPE_HARDWARE,   // DriverType
            nullptr,                       // Software
            create_flags,               // Flags
            nullptr,                       // pFeatureLevels
            0,                          // FeatureLevels
            D3D11_SDK_VERSION,          // SDKVersion
            &scd,                       // pSwapChainDesc
            &ctx_.swapchain,            // ppSwapChain
            &ctx_.device,               // ppDevice
            &feature_level,             // pFeatureLevel
            &ctx_.context               // ppImmediateContext
        );
        DCHECK(SUCCEEDED(hr) && ctx_.swapchain && ctx_.device && ctx_.context);

        return true;
    }

    bool create_render_target()
    {
        D3D_CHECK(ctx_.swapchain->ResizeBuffers(1, renderd_.width, renderd_.height, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
        D3D_CHECK2(ctx_.swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&ctx_.rtt),      ctx_.rtt);
        D3D_CHECK2(ctx_.device->CreateRenderTargetView((ID3D11Resource*)ctx_.rtt, nullptr, &ctx_.rtv), ctx_.rtv);

        D3D11_TEXTURE2D_DESC ds_desc;
        {
            ds_desc.Width = renderd_.width;
            ds_desc.Height = renderd_.height;
            ds_desc.MipLevels = 1;
            ds_desc.ArraySize = 1;
            ds_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            ds_desc.SampleDesc = ctx_.sample_desc;
            ds_desc.Usage = D3D11_USAGE_DEFAULT;
            ds_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            ds_desc.CPUAccessFlags = 0;
            ds_desc.MiscFlags = 0;
        }
        D3D_CHECK2(ctx_.device->CreateTexture2D(&ds_desc, nullptr, &ctx_.dst), ctx_.dst);

        D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
        {
            dsv_desc.Format = ds_desc.Format;
            dsv_desc.ViewDimension = ctx_.sample_desc.Count > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
        }
        D3D_CHECK2(ctx_.device->CreateDepthStencilView((ID3D11Resource*)ctx_.dst, nullptr, &ctx_.dsv), ctx_.dsv);

        ctx_.context->OMSetRenderTargets(1, &ctx_.rtv, ctx_.dsv);

        // TODO
        ctx_.viewport.TopLeftX = 0;
        ctx_.viewport.TopLeftY = 0;
        ctx_.viewport.Width = static_cast<float>(renderd_.width);
        ctx_.viewport.Height = static_cast<float>(renderd_.height);
        ctx_.viewport.MinDepth = 0.0f;
        ctx_.viewport.MaxDepth = 1.0f;
        ctx_.context->RSSetViewports(1, &ctx_.viewport);

        return true;
    }

private:
    struct context
    {
        DXGI_SAMPLE_DESC sample_desc;

        ID3D11DevicePtr device;
        ID3D11DeviceContextPtr context;
        IDXGISwapChainPtr swapchain;

        ID3D11Texture2DPtr rtt;
        ID3D11Texture2DPtr dst;
        ID3D11RenderTargetViewPtr rtv;
        ID3D11DepthStencilViewPtr dsv;
        D3D11_VIEWPORT viewport;
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

    std::stringstream log_;
    command_list_context cmd_{};

    context ctx_{};
    resource_pool<buffer, texture, shader, pipeline, frame> pool_{};
};

// TODO: _ptr -> _up, _uptr _sp _wp
render_backend_ptr create_backend_d3d11(const render_desc& desc)
{
    return std::make_unique<backend_d3d11>(desc);
}

} // namespace fay

#endif
