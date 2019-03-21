// #include "fay/core/fay.h"
#include "fay/render/backend.h"

#ifdef FAY_IN_WINDOWS

#define CINTERFACE
#define COBJMACROS
#define D3D11_NO_HELPERS
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
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
//#pragma comment(lib, "dxguid.lib")
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
    #ifndef D3D_CHECK2
    #define D3D_CHECK2(func, ptr) \
            D3D_CHECK(func) \
            { if(!ptr) LOG(ERROR) << "nullptr: "s + #ptr; }
    #endif
#else
    #ifndef D3D_CHECK
    #define D3D_CHECK(func) (func)
    #endif 
    #ifndef D3D_CHECK2
    #define D3D_CHECK2(func) (func)
    #endif 
#endif

namespace fay // TODO fay::d3d11
{

// D3D

FAY_SMART_COM_PTR(IUnknown);
FAY_SMART_COM_PTR(ID3DBlob);

FAY_SMART_COM_PTR(ID3D11Device);
FAY_SMART_COM_PTR(ID3D11DeviceContext);
FAY_SMART_COM_PTR(IDXGISwapChain);

FAY_SMART_COM_PTR(ID3D11Buffer);
FAY_SMART_COM_PTR(ID3D11Texture2D);
FAY_SMART_COM_PTR(ID3D11Texture3D);
FAY_SMART_COM_PTR(ID3D11SamplerState);
FAY_SMART_COM_PTR(ID3D11ShaderResourceView);
FAY_SMART_COM_PTR(ID3D11RenderTargetView);
FAY_SMART_COM_PTR(ID3D11DepthStencilView);

FAY_SMART_COM_PTR(ID3D11VertexShader);
FAY_SMART_COM_PTR(ID3D11PixelShader);

FAY_SMART_COM_PTR(ID3D11InputLayout);
FAY_SMART_COM_PTR(ID3D11RasterizerState);
FAY_SMART_COM_PTR(ID3D11DepthStencilState);
FAY_SMART_COM_PTR(ID3D11BlendState);

namespace backend_d3d11_func
{

D3D_FEATURE_LEVEL feature_level(uint32_t major, uint32_t minor)
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
            return D3D_FEATURE_LEVEL(0);
    }
}

D3D11_CPU_ACCESS_FLAG cpu_access_flag(D3D11_USAGE usage)
{
    switch(usage)
    {
        case D3D11_USAGE_IMMUTABLE:
            return D3D11_CPU_ACCESS_FLAG(0);
        case D3D11_USAGE_DYNAMIC:
        case D3D11_USAGE_STAGING:
            return D3D11_CPU_ACCESS_WRITE;
        default:
            LOG(ERROR);
            return D3D11_CPU_ACCESS_FLAG(0);
    }
}

UINT8 color_write_mask(blend_mask mask) 
{
    UINT8 res = 0;

    if (enum_have(mask, blend_mask::red))   res |= D3D11_COLOR_WRITE_ENABLE_RED;
    if (enum_have(mask, blend_mask::green)) res |= D3D11_COLOR_WRITE_ENABLE_GREEN;
    if (enum_have(mask, blend_mask::blue))  res |= D3D11_COLOR_WRITE_ENABLE_BLUE;
    if (enum_have(mask, blend_mask::alpha)) res |= D3D11_COLOR_WRITE_ENABLE_ALPHA;

    return res;
}

}
using namespace backend_d3d11_func;

namespace backend_d3d11_type
{

const inline enum_class_map<attribute_usage, const char*>
semantic_name_map
{
    { attribute_usage::position,       "POSITION" },
    { attribute_usage::normal,         "NORMAL" },
    { attribute_usage::texcoord0,      "TEXCOORD0" },
    { attribute_usage::tangent,        "TANGENT" },
    { attribute_usage::bitangent,      "BITANGENT" },
    { attribute_usage::instance_model, "INSTANCE_MODEL" },
};

const inline enum_class_map<attribute_format, DXGI_FORMAT>
vertex_format_map
{
    { attribute_format::float1, DXGI_FORMAT_R32_FLOAT },
    { attribute_format::float2, DXGI_FORMAT_R32G32_FLOAT },
    { attribute_format::float3, DXGI_FORMAT_R32G32B32_FLOAT },
    { attribute_format::float4, DXGI_FORMAT_R32G32B32A32_FLOAT },

    { attribute_format::short2, DXGI_FORMAT_R16G16_SINT },
    { attribute_format::short4, DXGI_FORMAT_R16G16B16A16_SINT },

    { attribute_format::byte4,  DXGI_FORMAT_R8G8B8A8_SINT },
    { attribute_format::ubyte4, DXGI_FORMAT_R8G8B8A8_UINT },
};

struct buffer // : public buffer_desc
{
    // init by buffer_desc
    std::string name{};
    uint        size{};
    // const void* data{};

    UINT        type{};
    D3D11_USAGE usage{};

    // used for vertex buffer, instance buffer
    /*
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },

        { "World", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        { "World", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        { "World", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        { "World", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1},
    */
    UINT        stride{};
    std::vector<D3D11_INPUT_ELEMENT_DESC> layout{};

    // append & update
    uint append_pos{};
    bool append_overflow{};
    uint append_frame_index{};
    uint update_frame_index{};

    // then assign others
    ID3D11BufferPtr d3d11_buf{};
    ID3D11InputLayoutPtr il;

    buffer() = default;
    buffer(const buffer_desc& desc) : layout(desc.layout.size())
    {
        name = desc.name;
        size = desc.size;
        type = buffer_type_map.at(desc.type).d3d11;
        usage = static_cast<D3D11_USAGE>(resource_usage_map.at(desc.usage).d3d11);

        if (desc.type == buffer_type::vertex || desc.type == buffer_type::instance)
        {
            uint offset = 0;

            for (uint i = 0; i < desc.layout.size(); ++i)
            {
                auto& da = desc.layout[i];
                auto& a = layout[i];

                a.SemanticName = semantic_name_map.at(da.usage());
                a.SemanticIndex = da.index();
                a.Format = vertex_format_map.at(da.format());

                // a.InputSlot = 0;
                a.AlignedByteOffset = offset; 
                    offset += attribute_format_map.at(da.format()).size;

                // a.InputSlotClass = desc.type == buffer_type::vertex ? D3D11_INPUT_PER_VERTEX_DATA : D3D11_INPUT_PER_INSTANCE_DATA;
                // a.InstanceDataStepRate = 0;

                if(desc.type == buffer_type::vertex)
                {
                    a.InputSlot = 0;
                    a.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
                    a.InstanceDataStepRate = 0;
                }
                else
                {
                    a.InputSlot = 1;
                    a.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
                    a.InstanceDataStepRate = 1;
                }
            }

            stride = offset;
        }
    }
};

struct texture
{
    // # init by texture_desc
    std::string name{};

    UINT width{};
    UINT height{};
    UINT depth{}; // layer
    UINT mipmaps{};
    DXGI_FORMAT format;

    texture_type type;
    D3D11_USAGE usage;

    D3D11_FILTER min_filter;
    D3D11_FILTER max_filter;
    UINT max_anisotropy;

    D3D11_TEXTURE_ADDRESS_MODE wrap_u;
    D3D11_TEXTURE_ADDRESS_MODE wrap_v;
    D3D11_TEXTURE_ADDRESS_MODE wrap_w;

    float min_lod;
    float max_lod;

    UINT render_target;
    uint rt_sample_count; // msaa

    // # then assign others
    uint update_frame_index;

    ID3D11Texture2DPtr d3d11_tex2d;
    ID3D11Texture3DPtr d3d11_tex3d;
    ID3D11Texture2DPtr d3d11_texds;
    ID3D11Texture2DPtr d3d11_texmsaa;
    ID3D11ShaderResourceViewPtr d3d11_srv;
    ID3D11SamplerStatePtr d3d11_smp;

    texture() = default;
    texture(const texture_desc& desc)
    {
        name = desc.name;

        width = desc.width;
        height = desc.height;
        depth = desc.depth;
        mipmaps = desc.mipmaps;
        format = static_cast<DXGI_FORMAT>(pixel_format_map.at(desc.format).d3d11);

        type = desc.type;
        usage = static_cast<D3D11_USAGE>(resource_usage_map.at(desc.usage).d3d11);

        min_filter = static_cast<D3D11_FILTER>(filter_mode_map.at(desc.min_filter).d3d11);
        max_filter = static_cast<D3D11_FILTER>(filter_mode_map.at(desc.max_filter).d3d11);
        max_anisotropy = desc.max_anisotropy;
        wrap_u = static_cast<D3D11_TEXTURE_ADDRESS_MODE>(wrap_mode_map.at(desc.wrap_u).d3d11);
        wrap_v = static_cast<D3D11_TEXTURE_ADDRESS_MODE>(wrap_mode_map.at(desc.wrap_v).d3d11);
        wrap_w = static_cast<D3D11_TEXTURE_ADDRESS_MODE>(wrap_mode_map.at(desc.wrap_w).d3d11);

        render_target = render_target_map.at(desc.as_render_target).d3d11;
        rt_sample_count = desc.rt_sample_count;
    }
};

struct shader
{
    /*
    struct uniform_block
    {
        UINT sz{};
        ID3D11BufferPtr d3d11_cbs{};
    };
    */

    // init by shader_desc
    std::string name{};
    uint vs_uniform_block_sz{};
    uint fs_uniform_block_sz{};
    uint vs_samplers_sz{};
    uint fs_samplers_sz{};

    // TODO: memory_view
    const uint8_t* vs_bytecode{};
    const uint8_t* ps_bytecode{};

    // then assign others

    // constant buffers
    std::vector<UINT> d3d11_cb_szs;
    std::vector<ID3D11BufferPtr> d3d11_cbs;

    // shader
    void* vs_blob{};
    uint vs_blob_length{};
    ID3D11VertexShaderPtr d3d11_vs{};
    ID3D11PixelShaderPtr  d3d11_ps{};

    shader() = default;
    shader(const shader_desc& desc)
    {
        name = desc.name;

        vs_uniform_block_sz = desc.vs_uniform_block_sz;
        fs_uniform_block_sz = desc.fs_uniform_block_sz;

        vs_samplers_sz = desc.vs_samplers_sz;
        fs_samplers_sz = desc.fs_samplers_sz;
    }
};

struct pipeline
{
    // init by pipeline_desc
    std::string name{};

    //
    D3D_PRIMITIVE_TOPOLOGY primitive_type{};

    //
    bool alpha_to_coverage_enabled{ false };
    D3D11_CULL_MODE cull_mode{};
    UINT face_winding{};
    int rasteriza_sample_count{ 0 };
    float depth_bias{ 0.f };
    float depth_bias_slope_scale{ 0.f };
    float depth_bias_clamp{ 0.f };

    // depth-stencil state
    struct stencil_state
    {
        D3D11_STENCIL_OP fail_op{};
        D3D11_STENCIL_OP depth_fail_op{};
        D3D11_STENCIL_OP pass_op{};
        D3D11_COMPARISON_FUNC compare_op{};
    };

    bool depth_enabled{ true };
    D3D11_COMPARISON_FUNC depth_compare_op{};

    bool stencil_enabled{ true };
    stencil_state stencil_front{};
    stencil_state stencil_back{};
    uint8_t stencil_test_mask{ 0 };
    uint8_t stencil_write_mask{ 0 };
    uint8_t stencil_ref{ 0 };

    //
    struct blend_state
    {
        D3D11_BLEND src_factor{};
        D3D11_BLEND dst_factor{};
        D3D11_BLEND_OP blend_op{};
    };

    bool blend_enabled{ false };

    blend_state blend_rgb;
    blend_state blend_alpha;

    UINT8 blend_write_mask{};
    UINT color_attachment_count{ 1 };
    UINT blend_color_format{};
    UINT blend_depth_format{};
    std::array<float, 4> blend_color{ 0.f, 0.f, 0.f, 0.f };

    ID3D11InputLayoutPtr d3d11_il{};
    ID3D11RasterizerStatePtr d3d11_rs{};
    ID3D11DepthStencilStatePtr d3d11_ds{};
    ID3D11BlendStatePtr d3d11_bs{};

    // then assign others

    pipeline() = default;
    pipeline(const pipeline_desc& desc)
    {
        name = desc.name;

        // primitive type
        primitive_type = static_cast<D3D_PRIMITIVE_TOPOLOGY>(primitive_type_map.at(desc.primitive_type).d3d11);

        // rasterization state
        alpha_to_coverage_enabled = desc.alpha_to_coverage_enabled;
        cull_mode              = static_cast<D3D11_CULL_MODE>(cull_mode_map.at(desc.cull_mode).d3d11);
        face_winding           = (desc.face_winding == face_winding::cw) ? GL_CW : GL_CCW;
        rasteriza_sample_count = desc.rasteriza_sample_count;
        depth_bias             = desc.depth_bias;
        depth_bias_slope_scale = desc.depth_bias_slope_scale;
        depth_bias_clamp       = desc.depth_bias_clamp;

        // depth-stencil state
        depth_enabled = desc.depth_enabled;
        depth_compare_op = static_cast<D3D11_COMPARISON_FUNC>(compare_op_map.at(desc.depth_compare_op).d3d11);

        auto copy_stencil_state = [](pipeline::stencil_state& dst, const pipeline_desc::stencil_state src)
        {
            dst.fail_op       = static_cast<D3D11_STENCIL_OP>(stencil_op_map.at(src.fail_op).d3d11);
            dst.depth_fail_op = static_cast<D3D11_STENCIL_OP>(stencil_op_map.at(src.depth_fail_op).d3d11);
            dst.pass_op       = static_cast<D3D11_STENCIL_OP>(stencil_op_map.at(src.pass_op).d3d11);
            dst.compare_op    = static_cast<D3D11_COMPARISON_FUNC>(compare_op_map.at(src.compare_op).d3d11);
        };

        stencil_enabled = desc.stencil_enabled;

        copy_stencil_state(stencil_front, desc.stencil_front);
        copy_stencil_state(stencil_back, desc.stencil_back);

        stencil_test_mask  = desc.stencil_test_mask;
        stencil_write_mask = desc.stencil_write_mask;
        stencil_ref        = desc.stencil_ref;

        // alpha-blending state
        auto copy_blend_state = [](pipeline::blend_state& dst, const pipeline_desc::blend_state src)
        {
            dst.src_factor = static_cast<D3D11_BLEND>(blend_factor_map.at(src.src_factor).d3d11);
            dst.dst_factor = static_cast<D3D11_BLEND>(blend_factor_map.at(src.dst_factor).d3d11);
            dst.blend_op   = static_cast<D3D11_BLEND_OP>(blend_op_map.at(src.blend_op).d3d11);
        };

        blend_enabled = desc.blend_enabled;

        copy_blend_state(blend_rgb, desc.blend_rgb);
        copy_blend_state(blend_alpha, desc.blend_alpha);

        color_attachment_count = desc.color_attachment_count;
        //blend_color_format = pixel_external_format(desc.blend_color_format);
        //blend_depth_format = pixel_external_format(desc.blend_depth_format);
        blend_color = desc.blend_color;
        blend_write_mask = color_write_mask(desc.blend_write_mask);
    }
};

struct attachment
{
    texture_id tex_pid{};
    uint layer; // rename: depth
    uint level; // rename: mip_level

    attachment() {}
    attachment(const attachment_desc& desc)
    {
        tex_pid = desc.tex_id;
        layer = desc.layer;
        level = desc.level;
    }
};

struct frame
{
    // # init by frame_desc
    std::string name{};

    uint width{};
    uint height{};

    std::vector<attachment> render_targets{};
    attachment depth_stencil{};

    // # then assign others
    std::vector <ID3D11RenderTargetViewPtr> d3d11_rtvs;
    ID3D11DepthStencilViewPtr d3d11_dsv;

    frame() = default;
    frame(const frame_desc& desc)
    {
        name = desc.name;

        width = desc.width;
        height = desc.height;

        for (const auto& ad : desc.render_targets)
            render_targets.emplace_back(ad);
        depth_stencil = desc.depth_stencil;

        d3d11_rtvs.resize(desc.render_targets.size());
    }
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
        resize_render_target();
    }
    ~backend_d3d11()
    {
    }

    buffer_id   create(const   buffer_desc& desc) override
    {
        // or id = buffer_pool.insert(besc), generate buffer by itself ???
        buffer_id pid = pool_.insert(desc); // id in the pool
        buffer& buf = pool_[pid];

        D3D11_BUFFER_DESC d3d11_desc;
        memset(&d3d11_desc, 0, sizeof(d3d11_desc));
        d3d11_desc.ByteWidth = buf.size;
        d3d11_desc.Usage = static_cast<D3D11_USAGE>(buf.usage);
        d3d11_desc.BindFlags = buf.type;
        d3d11_desc.CPUAccessFlags = cpu_access_flag(buf.usage);

        D3D11_SUBRESOURCE_DATA* init_data_ptr{};
        D3D11_SUBRESOURCE_DATA init_data;
        memset(&init_data, 0, sizeof(init_data));
        if (desc.usage == resource_usage::immutable) 
        {
            init_data.pSysMem = desc.data;
            init_data_ptr = &init_data;
        }
        D3D_CHECK2(ctx_.device->CreateBuffer(&d3d11_desc, init_data_ptr, &buf.d3d11_buf), buf.d3d11_buf);

        return pid;
    }
    texture_id  create(const  texture_desc& desc) override
    {
        texture_id pid = pool_.insert(desc); // id in the pool
        texture& tex = pool_[pid];

        if (desc.as_render_target == render_target::color) //!is_dpeth_stencil_pixel_format(desc.format))
        {
           // prepare initial content pointers
            D3D11_SUBRESOURCE_DATA* init_data_ptr{ nullptr };
            std::vector<D3D11_SUBRESOURCE_DATA> sub_data;
            if (desc.usage == resource_usage::immutable)
            {
                sub_data = texture_subres_data(desc);
                init_data_ptr = sub_data.data();
            }

            // TODO: auto gen mipmaps
            // https://github.com/Microsoft/DirectXTK/wiki/DDSTextureLoader

            if (desc.type != texture_type::three) 
            {
                // if this is an MSAA render target, the following texture will be the 'resolve-texture'
                D3D11_TEXTURE2D_DESC d3d11_tex_desc;
                memset(&d3d11_tex_desc, 0, sizeof(d3d11_tex_desc));
                d3d11_tex_desc.Width = tex.width;
                d3d11_tex_desc.Height = tex.height;
                d3d11_tex_desc.ArraySize = tex.depth; // ???
                d3d11_tex_desc.MipLevels = tex.mipmaps;
                d3d11_tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                d3d11_tex_desc.Format = tex.format;

                if (tex.render_target) // TODO
                {
                    d3d11_tex_desc.Usage = D3D11_USAGE_DEFAULT;
                    if (tex.rt_sample_count == 1)
                        d3d11_tex_desc.BindFlags |= D3D11_BIND_RENDER_TARGET; // ???
                    d3d11_tex_desc.CPUAccessFlags = 0;
                }
                else
                {
                    d3d11_tex_desc.Usage = tex.usage;
                    d3d11_tex_desc.CPUAccessFlags = cpu_access_flag(tex.usage);
                }

                d3d11_tex_desc.SampleDesc.Count = 1;
                d3d11_tex_desc.SampleDesc.Quality = 0;
                d3d11_tex_desc.MiscFlags = (desc.type == texture_type::cube) ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;

                D3D_CHECK2(ctx_.device->CreateTexture2D(&d3d11_tex_desc, init_data_ptr, &tex.d3d11_tex2d), tex.d3d11_tex2d);

                /* also need to create a separate MSAA render target texture? */
                if (tex.rt_sample_count > 1) 
                {
                    d3d11_tex_desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
                    d3d11_tex_desc.SampleDesc.Count = tex.rt_sample_count;
                    d3d11_tex_desc.SampleDesc.Quality = (UINT)D3D11_STANDARD_MULTISAMPLE_PATTERN;
                    D3D_CHECK2(ctx_.device->CreateTexture2D(&d3d11_tex_desc, NULL, &tex.d3d11_texmsaa), tex.d3d11_texmsaa);
                }

                /* shader-resource-view */
                D3D11_SHADER_RESOURCE_VIEW_DESC d3d11_srv_desc;
                memset(&d3d11_srv_desc, 0, sizeof(d3d11_srv_desc));
                d3d11_srv_desc.Format = d3d11_tex_desc.Format;

                switch (desc.type) 
                {
                    case texture_type::two:
                        d3d11_srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                        d3d11_srv_desc.Texture2D.MipLevels = tex.mipmaps;
                        break;
                    case texture_type::cube:
                        d3d11_srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
                        d3d11_srv_desc.TextureCube.MipLevels = tex.mipmaps;
                        break;
                    case texture_type::array:
                        d3d11_srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
                        d3d11_srv_desc.Texture2DArray.MipLevels = tex.mipmaps;
                        d3d11_srv_desc.Texture2DArray.ArraySize = tex.depth;
                        break;
                    default:
                        LOG(ERROR); 
                        break;
                }
                D3D_CHECK2(ctx_.device->CreateShaderResourceView((ID3D11Resource*)tex.d3d11_tex2d, &d3d11_srv_desc, &tex.d3d11_srv), tex.d3d11_srv);
            }
            else 
            {
                D3D11_TEXTURE3D_DESC d3d11_tex_desc;
                memset(&d3d11_tex_desc, 0, sizeof(d3d11_tex_desc));
                d3d11_tex_desc.Width = tex.width;
                d3d11_tex_desc.Height = tex.height;
                d3d11_tex_desc.Depth = tex.depth;
                d3d11_tex_desc.MipLevels = tex.mipmaps;

                if (tex.render_target) 
                {
                    d3d11_tex_desc.Format = tex.format;
                    d3d11_tex_desc.Usage = D3D11_USAGE_DEFAULT;
                    d3d11_tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
                    d3d11_tex_desc.CPUAccessFlags = 0;
                }
                else 
                {
                    d3d11_tex_desc.Format = tex.format;
                    d3d11_tex_desc.Usage = tex.usage;
                    d3d11_tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                    d3d11_tex_desc.CPUAccessFlags = cpu_access_flag(tex.usage);
                }

                D3D_CHECK2(ctx_.device->CreateTexture3D(&d3d11_tex_desc, init_data_ptr, &tex.d3d11_tex3d), tex.d3d11_tex3d);

                /* shader resource view for 3d texture */
                D3D11_SHADER_RESOURCE_VIEW_DESC d3d11_srv_desc;
                memset(&d3d11_srv_desc, 0, sizeof(d3d11_srv_desc));
                d3d11_srv_desc.Format = d3d11_tex_desc.Format;
                d3d11_srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
                d3d11_srv_desc.Texture3D.MipLevels = tex.mipmaps;
                D3D_CHECK2(ctx_.device->CreateShaderResourceView((ID3D11Resource*)tex.d3d11_tex3d, &d3d11_srv_desc, &tex.d3d11_srv), tex.d3d11_srv);
            }

            /* sampler state object, note D3D11 implements an internal shared-pool for sampler objects */
            D3D11_SAMPLER_DESC d3d11_smp_desc;
            memset(&d3d11_smp_desc, 0, sizeof(d3d11_smp_desc));
            d3d11_smp_desc.Filter = tex.min_filter; // TODO
            d3d11_smp_desc.AddressU = tex.wrap_u;
            d3d11_smp_desc.AddressV = tex.wrap_v;
            d3d11_smp_desc.AddressW = tex.wrap_w;
            d3d11_smp_desc.MaxAnisotropy = tex.max_anisotropy;
            d3d11_smp_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
            d3d11_smp_desc.MinLOD = desc.min_lod;
            d3d11_smp_desc.MaxLOD = desc.max_lod;
            D3D_CHECK2(ctx_.device->CreateSamplerState(&d3d11_smp_desc, &tex.d3d11_smp), tex.d3d11_smp);
        }
        else if (enum_have(render_target::DepthStencil, desc.as_render_target))// reanme: depth_or_stencil
        {
            D3D11_TEXTURE2D_DESC d3d11_desc;
            memset(&d3d11_desc, 0, sizeof(d3d11_desc));
            d3d11_desc.Width = tex.width;
            d3d11_desc.Height = tex.height;
            d3d11_desc.MipLevels = 1;
            d3d11_desc.ArraySize = 1;
            d3d11_desc.Format = tex.format;
            d3d11_desc.SampleDesc.Count = tex.rt_sample_count;
            d3d11_desc.SampleDesc.Quality = (tex.rt_sample_count > 1) ? D3D11_STANDARD_MULTISAMPLE_PATTERN : 0;
            d3d11_desc.Usage = D3D11_USAGE_DEFAULT;
            d3d11_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            D3D_CHECK2(ctx_.device->CreateTexture2D(&d3d11_desc, NULL, &tex.d3d11_texds), tex.d3d11_texds);
        }
        else
        {
            DCHECK(false) << "shouldn't be here";
        }

        return pid;
    }
    shader_id   create(const   shader_desc& desc) override
    {
        shader_id pid = pool_.insert(desc); // id in the pool
        shader& shd = pool_[pid];

        // create constant buffer
        {
            auto roundup = [](uint val, uint round_to) -> uint
            {
                return (((val)+((round_to)-1))&~((round_to)-1));
            };

            uint ub_sz = desc.uniform_blocks.size();
            shd.d3d11_cbs.resize(ub_sz);
            shd.d3d11_cb_szs.resize(ub_sz);

            for (int ub_index = 0; ub_index < ub_sz; ub_index++)
            {
                const auto& ub_desc = desc.uniform_blocks[ub_index];

                shd.d3d11_cb_szs[ub_index] = ub_desc.size;

                D3D11_BUFFER_DESC cb_desc;
                memset(&cb_desc, 0, sizeof(cb_desc));
                cb_desc.ByteWidth = roundup(ub_desc.size, 16); // remove
                cb_desc.Usage = D3D11_USAGE_DEFAULT;
                cb_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
                D3D_CHECK2(ctx_.device->CreateBuffer(&cb_desc, NULL, &shd.d3d11_cbs[ub_index]), shd.d3d11_cbs[ub_index]);
            }
        }

        // compile shader code
        {
            const void *vs_ptr = 0, *ps_ptr = 0;
            SIZE_T vs_length = 0, ps_length = 0;
            ID3DBlob *vs_blob = 0, *ps_blob = 0;

            /*
            if (desc->vs.bytecode && desc->ps.bytecode)
            {
                // create from byte code
                vs_ptr = desc->vs.byte_code;
                ps_ptr = desc->ps.byte_code;
                vs_length = desc->vs.byte_code_size;
                ps_length = desc->ps.byte_code_size;
            }
            else
            */
            {
                vs_blob = compile_shader(desc.vs, "vs_5_0");
                ps_blob = compile_shader(desc.fs, "ps_5_0");

                if (vs_blob && ps_blob)
                {
                    vs_ptr = vs_blob->GetBufferPointer();
                    vs_length = vs_blob->GetBufferSize();
                    ps_ptr = ps_blob->GetBufferPointer();
                    ps_length = ps_blob->GetBufferSize();
                }
            }

            DCHECK(vs_ptr && ps_ptr && (vs_length > 0) && (ps_length > 0));

            D3D_CHECK2(ctx_.device->CreateVertexShader(vs_ptr, vs_length, NULL, &shd.d3d11_vs), shd.d3d11_vs);
            D3D_CHECK2(ctx_.device->CreatePixelShader(ps_ptr, ps_length, NULL, &shd.d3d11_ps), shd.d3d11_ps);

            // need to store the vertex shader byte code, this is needed later in sg_create_pipeline
            shd.vs_blob = vs_blob;
            shd.vs_blob_length = vs_length;
        }

        return pid;
    }
    pipeline_id create(const pipeline_desc& desc) override
    {
        pipeline_id pid = pool_.insert(desc);
        pipeline& pipe = pool_[pid];

        // create input layout object
        // delay to the 'bind_vertex' stage

        /* create rasterizer state */
        D3D11_RASTERIZER_DESC rs_desc;
        memset(&rs_desc, 0, sizeof(rs_desc));
        rs_desc.FillMode              = D3D11_FILL_SOLID;
        rs_desc.CullMode              = pipe.cull_mode;
        rs_desc.FrontCounterClockwise = desc.face_winding == face_winding::ccw;
        rs_desc.DepthBias             = (INT)pipe.depth_bias;
        rs_desc.DepthBiasClamp        = pipe.depth_bias_clamp;
        rs_desc.SlopeScaledDepthBias  = pipe.depth_bias_slope_scale;
        rs_desc.DepthClipEnable       = TRUE;
        rs_desc.ScissorEnable         = TRUE;
        rs_desc.MultisampleEnable     = pipe.rasteriza_sample_count;
        rs_desc.AntialiasedLineEnable = FALSE;
        D3D_CHECK2(ctx_.device->CreateRasterizerState(&rs_desc, &pipe.d3d11_rs), pipe.d3d11_rs);

        /* create depth-stencil state */
        D3D11_DEPTH_STENCIL_DESC ds_desc;
        memset(&ds_desc, 0, sizeof(ds_desc));

        ds_desc.DepthEnable      = TRUE;
        ds_desc.DepthWriteMask   = pipe.depth_enabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
        ds_desc.DepthFunc        = pipe.depth_compare_op;
        
        ds_desc.StencilEnable    = pipe.stencil_enabled;
        ds_desc.StencilReadMask  = pipe.stencil_test_mask;
        ds_desc.StencilWriteMask = pipe.stencil_write_mask;
        
        const auto& sfront = pipe.stencil_front;
        ds_desc.FrontFace.StencilFailOp      = sfront.fail_op;
        ds_desc.FrontFace.StencilDepthFailOp = sfront.depth_fail_op;
        ds_desc.FrontFace.StencilPassOp      = sfront.pass_op;
        ds_desc.FrontFace.StencilFunc        = sfront.compare_op;

        const auto& sback = pipe.stencil_back;
        ds_desc.BackFace.StencilFailOp       = sback.fail_op;
        ds_desc.BackFace.StencilDepthFailOp  = sback.depth_fail_op;
        ds_desc.BackFace.StencilPassOp       = sback.pass_op;
        ds_desc.BackFace.StencilFunc         = sback.compare_op;
        D3D_CHECK2(ctx_.device->CreateDepthStencilState( &ds_desc, &pipe.d3d11_ds), pipe.d3d11_ds);

        // TODO
        /* create blend state */
        D3D11_BLEND_DESC bs_desc;
        memset(&bs_desc, 0, sizeof(bs_desc));
        bs_desc.AlphaToCoverageEnable                 = pipe.alpha_to_coverage_enabled;
        bs_desc.IndependentBlendEnable                = FALSE;
        bs_desc.RenderTarget[0].BlendEnable           = pipe.blend_enabled;

        bs_desc.RenderTarget[0].SrcBlend              = pipe.blend_rgb.src_factor;
        bs_desc.RenderTarget[0].DestBlend             = pipe.blend_rgb.dst_factor;
        bs_desc.RenderTarget[0].BlendOp               = pipe.blend_rgb.blend_op;
        bs_desc.RenderTarget[0].SrcBlendAlpha         = pipe.blend_alpha.src_factor;
        bs_desc.RenderTarget[0].DestBlendAlpha        = pipe.blend_alpha.dst_factor;
        bs_desc.RenderTarget[0].BlendOpAlpha          = pipe.blend_alpha.blend_op;

        bs_desc.RenderTarget[0].RenderTargetWriteMask = pipe.blend_write_mask;
        D3D_CHECK2(ctx_.device->CreateBlendState( &bs_desc, &pipe.d3d11_bs), pipe.d3d11_bs);

        return pid;
    }
    frame_id    create(const    frame_desc& desc) override
    {
        frame_id pid = pool_.insert(desc); // id in the pool
        frame& frm = pool_[pid];

        // create render targets
        for (uint i = 0; i < frm.render_targets.size(); ++i) 
        {
            const auto& att = frm.render_targets[i];
            const auto& tex = pool_[att.tex_pid];
            const bool is_msaa = tex.rt_sample_count > 1;

            /* create D3D11 render-target-view */
            ID3D11Resource* d3d11_res = 0;
            D3D11_RENDER_TARGET_VIEW_DESC d3d11_rtv_desc;
            memset(&d3d11_rtv_desc, 0, sizeof(d3d11_rtv_desc));
            d3d11_rtv_desc.Format = tex.format;

            switch (tex.type)
            {
                case texture_type::two:
                    if (is_msaa) {
                        d3d11_res = (ID3D11Resource*)tex.d3d11_texmsaa;
                        d3d11_rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
                    }
                    else {
                        d3d11_res = (ID3D11Resource*)tex.d3d11_tex2d;
                        d3d11_rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                        d3d11_rtv_desc.Texture2D.MipSlice = att.level;
                    }
                    break;
                case texture_type::cube:
                case texture_type::array:
                    if (is_msaa) {
                        d3d11_res = (ID3D11Resource*)tex.d3d11_texmsaa;
                        d3d11_rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
                        d3d11_rtv_desc.Texture2DMSArray.FirstArraySlice = att.layer;
                        d3d11_rtv_desc.Texture2DMSArray.ArraySize = 1;
                    }
                    else {
                        d3d11_res = (ID3D11Resource*)tex.d3d11_tex2d;
                        d3d11_rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                        d3d11_rtv_desc.Texture2DArray.MipSlice = att.level;
                        d3d11_rtv_desc.Texture2DArray.FirstArraySlice = att.layer;
                        d3d11_rtv_desc.Texture2DArray.ArraySize = 1;
                    }
                    break;
                case texture_type::three:
                    //SOKOL_ASSERT(!is_msaa);
                    d3d11_res = (ID3D11Resource*)tex.d3d11_tex3d;
                    d3d11_rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
                    d3d11_rtv_desc.Texture3D.MipSlice = att.level;
                    d3d11_rtv_desc.Texture3D.FirstWSlice = att.layer;
                    d3d11_rtv_desc.Texture3D.WSize = 1;
                    break;
                default:
                    LOG(ERROR);
                    break;
            }
            D3D_CHECK2(ctx_.device->CreateRenderTargetView(d3d11_res, &d3d11_rtv_desc, &frm.d3d11_rtvs[i]), frm.d3d11_rtvs[i]);
        }

        // create depth-stencil target
        {
            const auto& tex = pool_[frm.depth_stencil.tex_pid];
            const bool is_msaa = tex.rt_sample_count > 1;

            ID3D11Resource* d3d11_res = (ID3D11Resource*)tex.d3d11_texds;
            D3D11_DEPTH_STENCIL_VIEW_DESC d3d11_dsv_desc;
            memset(&d3d11_dsv_desc, 0, sizeof(d3d11_dsv_desc));
            d3d11_dsv_desc.Format        = tex.format;
            d3d11_dsv_desc.ViewDimension = is_msaa ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
            D3D_CHECK2(ctx_.device->CreateDepthStencilView(d3d11_res, &d3d11_dsv_desc, &frm.d3d11_dsv), frm.d3d11_dsv);
        }

        return pid;
    }

    void destroy(  buffer_id id) override { pool_.erase(id); }
    void destroy( texture_id id) override { pool_.erase(id); }
    void destroy(  shader_id id) override { pool_.erase(id); }
    void destroy(pipeline_id id) override { pool_.erase(id); }
    void destroy(   frame_id id) override { pool_.erase(id); }

    // render
    void begin_frame(frame_id id) override
    {
        //clear_context_state();
        cmd_ = command_list_context{};

        D3D11_VIEWPORT vp;
        memset(&vp, 0, sizeof(vp));
        D3D11_RECT rect;

        if (id.value != 0)
        {
            cmd_.is_offscreen = true;
            cmd_.frm = pool_[id];

            ctx_.context->OMSetRenderTargets(
                cmd_.frm.render_targets.size(), 
                reinterpret_cast<ID3D11RenderTargetView**>(cmd_.frm.d3d11_rtvs.data()), // ???
                cmd_.frm.d3d11_dsv);

            vp.Width = (FLOAT)cmd_.frm.width;
            vp.Height = (FLOAT)cmd_.frm.height;
            vp.MaxDepth = 1.0f;

            rect.left = 0;
            rect.top = 0;
            rect.right = cmd_.frm.width;
            rect.bottom = cmd_.frm.height;
        }
        else 
        {
            // render to default frame buffer

            cmd_.is_offscreen = false;
            // TODO: cmd_.frm = ctx_.frm;

            ID3D11RenderTargetView* rtvs[1]{ ctx_.rtv };
            ctx_.context->OMSetRenderTargets(1, rtvs, ctx_.dsv);

            vp.Width = (FLOAT)renderd_.width;
            vp.Height = (FLOAT)renderd_.height;
            vp.MaxDepth = 1.0f;

            rect.left = 0;
            rect.top = 0;
            rect.right = renderd_.width;
            rect.bottom = renderd_.height;
        }

        /* set viewport and scissor rect to cover whole screen */
        ctx_.context->RSSetViewports(1, &vp);
        ctx_.context->RSSetScissorRects(1, &rect);

        log_ << ("frame    : "s + (cmd_.is_offscreen ? cmd_.frm.name : "0") + '\n');
    }
    void end_frame() override
    {
        // resolve MSAA render target into texture
        if (cmd_.is_offscreen && renderd_.enable_msaa)
        {
            for (int i = 0; i < cmd_.frm.d3d11_rtvs.size(); ++i)
            {
                const auto& att = cmd_.frm.render_targets[i];
                const auto& tex = pool_[att.tex_pid];

                if (tex.rt_sample_count > 1)
                {
                    UINT subres = att.level + att.layer * tex.mipmaps;
                    ctx_.context->ResolveSubresource(
                        (ID3D11Resource*)tex.d3d11_tex2d,     /* pDstResource */
                        subres,                                 /* DstSubresource */
                        (ID3D11Resource*)tex.d3d11_texmsaa,   /* pSrcResource */
                        subres,                                 /* SrcSubresource */
                        tex.format);
                }
            }
        }
        else
        {
            ctx_.swapchain->Present(0, 0);
        }

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
        if (cmd_.is_offscreen)
        {
            for (auto i : targets)
            {
                ctx_.context->ClearRenderTargetView(cmd_.frm.d3d11_rtvs[i], reinterpret_cast<float*>(&rgba));
            }
        }
        else
        {
            ctx_.context->ClearRenderTargetView(ctx_.rtv, reinterpret_cast<float*>(&rgba));
        }
    }
    void clear_depth(float depth) const override
    {
        UINT ds_flags = D3D11_CLEAR_DEPTH;

        if (cmd_.is_offscreen)
        {
            ctx_.context->ClearDepthStencilView(cmd_.frm.d3d11_dsv, ds_flags, depth, 0);
        }
        else
        {
            ctx_.context->ClearDepthStencilView(ctx_.dsv, ds_flags, depth, 0);
        }
    }
    void clear_stencil(uint stencil) const override
    {
        UINT ds_flags = D3D11_CLEAR_STENCIL;
        
        if (cmd_.is_offscreen)
        {
            ctx_.context->ClearDepthStencilView(cmd_.frm.d3d11_dsv, ds_flags, 0.f, stencil);
        }
        else
        {
            ctx_.context->ClearDepthStencilView(ctx_.dsv, ds_flags, 0.f, stencil);
        }
    }

    void set_viewport(uint x, uint y, uint width, uint height) override
    {
        D3D11_VIEWPORT vp;
        vp.TopLeftX = (FLOAT)x;
        vp.TopLeftY = (FLOAT)y;
        vp.Width = (FLOAT)width;
        vp.Height = (FLOAT)height;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;

        ctx_.context->RSSetViewports(1, &vp);
    }
    void set_scissor(uint x, uint y, uint width, uint height) override
    {
        D3D11_RECT rect;
        rect.left = x;
        rect.top = y;
        rect.right = x + width;
        rect.bottom = y + height;

        ctx_.context->RSSetScissorRects(1, &rect);
    }

    void apply_pipeline(const pipeline_id id, std::array<bool, 4> flags) override
    {
        cmd_.pipe = pool_[id];
        const auto& pipe = cmd_.pipe;

        if (flags[0])
            ctx_.context->IASetPrimitiveTopology(pipe.primitive_type);

        if (flags[1])
            ctx_.context->RSSetState(pipe.d3d11_rs);

        if (flags[2])
            ctx_.context->OMSetDepthStencilState(pipe.d3d11_ds, pipe.stencil_ref);

        if (flags[3])
            ctx_.context->OMSetBlendState(pipe.d3d11_bs, pipe.blend_color.data(), 0xFFFFFFFF);

        log_ << ("pipeline : "s + cmd_.pipe.name + '\n');
    }
    void apply_shader(const shader_id id) override
    {
        cmd_.shd = pool_[id];
        cmd_.tex_ids = std::vector<texture_id>(cmd_.shd.vs_samplers_sz + cmd_.shd.fs_samplers_sz, texture_id(0u));

        ctx_.context->VSSetShader(cmd_.shd.d3d11_vs, NULL, 0);
        ctx_.context->VSSetConstantBuffers(0, cmd_.shd.vs_uniform_block_sz, reinterpret_cast<ID3D11Buffer**>(cmd_.shd.d3d11_cbs.data()));
        ctx_.context->PSSetShader(cmd_.shd.d3d11_ps, NULL, 0);
        ctx_.context->PSSetConstantBuffers(0, cmd_.shd.fs_uniform_block_sz, reinterpret_cast<ID3D11Buffer**>(cmd_.shd.d3d11_cbs.data() + cmd_.shd.vs_uniform_block_sz));

        log_ << ("shader   : "s + cmd_.shd.name + "\n");
    }

    // TODO: rename uniform_block
    void bind_uniform(uint ub_index, const void* data, uint size, shader_stage /*stage*/) override
    {
        ID3D11Buffer* cb = cmd_.shd.d3d11_cbs[ub_index];
        ctx_.context->UpdateSubresource((ID3D11Resource*)cb, 0, NULL, data, 0, 0);

        log_ << ("uniform block : "s + "TODO" + '\n');
    }
    void bind_uniform(const char* name, command::uniform uniform) override
    {
        log_ << ("uniform  : "s + std::to_string(uniform.index()) + '\n');
    }

    void bind_index(const buffer_id id) override
    {
        cmd_.index_id = id;
        const auto& index = pool_[id];
        ctx_.context->IASetIndexBuffer(index.d3d11_buf, DXGI_FORMAT_R32_UINT, 0);

        //log_ << ("index    : "s + (cmd_.index_id.value == 0 ? "null" : pool_[cmd_.index_id].name) + '\n');
    }
    void bind_vertex(const buffer_id id, std::vector<size_t> attrs, std::vector<size_t> slots, size_t instance_rate) override
    {
        cmd_.buf_ids.push_back(id);

        // FIXME
        // have to bind all attrs of buffer
        DCHECK(pool_[cmd_.buf_ids.back()].layout.size() == attrs.size());

        // delay all operators to 'bind_all_vertex_instance_buffers' stage

        log_ << ("buffer   : "s + pool_[id].name + '\n');
    }
    void bind_texture(const texture_id id, int tex_index, const std::string& sampler, shader_stage /*stage*/) override
    {
        cmd_.tex_ids[tex_index] = id;

        // log_ << ("texture  : "s + pool_[id].name + '\n');
    }

    void update(buffer_id id, const void* data, int size) override
    {
    }
    void update(texture_id id, const void* data) override
    {
    }



    void draw(uint count, uint first, uint instance_count) override
    {
        bind_all_vertex_instance_buffers();

        if (instance_count == 1) 
        {
            ctx_.context->Draw(count, first);
        }
        else 
        {
            ctx_.context->DrawInstanced(count, instance_count, first, 0);
        }

        log_ << "draw\n\n";
    }
    void draw_index(uint count, uint first, uint instance_count) override
    {
        bind_all_vertex_instance_buffers();

        if (instance_count == 1) 
        {
            ctx_.context->DrawIndexed(count, first, 0);
        }
        else 
        {
            ctx_.context->DrawIndexedInstanced(count, instance_count, first, 0, 0);
        }

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

    bool resize_render_target()
    {
        D3D_CHECK(ctx_.swapchain->ResizeBuffers(1, renderd_.width, renderd_.height, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
        D3D_CHECK2(ctx_.swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&ctx_.rtt), ctx_.rtt);
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
        D3D_CHECK2(ctx_.device->CreateDepthStencilView((ID3D11Resource*)ctx_.dst, nullptr, &ctx_.dsv), ctx_.dsv);

        ID3D11RenderTargetView* rtvs[1] { ctx_.rtv };
        ctx_.context->OMSetRenderTargets(1, rtvs, ctx_.dsv);

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

    void clear_context_state()
    {
        ctx_.context->IASetVertexBuffers(0, MaxShaderBuffers, ctx_.vbs, ctx_.vb_strides, ctx_.vb_offsets);
        ctx_.context->IASetIndexBuffer(NULL, DXGI_FORMAT_UNKNOWN, 0);
        ctx_.context->IASetInputLayout(NULL);

        ctx_.context->VSSetShader(NULL, NULL, 0);
        ctx_.context->VSSetSamplers(0, MaxShaderTextures, ctx_.smps);
        ctx_.context->VSSetConstantBuffers(0, MaxShaderUniformBlocks, ctx_.cbs);
        ctx_.context->VSSetShaderResources(0, MaxShaderTextures, ctx_.srvs);

        ctx_.context->RSSetState(NULL);

        ctx_.context->PSSetShader(NULL, NULL, 0);
        ctx_.context->PSSetSamplers(0, MaxShaderTextures, ctx_.smps);
        ctx_.context->PSSetConstantBuffers(0, MaxShaderUniformBlocks, ctx_.cbs);
        ctx_.context->PSSetShaderResources(0, MaxShaderTextures, ctx_.srvs);

        ctx_.context->OMSetRenderTargets(MaxColorAttachments, ctx_.rtvs, NULL);
        ctx_.context->OMSetDepthStencilState(NULL, 0);
        ctx_.context->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);
    }

    uint row_pitch(pixel_format fmt, uint width)
    {
        if (is_compressed_pixel_format(fmt))
        {
            LOG(ERROR);
            return 0;
        }
        else
        {
            return bytesize(fmt) * width;
        }
    }

    std::vector<D3D11_SUBRESOURCE_DATA> texture_subres_data(const texture_desc& desc)
    {
        const int num_faces = (desc.type == texture_type::cube) ? 6 : 1;
        const int num_slices = (desc.type == texture_type::array) ? desc.depth : 1;

        std::vector<D3D11_SUBRESOURCE_DATA> subres_datas;
        subres_datas.resize(num_faces * num_slices * desc.mipmaps);

        int subres_index = 0;
        for (int face_index = 0; face_index < num_faces; ++face_index)
        {
            for (int slice_index = 0; slice_index < num_slices; ++slice_index)
            {
                for (int mip_index = 0; mip_index < desc.mipmaps; ++mip_index, ++subres_index)
                {
                    const int mip_width =   ((desc.width >> mip_index) > 0) ?  desc.width >> mip_index : 1;
                    const int mip_height = ((desc.height >> mip_index) > 0) ? desc.height >> mip_index : 1;

                    // const sg_subimage_content* subimg_content = &(content->subimage[face_index][mip_index]);

                    const int slice_size = desc.size / num_slices;
                    const int slice_offset = slice_size * slice_index;

                    const uint8_t* ptr = static_cast<const uint8_t*>(desc.data[face_index]);

                    D3D11_SUBRESOURCE_DATA& subres_data = subres_datas[subres_index];
                    subres_data.pSysMem = ptr + slice_offset;
                    subres_data.SysMemPitch = row_pitch(desc.format, mip_width);
                    subres_data.SysMemSlicePitch = 0;
                    if (desc.type == texture_type::three)
                    {
                        // TODO
                        // subres_data->SysMemSlicePitch = surface_pitch(desc.pixel_format, mip_width, mip_height);
                    }
                }
            }
        }

        return subres_datas;
    }

    ID3DBlobPtr compile_shader(const std::string& src, const char* target) 
    {
        ID3DBlob* output = NULL;
        ID3DBlob* errors = NULL;
        D3DCompile(
            src.data(),     /* pSrcData */
            src.size(),     /* SrcDataSize */
            NULL,                  /* pSourceName */
            NULL,                  /* pDefines */
            NULL,                  /* pInclude */
            "main",     /* pEntryPoint */
            target,     /* pTarget (vs_5_0 or ps_5_0) */
            D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_OPTIMIZATION_LEVEL3,   /* Flags1 */
            0,          /* Flags2 */
            &output,    /* ppCode */
            &errors);   /* ppErrorMsgs */

        if (errors)
        {
            LOG(ERROR) << static_cast<LPCSTR>(errors->GetBufferPointer());
        }

        return output;
    }

    void bind_all_vertex_instance_buffers(UINT vb_offset = 0)
    {
        DCHECK(cmd_.buf_ids.size() <= 2); // vertex buffer + instance buffer

        std::vector<D3D11_INPUT_ELEMENT_DESC> il_descs{};

        ID3D11Buffer* vbs[MaxShaderBuffers]{};
        UINT   vb_strides[MaxShaderBuffers]{};
        UINT   vb_offsets[MaxShaderBuffers]{ vb_offset };

        for (uint i = 0; i < cmd_.buf_ids.size(); ++i)
        {
            const auto& buf = pool_[cmd_.buf_ids[i]];

            // TODO : input slot
            il_descs.insert(il_descs.cend(), buf.layout.cbegin(), buf.layout.cend());

            vbs[i] = buf.d3d11_buf;
            vb_strides[i] = buf.stride;
            // vb_offsets
        }

        if (il_descs.size() > 0)
        {
            ID3D11InputLayoutPtr il;
            D3D_CHECK2(ctx_.device->CreateInputLayout(
                il_descs.data(), il_descs.size(), cmd_.shd.vs_blob, cmd_.shd.vs_blob_length,
                &il), il);

            ctx_.context->IASetInputLayout(il);
            ctx_.context->IASetVertexBuffers(0, MaxShaderBuffers, vbs, vb_strides, vb_offsets);
        }
    }

    // 0 texs check
    void bind_all_textures()
    {
        DCHECK((cmd_.shd.fs_samplers_sz + cmd_.shd.vs_samplers_sz) <= MaxShaderTextures);

        ID3D11ShaderResourceView* srvs[MaxShaderTextures]{};
        ID3D11SamplerState*       smps[MaxShaderTextures]{};

        for (uint i = 0; i < cmd_.shd.vs_samplers_sz; ++i)
        {
            if (cmd_.tex_ids[i])
            {
                const auto& tex = pool_[cmd_.tex_ids[i]];

                srvs[i] = tex.d3d11_srv;
                smps[i] = tex.d3d11_smp;
            }
        }

        ctx_.context->VSSetShaderResources(0, cmd_.shd.vs_samplers_sz, srvs);
        ctx_.context->VSSetSamplers(0,        cmd_.shd.vs_samplers_sz, smps);

        ctx_.context->PSSetShaderResources(0, cmd_.shd.fs_samplers_sz, srvs + cmd_.shd.vs_samplers_sz);
        ctx_.context->PSSetSamplers(0,        cmd_.shd.fs_samplers_sz, smps + cmd_.shd.vs_samplers_sz);
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

        // the following arrays are used for unbinding resources, they will always contain zeroes
        ID3D11RenderTargetView* rtvs[MaxColorAttachments];
        ID3D11Buffer* vbs[MaxShaderBuffers];
        UINT vb_offsets[MaxShaderBuffers];
        UINT vb_strides[MaxShaderBuffers];
        ID3D11Buffer* cbs[MaxShaderUniformBlocks];
        ID3D11ShaderResourceView* srvs[MaxShaderTextures];
        ID3D11SamplerState* smps[MaxShaderTextures];
        // global subresourcedata array for texture updates
        D3D11_SUBRESOURCE_DATA subres_data[MaxMipmaps * MaxTextureArrayLayers];
    };

    struct command_list_context
    {
        buffer_id index_id{};
        std::vector<buffer_id> buf_ids{};
        std::vector<texture_id> tex_ids{}; // !!!

        shader shd{};
        pipeline pipe{};

        bool is_offscreen{};
        frame frm{};
    };

    std::stringstream log_; // move to device
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

#endif // FAY_IN_WINDOWS
