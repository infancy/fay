#include "fay/core/hash.h"
#include "fay/math/math.h"
#include "fay/render/backend.h"



// https://www.3dgep.com/learning-directx-12-1/

// https://github.com/alaingalvan/directx12-seed
// https://github.com/vinjn/vgfx
// https://github.com/microsoft/DirectX-Graphics-Samples



#ifdef FAY_IN_WINDOWS

#include "backend_d3d_.h"

namespace fay::d3d12
{

using namespace fay::d3d;

inline namespace enum_
{



}

inline namespace func
{

UINT8 color_write_mask_d3d12(blend_mask mask)
{
    UINT8 res = 0;

    if (enum_have(mask, blend_mask::red))   res |= D3D12_COLOR_WRITE_ENABLE_RED;
    if (enum_have(mask, blend_mask::green)) res |= D3D12_COLOR_WRITE_ENABLE_GREEN;
    if (enum_have(mask, blend_mask::blue))  res |= D3D12_COLOR_WRITE_ENABLE_BLUE;
    if (enum_have(mask, blend_mask::alpha)) res |= D3D12_COLOR_WRITE_ENABLE_ALPHA;

    return res;
}

}

inline namespace type
{

struct buffer
{
    UINT64 btsz{};
    void* mapped_address;
    ID3D12ResourcePtr resource{};

    D3D12_INDEX_BUFFER_VIEW index_buffer_view{};

    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view{};
    std::vector<D3D12_INPUT_ELEMENT_DESC> layout{};
    D3D12_INPUT_LAYOUT_DESC input_layout_desc{};

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_view_desc{};
    D3D12_SHADER_RESOURCE_VIEW_DESC srv_view_desc{};
    D3D12_UNORDERED_ACCESS_VIEW_DESC uav_view_desc{};


    buffer() = default;
    buffer(const buffer_desc & desc) : layout(desc.layout.size())
    {
        if (desc.is_vertex_or_instance())
        {
            uint offset = 0;
            for (uint i = 0; i < desc.layout.size(); ++i)
            {
                auto& desc_attrib = desc.layout[i];
                auto& attrib = layout[i];

                attrib.SemanticName = semantic_name_map.at(desc_attrib.usage());
                attrib.SemanticIndex = desc_attrib.index(); // TODO: ???
                attrib.Format = vertex_format_map.at(desc_attrib.format());

                attrib.AlignedByteOffset = offset;
                offset += attribute_format_map.at(desc_attrib.format()).size;

                if (desc.type == buffer_type::vertex)
                {
                    attrib.InputSlot = 0;
                    attrib.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                    attrib.InstanceDataStepRate = 0;
                }
                else
                {
                    attrib.InputSlot = 1;
                    attrib.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
                    attrib.InstanceDataStepRate = 1;
                }
            }

            input_layout_desc.pInputElementDescs = layout.data();
            input_layout_desc.NumElements = layout.size();
        }
    }
};



struct texture
{
    ID3D12ResourcePtr resource{ 0 };
    // ID3D12ResourcePtr multisample_resource;

    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
    D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc{};

    D3D12_SAMPLER_DESC sampler_desc{};

    texture() = default;
    texture(const texture_desc& desc)
    {
    }
};



enum class descriptor_type
{
    undefined = 0,

    sampler,

    uniform_buffer_cbv,       // CBV | VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
    
    storage_buffer_srv,       // SRV | VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
    storage_buffer_uav,       // UAV | VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
    
    uniform_texel_buffer_srv, // SRV | VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
    storage_texel_buffer_uav, // UAV | VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
    
    texture_srv,              // SRV | VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
    texture_uav,              // UAV | VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
};

struct descriptor_table
{
    descriptor_type type;
    uint32_t base_binding; // base shader register binding point, like b0/t0...
    uint32_t count; // b0~bn/t0~tn
    //shader_stage shader_stages;

    // std::vector<size_t> render_ids;
    uint32_t heap_offset;
    uint32_t root_parameter_index;
};

// material_data(shader, shader_params, resource) 
// shader_params
// static_respack, dynamic_respack
struct respack
{
    descriptor_table sampler_table{};
    descriptor_table uniform_table{};

    std::vector<descriptor_table*> active_tables{};

    ID3D12DescriptorHeapPtr sampler_heap{};
    ID3D12DescriptorHeapPtr cbvsrvuav_heap{};

    ID3D12RootSignaturePtr respack_layout{};

    respack() {}
    respack(const respack_desc& desc)
    {
        // TODO: base_binding
        sampler_table = { descriptor_type::sampler,            0, (uint32_t)desc.textures.size() };
        uniform_table = { descriptor_type::uniform_buffer_cbv, 0, (uint32_t)desc.uniform_buffers.size() };

        descriptor_table* raw_desc_tables[] =
        {
            &sampler_table,
            &uniform_table
        };
        for (auto desc_table : raw_desc_tables)
        {
            if (desc_table->count > 0)
            {
                active_tables.emplace_back(desc_table);
            }
        }
        // check it by render_device
        //DCHECK(active_tables.size() > 0) << "empty respack";
    }
};



struct shader_unit
{
    shader_stage stage{};
    string source;
    string name;
    string target;
    string entry;

    ID3DBlobPtr blob_ptr{};
    D3D12_SHADER_BYTECODE bytecode{};
};

struct shader // shader_program
{
    shader_unit vs{};
    shader_unit hs{};
    shader_unit ds{};
    shader_unit gs{};
    shader_unit ps{};
    shader_unit cs{};

    //std::vector<respack_layout> respack_layouts; // TODO: cache them

    std::vector<shader_unit*> active_units{};

    shader() {}
    shader(const shader_desc& desc)
    {
        vs = { shader_stage::vertex,   desc.vs, "vs", };
        hs = { shader_stage::hull,     desc.hs, "hs", };
        ds = { shader_stage::domn,     desc.ds, "ds", };
        gs = { shader_stage::geometry, desc.gs, "gs", };
        ps = { shader_stage::fragment, desc.fs, "ps", };
        cs = { shader_stage::compute,  desc.cs, "cs", };

        shader_unit* raw_stages[]{ &vs, &hs, &ds, &gs, &ps, &cs };
        for (auto& stage : raw_stages)
        {
            if (!stage->source.empty())
            {
                // https://github.com/Microsoft/DirectXShaderCompiler/wiki/Shader-Model
                stage->target = stage->name + "_5_1";
                stage->entry  = stage->name + "_main";
                stage->name   = stage->name + desc.name;

                active_units.emplace_back(stage);
            }
        }
    }
};



struct pipeline
{
    D3D12_STREAM_OUTPUT_DESC stream_output_desc{};
    D3D12_PRIMITIVE_TOPOLOGY_TYPE primitive_topology{};
    D3D12_CULL_MODE cull_mode{}; // TODO: remove
    D3D12_RASTERIZER_DESC rasterizer_desc{};

    D3D12_DEPTH_STENCIL_DESC depth_stencil_desc{};
    D3D12_BLEND_DESC blend_desc{};

    pipeline() {}
    pipeline(const pipeline_desc& desc)
    {
        stream_output_desc.pSODeclaration = NULL;
        stream_output_desc.NumEntries = 0;
        stream_output_desc.pBufferStrides = NULL;
        stream_output_desc.NumStrides = 0;
        stream_output_desc.RasterizedStream = 0;


        primitive_topology = static_cast<D3D12_PRIMITIVE_TOPOLOGY_TYPE>(primitive_type_map.at(desc.primitive_type).d3d12);
        cull_mode = static_cast<D3D12_CULL_MODE>(cull_mode_map.at(desc.cull_mode).d3d12);


        rasterizer_desc.FillMode = static_cast<D3D12_FILL_MODE>(fill_mode_map.at(desc.fill_mode).d3d12);
        rasterizer_desc.CullMode = static_cast<D3D12_CULL_MODE>(cull_mode_map.at(desc.cull_mode).d3d12);
        rasterizer_desc.FrontCounterClockwise = (desc.face_winding == face_winding::ccw) ? TRUE : FALSE;
        // TODO
        rasterizer_desc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        rasterizer_desc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        rasterizer_desc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        rasterizer_desc.DepthClipEnable = TRUE;
        rasterizer_desc.MultisampleEnable = FALSE;
        rasterizer_desc.AntialiasedLineEnable = FALSE;
        rasterizer_desc.ForcedSampleCount = 0;
        rasterizer_desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;



        auto copy_stencil_state = [](const pipeline_desc::stencil_state src)
        {
            D3D12_DEPTH_STENCILOP_DESC dst;

            dst.StencilFailOp      = static_cast<D3D12_STENCIL_OP>(stencil_op_map.at(src.fail_op).d3d12);
            dst.StencilDepthFailOp = static_cast<D3D12_STENCIL_OP>(stencil_op_map.at(src.depth_fail_op).d3d12);
            dst.StencilPassOp      = static_cast<D3D12_STENCIL_OP>(stencil_op_map.at(src.pass_op).d3d12);
            dst.StencilFunc        = static_cast<D3D12_COMPARISON_FUNC>(compare_op_map.at(src.compare_op).d3d12);

            return dst;
        };

        depth_stencil_desc.DepthEnable = desc.depth_enabled ? TRUE : FALSE;
        depth_stencil_desc.DepthFunc = static_cast<D3D12_COMPARISON_FUNC>(compare_op_map.at(desc.depth_compare_op).d3d12);
        depth_stencil_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

        depth_stencil_desc.StencilEnable = desc.stencil_enabled ? TRUE : FALSE;
        depth_stencil_desc.StencilReadMask = desc.stencil_test_mask;
        depth_stencil_desc.StencilWriteMask = desc.stencil_write_mask;
        depth_stencil_desc.FrontFace = copy_stencil_state(desc.stencil_front);
        depth_stencil_desc.BackFace  = copy_stencil_state(desc.stencil_back);



        D3D12_RENDER_TARGET_BLEND_DESC render_target_desc{};
        {
            render_target_desc.BlendEnable = desc.blend_enabled ? TRUE : FALSE;

            auto rgb = desc.blend_rgb;
            render_target_desc.SrcBlend  = static_cast<D3D12_BLEND>(blend_factor_map.at(rgb.src_factor).d3d12);
            render_target_desc.DestBlend = static_cast<D3D12_BLEND>(blend_factor_map.at(rgb.src_factor).d3d12);
            render_target_desc.BlendOp   = static_cast<D3D12_BLEND_OP>(blend_op_map.at(rgb.blend_op).d3d12);

            auto alpha = desc.blend_alpha;
            render_target_desc.SrcBlendAlpha  = static_cast<D3D12_BLEND>(blend_factor_map.at(alpha.src_factor).d3d12);
            render_target_desc.DestBlendAlpha = static_cast<D3D12_BLEND>(blend_factor_map.at(alpha.src_factor).d3d12);
            render_target_desc.BlendOpAlpha   = static_cast<D3D12_BLEND_OP>(blend_op_map.at(alpha.blend_op).d3d12);

            render_target_desc.RenderTargetWriteMask = color_write_mask_d3d12(desc.blend_write_mask);

            // TODO
            render_target_desc.LogicOpEnable = FALSE;
            render_target_desc.LogicOp = D3D12_LOGIC_OP_NOOP; 
        }

        blend_desc.AlphaToCoverageEnable = desc.alpha_to_coverage_enabled ? TRUE : FALSE;
        blend_desc.IndependentBlendEnable = FALSE;
        for (auto& render_target : blend_desc.RenderTarget)
        {
            render_target = render_target_desc;
        }
    }
};

struct frame
{
    ID3D12DescriptorHeapPtr rtv_heap{};
    ID3D12DescriptorHeapPtr dsv_heap{};

    std::vector<DXGI_FORMAT> rtv_fmts{};
    DXGI_FORMAT dsv_fmt{};

    frame() {}
    frame(const frame_desc& desc)
    {
    }
};

}



#define ThrowIfFailed D3D12_CHECK

class backend_d3d12 : public render_backend
{
#pragma region field
private:
    struct IDXGIAdapter_
    {
        IDXGIAdapter3Ptr adapter;
        D3D_FEATURE_LEVEL featureLevel;

        IDXGIAdapter_(IDXGIAdapter3Ptr adapter, D3D_FEATURE_LEVEL featureLevel) :
            adapter { adapter },
            featureLevel{ featureLevel }
        {}
    };

    struct Queue_
    {
        ID3D12CommandQueuePtr queue;

        // sync
        ID3D12Fence1Ptr fence;
        UINT64 fence_value;
        HANDLE fence_event;
    };

    struct context
    {
        // Initialization
        IDXGIFactory5Ptr mFactory;
    #ifdef FAY_DEBUG
        ID3D12Debug3Ptr mDebugController;
    #endif
        std::vector<IDXGIAdapter_> mAdapterList{}; // all adapter without software adapter
        
        D3D_FEATURE_LEVEL mFeatureLevel;
        IDXGIAdapter3Ptr mAdapter; // main adapter


    #ifdef FAY_DEBUG
        ID3D12DebugDevice1Ptr mDebugDevice;
    #endif
        ID3D12DevicePtr mDevice;
        ID3D12CommandAllocatorPtr mCommandAllocator;
        Queue_ mGraphicsQueue; // use the graphics queue as present queue
        //Queue_ mPresentQueue;


        // swapchain, frame and cmdlist
        IDXGISwapChain4Ptr mSwapchain;
        static const uint SwapChainBufferCount{ 2 }; // TODO
        // Current Frame
        UINT mCurrentFrameIndex;
        texture_id tex_ids[SwapChainBufferCount]{};
        frame_id frm_ids[SwapChainBufferCount]{};

        // cmdlist
        ID3D12GraphicsCommandListPtr mCommandList{};
        ID3D12GraphicsCommandListPtr mFrameCommandList[SwapChainBufferCount]{};
        respack_id res_id{};
    };
    context ctx_{};

    using render_pool = resource_pool<buffer, texture, respack, shader, pipeline, frame>;
    render_pool pool_{};

    struct command_list_context
    {
        bool is_offscreen{};
        frame_id frm_id{};
        frame_desc frm_desc{};
        frame frm{};

        pipeline_id pipe_id{};
        pipeline pipe{};
        pipeline_desc pipe_desc{};

        shader_id shd_id{};
        shader shd{};

        respack_id res_id{};
        respack res{};

        buffer index{};
        buffer_id vertex_id{};
        buffer vertex{};
    };
    command_list_context cmd_{};

#pragma endregion field

#pragma region init
public:
    backend_d3d12(const render_desc& desc) : 
        render_backend(desc)
    {
        create_device();

        create_swapchain();
        create_swapchain_frame();
        resize_swapchain_frame();

        create_command_list();
        create_default_respack();
    }
    ~backend_d3d12()
    {
    }

private:
    void d3d12_error_handle(HRESULT hr)
    {
        if (hr == DXGI_ERROR_DEVICE_RESET)
        {
            char buff[128] = {};
            sprintf_s(buff, "Device Reset: Reason code 0x%08X\n", hr);
            OutputDebugStringA(buff);
        }


        if (hr == DXGI_ERROR_DEVICE_REMOVED)
        {
            char buff[128] = {};
            sprintf_s(buff, "Device Lost: Reason code 0x%08X\n", ctx_.mDevice->GetDeviceRemovedReason());
            OutputDebugStringA(buff);

            d3d_error_handle(ctx_.mDevice->GetDeviceRemovedReason(), "");
        }
    }

    void create_device()
    {
        UINT dxgiFactoryFlags = 0;
    #ifdef FAY_DEBUG
        // enable d3d12 debug layer

        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

        ID3D12DebugPtr debugController;
        ThrowIfFailed(
            D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));

        // ID3D12Debug3Ptr debugController1;
        // https://docs.microsoft.com/en-us/windows/win32/direct3d12/using-d3d12-debug-layer-gpu-based-validation
        ThrowIfFailed(
            debugController->QueryInterface(IID_PPV_ARGS(&ctx_.mDebugController)));
        ctx_.mDebugController->EnableDebugLayer();

        // or can't use Nvidia Nsight Graphics 
        //ctx_.mDebugController->SetEnableGPUBasedValidation(true);
    #endif
        // Create Entry Point
        ThrowIfFailed(
            CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&ctx_.mFactory)));


        // find all hardware Device
        IDXGIAdapter1Ptr adapter;
        for (UINT adapterIndex = 0;
            ctx_.mFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND;
            ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Skip software adapters
                continue;
            }

            // find the highest feature level of this hardware Device
            for (auto featureLevel : D3D_FEATURE_LEVELS)
            {
                if (featureLevel < D3D_FEATURE_LEVEL_12_0)
                    continue;

                if (SUCCEEDED(D3D12CreateDevice(adapter, featureLevel,
                    __uuidof(ID3D12Device), 
                    nullptr))) // find it, not create it
                {
                    IDXGIAdapter3Ptr adapter_;
                    if (SUCCEEDED(adapter->QueryInterface(
                        __uuidof(IDXGIAdapter3), (void**)&(adapter_))))
                    {
                        ctx_.mAdapterList.emplace_back(adapter_, featureLevel);
                        break;
                    }
                }
            }
        }
        

        // find the highest feature level adapter
        FAY_CHECK(ctx_.mAdapterList.size() > 0);
        std::sort(ctx_.mAdapterList.begin(), ctx_.mAdapterList.end(), 
            [](IDXGIAdapter_ a, IDXGIAdapter_ b) { return a.featureLevel > b.featureLevel; });
        ctx_.mAdapter = ctx_.mAdapterList[0].adapter;
        ctx_.mFeatureLevel = ctx_.mAdapterList[0].featureLevel;


        // Create Logical Device actually
        D3D12_CHECK(D3D12CreateDevice(ctx_.mAdapter, ctx_.mFeatureLevel,
            IID_PPV_ARGS(&ctx_.mDevice)));
        // TODO
        ctx_.mDevice->SetName(L"MainD3D12Device");
    #ifdef FAY_DEBUG
        // Get debug device
        ThrowIfFailed(ctx_.mDevice->QueryInterface(&ctx_.mDebugDevice));
    #endif


        // create command producer
        ThrowIfFailed(
            ctx_.mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&ctx_.mCommandAllocator)));


        // create command consumer
        auto& gfxQueue = ctx_.mGraphicsQueue;
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        ThrowIfFailed(
            ctx_.mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&gfxQueue.queue)));
        
        // sync
        ThrowIfFailed(
            ctx_.mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&gfxQueue.fence)));
        gfxQueue.fence_value = 1;
        gfxQueue.fence_event = CreateEvent(NULL, FALSE, FALSE, NULL);
        FAY_CHECK(gfxQueue.fence_event != nullptr);
    }

    // can call it repeatedly when windows size change
    void create_swapchain()
    {
        assert(ctx_.mDevice != nullptr);

        DXGI_SWAP_CHAIN_DESC1 desc = {};
        desc.Width = render_desc_.width;
        desc.Height = render_desc_.height;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // TODO
        desc.Stereo = false;

        desc.SampleDesc.Count = 1; // If multisampling is needed, we'll resolve it later
        //desc.SampleDesc.Quality = p_renderer->settings.swapchain.sample_quality;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.BufferCount = ctx_.SwapChainBufferCount;
        desc.Scaling = DXGI_SCALING_STRETCH;
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        desc.Flags = 0;

        if ((desc.SwapEffect == DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL) ||
            (desc.SwapEffect == DXGI_SWAP_EFFECT_FLIP_DISCARD))
        {
            if (desc.BufferCount < 2)
            {
                desc.BufferCount = 2;
            }
        }

        if (desc.BufferCount > DXGI_MAX_SWAP_CHAIN_BUFFERS)
        {
            throw std::exception("too many buffer count of swapchain");
        }

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC* fullscreenDesc = nullptr;
        IDXGIOutput* output = nullptr;

        IDXGISwapChain1Ptr swapchain;
        D3D12_CHECK(ctx_.mFactory->CreateSwapChainForHwnd(
            ctx_.mGraphicsQueue.queue, 
            static_cast<HWND>(render_desc_.d3d_handle), 
            &desc, fullscreenDesc, output, &swapchain));

        D3D12_CHECK(ctx_.mFactory->MakeWindowAssociation(
            static_cast<HWND>(render_desc_.d3d_handle),
            DXGI_MWA_NO_ALT_ENTER));

        D3D12_CHECK(swapchain->QueryInterface(IID_PPV_ARGS(&ctx_.mSwapchain)));
    }

    // int left, int top, uint width, uint height, int min_depth, int max_depth
    void resize_swapchain(uint width, uint height)
    {
        assert(ctx_.mSwapchain != nullptr);

        ctx_.mSwapchain->ResizeBuffers(ctx_.SwapChainBufferCount, width, height,
            DXGI_FORMAT_R8G8B8A8_UNORM, 0);
        ctx_.mCurrentFrameIndex = ctx_.mSwapchain->GetCurrentBackBufferIndex();
    }

    void create_swapchain_frame()
    {
        for (int i = 0; i < ctx_.SwapChainBufferCount; ++i)
        {
            texture_desc tex_desc;
            tex_desc.as_render_target = render_target::color;
            tex_desc.format = pixel_format::rgba8;
            ctx_.tex_ids[i] = pool_.insert(tex_desc);


            // dummy
            frame_desc frm_desc;
            ctx_.frm_ids[i] = pool_.insert(frm_desc);
        }
    }

    void resize_swapchain_frame()
    {
        for (size_t i = 0; i < ctx_.SwapChainBufferCount; ++i)
        {
            texture& tex = pool_[ctx_.tex_ids[i]];
            if (tex.resource)
            {
                tex.resource->Release();
            }
            HRESULT hres = ctx_.mSwapchain->GetBuffer(i, IID_PPV_ARGS(&tex.resource));
            assert(SUCCEEDED(hres));


            frame& frm = pool_[ctx_.frm_ids[i]];
            if (frm.rtv_heap)
            {
                frm.rtv_heap->Release();
                frm.rtv_heap = nullptr;
            }

            frame_desc frm_desc;
            frm_desc.render_targets.push_back({ ctx_.tex_ids[i] });
            ctx_.frm_ids[i] = create(frm_desc);
        }
    }

    bool resize_render_target()
    {
        return false;
    }

    void create_command_list()
    {
        for(int i = 0; i < ctx_.SwapChainBufferCount; ++i)
        {
            auto& cmdlist = ctx_.mFrameCommandList[i];

            ID3D12PipelineState* initialState = NULL;
            HRESULT hres = ctx_.mDevice->CreateCommandList(
                0, D3D12_COMMAND_LIST_TYPE_DIRECT, ctx_.mCommandAllocator, initialState,
                IID_PPV_ARGS(&cmdlist));
            assert(SUCCEEDED(hres));

            // Command lists are created in the recording state, but there is nothing
            // to record yet. The main loop expects it to be closed, so close it now.
            cmdlist->Close();
        }

        // TODO
        ctx_.mCommandList = ctx_.mFrameCommandList[0];
    }

    void create_default_respack()
    {
        respack_desc desc;
        ctx_.res_id = create(desc);
    }

#pragma endregion init

#pragma region create, update and destroy
public:
    virtual   buffer_id create(const   buffer_desc& buf_desc) override
    {
        auto pid = pool_.insert(buf_desc);
        buffer& buffer = pool_[pid];



        // TODO: enum_contain
        buffer.btsz = buf_desc.btsz;
        if (enum_have(buf_desc.type, buffer_type::uniform_cbv))
        {
            buffer.btsz = round_up(buffer.btsz, 256);
        }

        D3D12_RESOURCE_DESC res_desc = {};
        res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        res_desc.Alignment = 0;
        res_desc.Width = buffer.btsz;
        res_desc.Height = 1;
        res_desc.DepthOrArraySize = 1;
        res_desc.MipLevels = 1;
        res_desc.Format = DXGI_FORMAT_UNKNOWN;
        res_desc.SampleDesc.Count = 1;
        res_desc.SampleDesc.Quality = 0;
        res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
        if (enum_have(buf_desc.type, buffer_type::storage_uav) ||
            enum_have(buf_desc.type, buffer_type::counter_uav))
        {
            res_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        }

        // Adjust for padding
        UINT64 padded_size = 0;
        ctx_.mDevice->GetCopyableFootprints(&res_desc, 0, 1, 0, NULL, NULL, NULL, &padded_size);
        buffer.btsz = padded_size;
        res_desc.Width = padded_size;



        D3D12_RESOURCE_STATES res_states = (D3D12_RESOURCE_STATES)buffer_type_map.at(buf_desc.type).d3d12;
        D3D12_HEAP_PROPERTIES heap_props = {};
        heap_props.Type = D3D12_HEAP_TYPE_DEFAULT;
        heap_props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heap_props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heap_props.CreationNodeMask = 1;
        heap_props.VisibleNodeMask = 1;

        D3D12_HEAP_FLAGS heap_flags = D3D12_HEAP_FLAG_NONE;
        // TODO
        //if (buf_desc.usage == resource_usage::stream)
        {
            // D3D12_HEAP_TYPE_UPLOAD requires D3D12_RESOURCE_STATE_GENERIC_READ
            heap_props.Type = D3D12_HEAP_TYPE_UPLOAD;
            res_states = D3D12_RESOURCE_STATE_GENERIC_READ;
        }

        D3D12_CHECK2(ctx_.mDevice->CreateCommittedResource(
            &heap_props, heap_flags, &res_desc, res_states, NULL, IID_PPV_ARGS(&buffer.resource)), buffer.resource);

        // TODO
        //if (buf_desc.usage == resource_usage::stream)
        {
            D3D12_RANGE read_range = { 0, 0 };
            D3D12_CHECK2(buffer.resource->Map(
                0, &read_range, (void**)&(buffer.mapped_address)), buffer.mapped_address);
            std::memcpy(buffer.mapped_address, buf_desc.data, buf_desc.btsz);
        }


        int first_element = 0;
        bool raw = false;
        switch (buf_desc.type)
        {
            case buffer_type::index:
            {
                buffer.index_buffer_view.BufferLocation =
                    buffer.resource->GetGPUVirtualAddress();
                buffer.index_buffer_view.SizeInBytes = (UINT)buf_desc.btsz;
                buffer.index_buffer_view.Format = DXGI_FORMAT_R32_UINT;
            }
            break;

            case buffer_type::vertex:
            {
                buffer.vertex_buffer_view.BufferLocation =
                    buffer.resource->GetGPUVirtualAddress();
                buffer.vertex_buffer_view.SizeInBytes = (UINT)buf_desc.btsz;
                buffer.vertex_buffer_view.StrideInBytes = buf_desc.layout.stride();
            }
            break;

            case buffer_type::uniform_cbv:
            {
                buffer.cbv_view_desc.BufferLocation = buffer.resource->GetGPUVirtualAddress();
                buffer.cbv_view_desc.SizeInBytes = (UINT)buf_desc.btsz;
            }
            break;

            case buffer_type::storage_srv:
            {
                D3D12_SHADER_RESOURCE_VIEW_DESC* desc = &(buffer.srv_view_desc);
                desc->Format = DXGI_FORMAT_UNKNOWN;
                desc->ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
                desc->Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
                desc->Buffer.FirstElement = first_element;
                desc->Buffer.NumElements = (UINT)(buf_desc.count);
                desc->Buffer.StructureByteStride = (UINT)(buf_desc.stride);
                desc->Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
                if (raw)
                {
                    desc->Format = DXGI_FORMAT_R32_TYPELESS;
                    desc->Buffer.Flags |= D3D12_BUFFER_SRV_FLAG_RAW;
                }
            }
            break;

            case buffer_type::storage_uav:
            {
                D3D12_UNORDERED_ACCESS_VIEW_DESC* desc = &(buffer.uav_view_desc);
                desc->Format = DXGI_FORMAT_UNKNOWN;
                desc->ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
                desc->Buffer.FirstElement = first_element;
                desc->Buffer.NumElements = (UINT)(buf_desc.count);
                desc->Buffer.StructureByteStride = (UINT)(buf_desc.stride);
                desc->Buffer.CounterOffsetInBytes = 0;
                desc->Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
                if (raw)
                {
                    desc->Format = DXGI_FORMAT_R32_TYPELESS;
                    desc->Buffer.Flags |= D3D12_BUFFER_UAV_FLAG_RAW;
                }
            }
            break;

            case buffer_type::uniform_texel_srv:
            {
                D3D12_SHADER_RESOURCE_VIEW_DESC* desc = &(buffer.srv_view_desc);
                desc->Format = (DXGI_FORMAT)pixel_format_map.at(buf_desc.pixel_format).d3d11;
                desc->ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
                desc->Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
                desc->Buffer.FirstElement = first_element;
                desc->Buffer.NumElements = (UINT)(buf_desc.count);
                desc->Buffer.StructureByteStride = (UINT)(buf_desc.stride);
                desc->Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
            }
            break;

            case buffer_type::storage_texel_uav:
            {
                D3D12_UNORDERED_ACCESS_VIEW_DESC* desc = &(buffer.uav_view_desc);
                desc->Format = DXGI_FORMAT_UNKNOWN;
                desc->ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
                desc->Buffer.FirstElement = first_element;
                desc->Buffer.NumElements = (UINT)(buf_desc.count);
                desc->Buffer.StructureByteStride = (UINT)(buf_desc.stride);
                desc->Buffer.CounterOffsetInBytes = 0;
                desc->Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
            }
            break;

            case buffer_type::counter_uav:
            {
                D3D12_UNORDERED_ACCESS_VIEW_DESC* desc = &(buffer.uav_view_desc);
                desc->Format = DXGI_FORMAT_R32_TYPELESS;
                desc->ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
                desc->Buffer.FirstElement = first_element;
                desc->Buffer.NumElements = (UINT)(buf_desc.count);
                desc->Buffer.StructureByteStride = (UINT)(buf_desc.stride);
                desc->Buffer.CounterOffsetInBytes = 0;
                desc->Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
            }
            break;
        }

        return pid;
    }
    virtual  texture_id create(const  texture_desc& tex_desc) override
    {
        auto pid = pool_.insert(tex_desc);
        texture& tex = pool_[pid];



        if (tex.resource == nullptr)
        {
            D3D12_HEAP_PROPERTIES heap_props = {};
            heap_props.Type = D3D12_HEAP_TYPE_DEFAULT;
            heap_props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            heap_props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            heap_props.CreationNodeMask = 1;
            heap_props.VisibleNodeMask = 1;



            D3D12_RESOURCE_DIMENSION res_dim = D3D12_RESOURCE_DIMENSION_UNKNOWN;
            switch (tex_desc.type)
            {
            case texture_type::one:
                res_dim = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
                break;
            case texture_type::two:
                res_dim = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                break;
            case texture_type::three:
                res_dim = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
                break;
            case texture_type::cube:
                res_dim = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                break;
            }
            assert(D3D12_RESOURCE_DIMENSION_UNKNOWN != res_dim);

            D3D12_HEAP_FLAGS heap_flags = D3D12_HEAP_FLAG_NONE;

            D3D12_RESOURCE_DESC res_desc = {};
            res_desc.Dimension = res_dim;
            res_desc.Alignment = 0;
            res_desc.Width = tex_desc.width;
            res_desc.Height = tex_desc.height;
            res_desc.DepthOrArraySize = tex_desc.depth;
            res_desc.MipLevels = (UINT16)tex_desc.mipmaps;
            res_desc.Format = (DXGI_FORMAT)pixel_format_map.at(tex_desc.format).d3d11;
            // TODO
            res_desc.SampleDesc.Count = 1;
            res_desc.SampleDesc.Quality = 0;
            res_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

            if (enum_have(tex_desc.as_render_target, render_target::color))
            {
                res_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
            }
            if (enum_have(tex_desc.as_render_target, render_target::depth_stencil))
            {
                res_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
            }
            /*
            if (enum_have(tex_desc.as_render_target, render_target::color))
            {
                desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            }
            */


            D3D12_RESOURCE_STATES res_states = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            if (enum_have(tex_desc.as_render_target, render_target::color))
            {
                res_states = D3D12_RESOURCE_STATE_RENDER_TARGET;
            }



            D3D12_CLEAR_VALUE clear_value = {};
            clear_value.Format = (DXGI_FORMAT)pixel_format_map.at(tex_desc.format).d3d11;
            if (enum_have(tex_desc.as_render_target, render_target::depth_or_stencil))
            {
                // TODO
                clear_value.DepthStencil.Depth = 0.f;
                clear_value.DepthStencil.Stencil = 0;
            }
            else
            {
                // TODO: fay::array::fill(clear_value.Color, tex_desc.clear_value);
                clear_value.Color[0] = 0.f;
                clear_value.Color[1] = 0.f;
                clear_value.Color[2] = 0.f;
                clear_value.Color[3] = 0.f;
            }

            D3D12_CLEAR_VALUE* p_clear_value = NULL;
            if ((res_desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) ||
                (res_desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
            {
                p_clear_value = &clear_value;
            }



            HRESULT hres = ctx_.mDevice->CreateCommittedResource(
                &heap_props, heap_flags, &res_desc, res_states, p_clear_value,
                IID_PPV_ARGS(&tex.resource));
            assert(SUCCEEDED(hres));
        }

        if (enum_have(tex_desc.usage_, texture_usage_::sampled_texture))
        {
            D3D12_SRV_DIMENSION view_dim = D3D12_SRV_DIMENSION_UNKNOWN;
            switch (tex_desc.type)
            {
            case texture_type::one:
                view_dim = D3D12_SRV_DIMENSION_TEXTURE1D;
                break;
            case texture_type::two:
                view_dim = D3D12_SRV_DIMENSION_TEXTURE2D;
                break;
            case texture_type::three:
                view_dim = D3D12_SRV_DIMENSION_TEXTURE3D;
                break;
            case texture_type::cube:
                view_dim = D3D12_SRV_DIMENSION_TEXTURE2D;
                break;
            }
            assert(D3D12_SRV_DIMENSION_UNKNOWN != view_dim);


            tex.srv_desc.Shader4ComponentMapping =
                D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            tex.srv_desc.Format = (DXGI_FORMAT)pixel_format_map.at(tex_desc.format).d3d11;
            tex.srv_desc.ViewDimension = view_dim;
            tex.srv_desc.Texture2D.MipLevels = (UINT)tex_desc.mipmaps;
        }

        if (enum_have(tex_desc.usage_, texture_usage_::storage_texture))
        {
            tex.uav_desc.Format = (DXGI_FORMAT)pixel_format_map.at(tex_desc.format).d3d11;
            tex.uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
            tex.uav_desc.Texture2D.MipSlice = 0;
            tex.uav_desc.Texture2D.PlaneSlice = 0;
        }

        // TODO
        {
            tex.sampler_desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            tex.sampler_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            tex.sampler_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            tex.sampler_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            tex.sampler_desc.MipLODBias = 0;
            tex.sampler_desc.MaxAnisotropy = 0;
            tex.sampler_desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
            tex.sampler_desc.BorderColor[0] = 0.0f;
            tex.sampler_desc.BorderColor[1] = 0.0f;
            tex.sampler_desc.BorderColor[2] = 0.0f;
            tex.sampler_desc.BorderColor[3] = 0.0f;
            tex.sampler_desc.MinLOD = 0.0f;
            tex.sampler_desc.MaxLOD = D3D12_FLOAT32_MAX;
        }

        return pid;
    }
    virtual  respack_id create(const  respack_desc& res_desc) override
    {    
        auto pid = pool_.insert(res_desc);
        respack& res = pool_[pid];

        // only descriptor_set, doesn't need contain vertex_layout
        // create_descriptor_set

        // create resource heap

        UINT sampler_count = res_desc.textures.size();
        if (sampler_count > 0)
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
            desc.NumDescriptors = sampler_count;
            desc.NodeMask = 0;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

            HRESULT hres = ctx_.mDevice->CreateDescriptorHeap(
                &desc, IID_PPV_ARGS(&res.sampler_heap));
            assert(SUCCEEDED(hres));

            // TODO
            res.sampler_table.heap_offset = 0;
        }
        // TODO
        UINT cbvsrvuav_count = res_desc.uniform_buffers.size(); // + res_desc.textures.size()
        if (cbvsrvuav_count > 0)
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.NumDescriptors = cbvsrvuav_count;
            desc.NodeMask = 0;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

            HRESULT hres = ctx_.mDevice->CreateDescriptorHeap(
                &desc, IID_PPV_ARGS(&res.cbvsrvuav_heap));
            assert(SUCCEEDED(hres));

            // TODO
            res.uniform_table.heap_offset = 0;
        }



        // create resource pack layout 

        D3D12_FEATURE_DATA_ROOT_SIGNATURE feature_data = {};
        feature_data.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
        HRESULT hres = ctx_.mDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE,
            &feature_data, sizeof(feature_data));
        if (FAILED(hres))
        {
            feature_data.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        const int descriptor_table_count = res.active_tables.size();
        // Allocate everything with an upper bound of descriptor counts
        std::vector<D3D12_ROOT_PARAMETER1> parameters_11(descriptor_table_count);
        std::vector<D3D12_ROOT_PARAMETER> parameters_10(descriptor_table_count);

        std::vector<D3D12_DESCRIPTOR_RANGE1> ranges_11(descriptor_table_count);
        std::vector<D3D12_DESCRIPTOR_RANGE> ranges_10(descriptor_table_count);

        // Build ranges
        int paramter_range_index = 0;
        for (int descriptor_index = 0; descriptor_index < descriptor_table_count; ++descriptor_index)
        {
            auto* descriptor = res.active_tables[descriptor_index];


            D3D12_ROOT_PARAMETER1* param_11 = &parameters_11[paramter_range_index];
            D3D12_ROOT_PARAMETER* param_10 = &parameters_10[paramter_range_index];

            param_11->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            param_10->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;

            // Start out with visibility on all shader stages
            // TODO: optimization
            param_11->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            param_10->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


            D3D12_DESCRIPTOR_RANGE1* range_11 = &ranges_11[paramter_range_index];
            D3D12_DESCRIPTOR_RANGE* range_10 = &ranges_10[paramter_range_index];

            bool assign_range = false;
            switch (descriptor->type)
            {
                case descriptor_type::sampler:
                {
                    range_11->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
                    range_10->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
                    assign_range = true;
                }
                break;

                case descriptor_type::uniform_buffer_cbv:
                {
                    range_11->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                    range_10->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                    assign_range = true;
                }
                break;

                case descriptor_type::storage_buffer_srv:
                case descriptor_type::uniform_texel_buffer_srv:
                case descriptor_type::texture_srv:
                {
                    range_11->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                    range_10->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                    assign_range = true;
                }
                break;

                case descriptor_type::storage_buffer_uav:
                case descriptor_type::storage_texel_buffer_uav:
                case descriptor_type::texture_uav:
                {
                    range_11->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                    range_10->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                    assign_range = true;
                }
                break;
            }

            if (assign_range)
            {
                range_11->NumDescriptors = descriptor->count;
                range_11->BaseShaderRegister = descriptor->base_binding;
                range_11->RegisterSpace = 0;
                range_11->Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
                range_11->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

                range_10->NumDescriptors = descriptor->count;
                range_10->BaseShaderRegister = descriptor->base_binding;
                range_10->RegisterSpace = 0;
                range_10->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


                param_11->DescriptorTable.pDescriptorRanges = range_11;
                param_11->DescriptorTable.NumDescriptorRanges = 1;

                param_10->DescriptorTable.pDescriptorRanges = range_10;
                param_10->DescriptorTable.NumDescriptorRanges = 1;


                descriptor->root_parameter_index = paramter_range_index;
                ++paramter_range_index;
            }
        }
        int parameter_count = paramter_range_index;


        // create root signature

        hres = 0;
        D3D12_VERSIONED_ROOT_SIGNATURE_DESC desc = {};
        ID3DBlobPtr sig_blob;
        ID3DBlobPtr error_msgs;
        if (feature_data.HighestVersion == D3D_ROOT_SIGNATURE_VERSION_1_1)
        {
            desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
            desc.Desc_1_1.NumParameters = parameter_count;
            desc.Desc_1_1.pParameters = parameters_11.data();
            desc.Desc_1_1.NumStaticSamplers = 0;
            desc.Desc_1_1.pStaticSamplers = NULL;
            desc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

            hres = D3D12SerializeVersionedRootSignature(&desc, &sig_blob, &error_msgs);
        }
        else if (feature_data.HighestVersion == D3D_ROOT_SIGNATURE_VERSION_1_0)
        {
            desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_0;
            desc.Desc_1_0.NumParameters = parameter_count;
            desc.Desc_1_0.pParameters = parameters_10.data();
            desc.Desc_1_0.NumStaticSamplers = 0;
            desc.Desc_1_0.pStaticSamplers = NULL;
            desc.Desc_1_0.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

            hres = D3D12SerializeRootSignature(&(desc.Desc_1_0), D3D_ROOT_SIGNATURE_VERSION_1_0,
                &sig_blob, &error_msgs);
        }
        assert(SUCCEEDED(hres));

        hres = 0;
        hres = ctx_.mDevice->CreateRootSignature(0, sig_blob->GetBufferPointer(),
            sig_blob->GetBufferSize(),
            IID_PPV_ARGS(&res.respack_layout));
        assert(SUCCEEDED(hres));



        return pid;
    }
    virtual   shader_id create(const   shader_desc& shd_desc) override
    {
        auto pid = pool_.insert(shd_desc);
        shader& shd = pool_[pid];

        UINT compile_flags = 0;
    #ifdef FAY_DEBUG
        // Enable better shader debugging with the graphics debugging tools.
        compile_flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
    #endif

        D3D_SHADER_MACRO macros[] = { "D3D12", "1", NULL, NULL };
        for (auto unit_ptr : shd.active_units)
        {
            auto& unit = *unit_ptr;

            ID3DBlobPtr error_msgs;
            HRESULT hres = D3DCompile2(
                unit.source.c_str(), unit.source.size(), unit.name.c_str(), 
                macros, NULL, 
                unit.entry.c_str(), unit.target.c_str(),
                compile_flags, 0, 0, NULL, 0, &unit.blob_ptr, &error_msgs);

            if (FAILED(hres))
            {
                // std::string msg{ error_msgs->GetBufferSize() + 1 };
                std::vector<char> msg(error_msgs->GetBufferSize() + 1);
                std::memcpy(msg.data(), error_msgs->GetBufferPointer(), error_msgs->GetBufferSize());
                
                LOG(ERROR) << "d3d12 create_shader " << msg.data();
            }

            unit.bytecode.BytecodeLength  = unit.blob_ptr->GetBufferSize();
            unit.bytecode.pShaderBytecode = unit.blob_ptr->GetBufferPointer();
        }

        return pid;
    }
    virtual pipeline_id create(const pipeline_desc& pipe_desc) override
    {
        auto pid = pool_.insert(pipe_desc);
        pipeline& pipe = pool_[pid]; // all is taken care of by pipe's ctor 

        return pid;
    }
    virtual    frame_id create(const    frame_desc& frm_desc) override
    {
        auto pid = pool_.insert(frm_desc);
        frame& frm = pool_[pid];

        // create RTV
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            desc.NumDescriptors = frm_desc.render_targets.size();
            desc.NodeMask = 0;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            D3D12_CHECK(ctx_.mDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&frm.rtv_heap)));

            D3D12_CPU_DESCRIPTOR_HANDLE handle = frm.rtv_heap->GetCPUDescriptorHandleForHeapStart();
            const UINT inc_size = ctx_.mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            
            for (uint32_t i = 0; i < frm_desc.render_targets.size(); ++i)
            {
                auto tex_id = frm_desc.render_targets[i].tex_id;
                const auto& tex = pool_[tex_id];
                const auto& tex_desc = pool_.desc(tex_id);

                frm.rtv_fmts.push_back(static_cast<DXGI_FORMAT>(pixel_format_map.at(tex_desc.format).d3d11));

                bool use_msaa = tex_desc.rt_sample_count > 1;
                if (use_msaa)
                {
                    LOG(ERROR) << "not support MSAA now";
                    //ctx_.mDevice->CreateRenderTargetView(tex.multisample_resource, NULL, handle);
                }
                else
                {
                    ctx_.mDevice->CreateRenderTargetView(tex.resource, NULL, handle);
                }
                handle.ptr += inc_size;
            }
        }

        auto tex_id = frm_desc.depth_stencil.tex_id;
        if(tex_id)
        {
            const auto& tex = pool_[tex_id];
            const auto& tex_desc = pool_.desc(tex_id);

            frm.dsv_fmt = static_cast<DXGI_FORMAT>(pixel_format_map.at(tex_desc.format).d3d11);

            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
            desc.NumDescriptors = 1;
            desc.NodeMask = 0;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            D3D12_CHECK(ctx_.mDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&frm.dsv_heap)));

            D3D12_CPU_DESCRIPTOR_HANDLE handle = frm.dsv_heap->GetCPUDescriptorHandleForHeapStart();
            //const UINT inc_size = ctx_.mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

            //const auto& [tex_desc, tex] = pool_.pair(tex_id);
            //static_assert(std::is_same_v<decltype(tex_desc), const fay::texture_desc&>);
            //static_assert(std::is_same_v<decltype(tex), const fay::d3d::type::texture&>);

            bool use_msaa = tex_desc.rt_sample_count > 1;
            if (use_msaa)
            {
                LOG(ERROR) << "not support MSAA now";
                //ctx_.mDevice->CreateDepthStencilView(tex.multisample_resource, NULL, handle);
            }
            else
            {
                ctx_.mDevice->CreateDepthStencilView(tex.resource, NULL, handle);
            }
        }

        return pid;
    }

    virtual void update( buffer_id id, const void* data, int size) override {}
    virtual void update(texture_id id, const void* data) override {}
    // directly create a new respack???
    virtual void update(respack_id id, const respack_desc& update_desc) override
    {
        const auto& origin_desc = pool_.desc(id);
        respack& res = pool_[id];

        for (uint32_t i = 0; i < res.active_tables.size(); ++i)
        {
            descriptor_table* descriptor_table = res.active_tables[i];

            /*
            switch (descriptor_table->type)
            {
                case descriptor_type::sampler:
                {
                    D3D12_CPU_DESCRIPTOR_HANDLE handle =
                        res.sampler_heap->GetCPUDescriptorHandleForHeapStart();
                    UINT handle_inc_size = ctx_.mDevice->GetDescriptorHandleIncrementSize(
                        D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
                    handle.ptr += descriptor_table->heap_offset * handle_inc_size;

                    for (uint32_t i = 0; i < descriptor_table->count; ++i)
                    {
                        D3D12_SAMPLER_DESC* sampler_desc = &(descriptor_table->samplers[i]->dx_sampler_desc);
                        ctx_.mDevice->CreateSampler(sampler_desc, handle);
                        handle.ptr += handle_inc_size;
                    }
                }
                break;

                case descriptor_type::uniform_buffer_cbv:
                {
                    D3D12_CPU_DESCRIPTOR_HANDLE handle =
                        res.cbvsrvuav_heap->GetCPUDescriptorHandleForHeapStart();
                    UINT handle_inc_size = ctx_.mDevice->GetDescriptorHandleIncrementSize(
                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                    handle.ptr += descriptor_table->heap_offset * handle_inc_size;

                    for (uint32_t i = 0; i < descriptor_table->count; ++i)
                    {
                        ID3D12Resource* resource = descriptor_table->uniform_buffers[i]->dx_resource;
                        D3D12_CONSTANT_BUFFER_VIEW_DESC* view_desc =
                            &(descriptor_table->uniform_buffers[i]->dx_cbv_view_desc);
                        ctx_.mDevice->CreateConstantBufferView(view_desc, handle);
                        handle.ptr += handle_inc_size;
                    }
                }
                break;

                case descriptor_type::storage_buffer_srv:
                case descriptor_type::uniform_texel_buffer_srv:
                {
                    D3D12_CPU_DESCRIPTOR_HANDLE handle =
                        res.cbvsrvuav_heap->GetCPUDescriptorHandleForHeapStart();
                    UINT handle_inc_size = ctx_.mDevice->GetDescriptorHandleIncrementSize(
                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                    handle.ptr += descriptor_table->heap_offset * handle_inc_size;

                    for (uint32_t i = 0; i < descriptor_table->count; ++i)
                    {
                        ID3D12Resource* resource = descriptor_table->buffers[i]->dx_resource;
                        D3D12_SHADER_RESOURCE_VIEW_DESC* view_desc =
                            &(descriptor_table->buffers[i]->dx_srv_view_desc);
                        ctx_.mDevice->CreateShaderResourceView(resource, view_desc, handle);
                        handle.ptr += handle_inc_size;
                    }
                }
                break;

                case descriptor_type::storage_buffer_uav:
                case descriptor_type::storage_texel_buffer_uav:
                {
                    D3D12_CPU_DESCRIPTOR_HANDLE handle =
                        res.cbvsrvuav_heap->GetCPUDescriptorHandleForHeapStart();
                    UINT handle_inc_size = ctx_.mDevice->GetDescriptorHandleIncrementSize(
                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                    handle.ptr += descriptor_table->heap_offset * handle_inc_size;

                    for (uint32_t i = 0; i < descriptor_table->count; ++i)
                    {
                        ID3D12Resource* resource = descriptor_table->buffers[i]->dx_resource;
                        D3D12_UNORDERED_ACCESS_VIEW_DESC* view_desc =
                            &(descriptor_table->buffers[i]->dx_uav_view_desc);
                        if (descriptor_table->buffers[i]->counter_buffer != NULL)
                        {
                            ID3D12Resource* counter_resource =
                                descriptor_table->buffers[i]->counter_buffer->dx_resource;
                            ctx_.mDevice->CreateUnorderedAccessView(resource, counter_resource,
                                view_desc, handle);
                        }
                        else
                        {
                            ctx_.mDevice->CreateUnorderedAccessView(resource, NULL, view_desc,
                                handle);
                        }
                        handle.ptr += handle_inc_size;
                    }
                }
                break;

                case descriptor_type::texture_srv:
                {
                    D3D12_CPU_DESCRIPTOR_HANDLE handle =
                        res.cbvsrvuav_heap->GetCPUDescriptorHandleForHeapStart();
                    UINT handle_inc_size = ctx_.mDevice->GetDescriptorHandleIncrementSize(
                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                    handle.ptr += descriptor_table->heap_offset * handle_inc_size;

                    for (uint32_t i = 0; i < descriptor_table->count; ++i)
                    {
                        ID3D12Resource* resource = descriptor_table->textures[i]->dx_resource;
                        D3D12_SHADER_RESOURCE_VIEW_DESC* view_desc =
                            &(descriptor_table->textures[i]->dx_srv_view_desc);
                        ctx_.mDevice->CreateShaderResourceView(resource, view_desc, handle);
                        handle.ptr += handle_inc_size;
                    }
                }
                break;

                case descriptor_type::texture_uav:
                {
                    D3D12_CPU_DESCRIPTOR_HANDLE handle =
                        res.cbvsrvuav_heap->GetCPUDescriptorHandleForHeapStart();
                    UINT handle_inc_size = ctx_.mDevice->GetDescriptorHandleIncrementSize(
                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                    handle.ptr += descriptor_table->heap_offset * handle_inc_size;

                    for (uint32_t i = 0; i < descriptor_table->count; ++i)
                    {
                        ID3D12Resource* resource = descriptor_table->textures[i]->dx_resource;
                        D3D12_UNORDERED_ACCESS_VIEW_DESC* view_desc =
                            &(descriptor_table->textures[i]->dx_uav_view_desc);
                        ctx_.mDevice->CreateUnorderedAccessView(resource, NULL, view_desc, handle);
                        handle.ptr += handle_inc_size;
                    }
                }
                break;
            }
            */
        }
    }

    virtual void destroy(  buffer_id id) override {}
    virtual void destroy( texture_id id) override {}
    virtual void destroy(  shader_id id) override {}
    virtual void destroy(pipeline_id id) override {}
    virtual void destroy(   frame_id id) override {}

private:
    // TODO: id
    ID3D12PipelineStatePtr create_rasterization_state(
        const buffer& buf, const respack& res, 
        const shader& shd, const pipeline& pipe, const frame& frm)
    {
        D3D12_CACHED_PIPELINE_STATE cached_pso_desc = {};
        cached_pso_desc.pCachedBlob = NULL;
        cached_pso_desc.CachedBlobSizeInBytes = 0;

        D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_state_desc = {};
        pipeline_state_desc.pRootSignature = res.respack_layout;
        pipeline_state_desc.VS = shd.vs.bytecode;
        pipeline_state_desc.HS = shd.hs.bytecode;
        pipeline_state_desc.DS = shd.ds.bytecode;
        pipeline_state_desc.GS = shd.gs.bytecode;
        pipeline_state_desc.PS = shd.ps.bytecode;
        pipeline_state_desc.NodeMask = 0;
        pipeline_state_desc.CachedPSO = cached_pso_desc;
        pipeline_state_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

        pipeline_state_desc.InputLayout = buf.input_layout_desc;
        pipeline_state_desc.StreamOutput = pipe.stream_output_desc;
        pipeline_state_desc.PrimitiveTopologyType = pipe.primitive_topology;        
        pipeline_state_desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED; // https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_index_buffer_strip_cut_value
        pipeline_state_desc.RasterizerState = pipe.rasterizer_desc;

        pipeline_state_desc.DepthStencilState = pipe.depth_stencil_desc;
        pipeline_state_desc.BlendState = pipe.blend_desc;

        // TODO
        DXGI_SAMPLE_DESC sample_desc = {};
        sample_desc.Count = 1;
        sample_desc.Quality = 0;
        pipeline_state_desc.SampleDesc = sample_desc;
        pipeline_state_desc.SampleMask = UINT_MAX;


        pipeline_state_desc.DSVFormat = frm.dsv_fmt;
        pipeline_state_desc.NumRenderTargets = frm.rtv_fmts.size();
        for (uint32_t i = 0; i < frm.rtv_fmts.size(); ++i)
        {
            pipeline_state_desc.RTVFormats[i] = frm.rtv_fmts[i];
        }



        ID3D12PipelineStatePtr ptr;
        D3D12_CHECK2(ctx_.mDevice->CreateGraphicsPipelineState(
            &pipeline_state_desc, IID_PPV_ARGS(&ptr)), ptr);

        return ptr;
    }

    ID3D12PipelineStatePtr create_compute_state(const respack& res, const shader& shd)
    {
        D3D12_CACHED_PIPELINE_STATE cached_pso_desc = {};
        cached_pso_desc.pCachedBlob = NULL;
        cached_pso_desc.CachedBlobSizeInBytes = 0;

        D3D12_COMPUTE_PIPELINE_STATE_DESC pipeline_state_desc = {};
        pipeline_state_desc.pRootSignature = res.respack_layout;
        pipeline_state_desc.CS = shd.cs.bytecode;
        pipeline_state_desc.NodeMask = 0;
        pipeline_state_desc.CachedPSO = cached_pso_desc;
        pipeline_state_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

        ID3D12PipelineStatePtr ptr;
        HRESULT hres = ctx_.mDevice->CreateComputePipelineState(
            &pipeline_state_desc, IID_PPV_ARGS(&ptr));
        assert(SUCCEEDED(hres));

        return ptr;
    }

#pragma endregion create, update and destroy

#pragma region render command
public:
    void begin_cmdlist()
    {
        D3D12_CHECK(ctx_.mCommandAllocator->Reset());

        ctx_.mCurrentFrameIndex = ctx_.mSwapchain->GetCurrentBackBufferIndex();
        //ctx_.mCommandList = ctx_.mFrameCommandList[ctx_.mCurrentFrameIndex]; // TODO
        D3D12_CHECK(ctx_.mCommandList->Reset(ctx_.mCommandAllocator, nullptr));
    }

    void end_cmdlist()
    {
        D3D12_CHECK(ctx_.mCommandList->Close());
    }

    virtual void begin() override
    {
        begin_cmdlist();
        //transition_render_target(texture_usage_::present, texture_usage_::color_attachment);
    }
    virtual void end() override
    {
        end_cmdlist();
        queue_submit();

        if (!cmd_.is_offscreen)
        {
            queue_present();
        }

        queue_wait_idle();
    }

    virtual void clear_command_list() override
    {
        cmd_ = command_list_context{};

        cmd_.res_id = ctx_.res_id;
        cmd_.res = pool_[ctx_.res_id]; // default respack can't be nullptr, so get it a empty respack.
    }

    // WARNNING: use 0 as default frame(rather than invalid value) by limitations of command_list
    virtual void begin_frame(frame_id id) override
    {
        cmd_.is_offscreen = id.operator bool();
        if (cmd_.is_offscreen)
        {

        }
        else
        {
            id = ctx_.frm_ids[ctx_.mCurrentFrameIndex];
        }

        cmd_.frm_id = id;
        cmd_.frm = pool_[id];
        cmd_.frm_desc = pool_.desc(id);

        if (!cmd_.is_offscreen)
        {
            transition_render_target(texture_usage_::present, texture_usage_::color_attachment);
            // This needs to be reset whenever the command list is reset
            set_viewport(0, 0, render_desc_.width, render_desc_.height);
            set_scissor(0, 0, render_desc_.width, render_desc_.height);
        }

        D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = {};
        D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle = {};
        D3D12_CPU_DESCRIPTOR_HANDLE* p_rtv_handle = NULL;
        D3D12_CPU_DESCRIPTOR_HANDLE* p_dsv_handle = NULL;

        if (cmd_.frm.rtv_fmts.size() > 0)
        {
            rtv_handle = cmd_.frm.rtv_heap->GetCPUDescriptorHandleForHeapStart();
            p_rtv_handle = &rtv_handle;
        }

        if (cmd_.frm.dsv_fmt)
        {
            dsv_handle = cmd_.frm.dsv_heap->GetCPUDescriptorHandleForHeapStart();
            p_dsv_handle = &dsv_handle;
        }

        ctx_.mCommandList->OMSetRenderTargets(
            cmd_.frm.rtv_fmts.size(), p_rtv_handle, TRUE, 
            p_dsv_handle);
    }
    virtual void end_frame() override
    {
        if(!cmd_.is_offscreen)
        {
            transition_render_target(texture_usage_::color_attachment, texture_usage_::present);
        }

        // multisample texture to original texture
        // TODO
    }

    virtual void clear_color(glm::vec4 rgba, std::vector<uint> targets) const override
    {
        if (cmd_.frm.rtv_heap)
        {
            UINT inc_size = ctx_.mDevice->GetDescriptorHandleIncrementSize(
                D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

            for (auto index : targets)
            {
                D3D12_CPU_DESCRIPTOR_HANDLE handle =
                    cmd_.frm.rtv_heap->GetCPUDescriptorHandleForHeapStart();
                handle.ptr += index * inc_size;
                ctx_.mCommandList->ClearRenderTargetView(handle, (float*)(&rgba), 0, NULL);
            }
        }
    }
    virtual void clear_depth(float depth) const override
    {
        if (cmd_.frm.dsv_heap)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE handle =
                cmd_.frm.dsv_heap->GetCPUDescriptorHandleForHeapStart();

            ctx_.mCommandList->ClearDepthStencilView(
                handle, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
        }
    }
    virtual void clear_stencil(uint stencil) const override
    {
        if (cmd_.frm.dsv_heap)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE handle =
                cmd_.frm.dsv_heap->GetCPUDescriptorHandleForHeapStart();

            ctx_.mCommandList->ClearDepthStencilView(
                handle, D3D12_CLEAR_FLAG_DEPTH, 0.f, (uint8_t)stencil, 0, nullptr);
        }
    }

    virtual void set_viewport(uint x, uint y, uint width, uint height) override
    {
        D3D12_VIEWPORT viewport = {};
        viewport.TopLeftX = x;
        viewport.TopLeftY = y;
        viewport.Width = width;
        viewport.Height = height;
        viewport.MinDepth = 0.f;
        viewport.MaxDepth = 1.f;

        ctx_.mCommandList->RSSetViewports(1, &viewport);
    }
    virtual void set_scissor(uint x, uint y, uint width, uint height) override
    {
        D3D12_RECT scissor = {};
        scissor.left = x;
        scissor.top = y;
        scissor.right = x + width;
        scissor.bottom = y + height;

        ctx_.mCommandList->RSSetScissorRects(1, &scissor);
    }



    virtual void apply_pipeline(const pipeline_id id, std::array<bool, 4>) override
    {
        FAY_DCHECK(pool_.contains(id));
        cmd_.pipe_id = id;
        cmd_.pipe = pool_[id];
        cmd_.pipe_desc = pool_.desc(id);
    }
    virtual void apply_shader(const shader_id id) override
    {
        cmd_.shd_id = id;
        cmd_.shd = pool_[id];
    }

    //virtual void bind_resource() override {}

    // bind resource, create RootSignature, ShaderInputLayout
    void bind_respack(const respack_id& id)
    {
        // by currenty cmds_
        // actually we pack resource descriptor up, not resource themselves

        cmd_.res = pool_[id];
    }

    virtual void bind_index(const buffer_id id) override
    {
        cmd_.index = pool_[id];

        ctx_.mCommandList->IASetIndexBuffer(&cmd_.index.index_buffer_view);
    }
    virtual void bind_vertex(const buffer_id id, std::vector<size_t> attrs, std::vector<size_t> slots, size_t instance_rate) override
    {
        cmd_.vertex_id = id;
        cmd_.vertex = pool_[id];
        set_backend_state();

        // TODO
        D3D12_VERTEX_BUFFER_VIEW views[1]{};
        for (uint32_t i = 0; i < 1; ++i)
        {
            views[i] = cmd_.vertex.vertex_buffer_view;
        }

        ctx_.mCommandList->IASetVertexBuffers(0, 1, views);
    }

    virtual void bind_uniform(const std::string& name, command::uniform uniform, shader_stage stage = shader_stage::none) override
    {

    }
    virtual void bind_uniform(uint ub_index, const void* data, uint size, shader_stage stage = shader_stage::none) override
    {

    }
    virtual void bind_texture(const texture_id id, int tex_index, const std::string& sampler, shader_stage stage = shader_stage::none) override
    {

    }



    virtual void draw(uint vertex_count, uint first_vertex, uint instance_count) override
    {
        //set_backend_state();

        ctx_.mCommandList->DrawInstanced((UINT)vertex_count, (UINT)1, (UINT)first_vertex, (UINT)0);
    }
    virtual void draw_index(uint index_count, uint first_index, uint instance_count) override
    {
        //set_backend_state();

        ctx_.mCommandList->DrawIndexedInstanced((UINT)index_count, (UINT)1, (UINT)first_index, (UINT)0,
            (UINT)0);
    }

protected:
    std::unordered_map<string, ID3D12PipelineStatePtr> cache_pso_map{};
    void set_backend_state()
    {
        // TODO
        uint ids[] = { cmd_.frm_id.value, cmd_.pipe_id.value, cmd_.shd_id.value, cmd_.res_id.value, cmd_.vertex_id.value };
        string hash_string{};
        for (auto id : ids)
        {
            hash_string += std::to_string(id);
            hash_string += " ";
        }

        if (cache_pso_map.contains(hash_string))
        {
            auto pso = cache_pso_map[hash_string];
            ctx_.mCommandList->SetPipelineState(pso);
        }
        else
        {
            auto pso = create_rasterization_state(cmd_.vertex, cmd_.res, cmd_.shd, cmd_.pipe, cmd_.frm);
            cache_pso_map[hash_string] = pso;
        }


        ctx_.mCommandList->SetGraphicsRootSignature(cmd_.res.respack_layout);
        auto primitive_topology = static_cast<D3D_PRIMITIVE_TOPOLOGY>(primitive_topology_map.at(cmd_.pipe_desc.primitive_type));
        ctx_.mCommandList->IASetPrimitiveTopology(primitive_topology);


        // bind respack
        auto& res = cmd_.res;

        uint32_t descriptor_heap_count = 0;
        ID3D12DescriptorHeap* descriptor_heaps[2];
        if (res.cbvsrvuav_heap)
        {
            descriptor_heaps[descriptor_heap_count] = res.cbvsrvuav_heap;
            ++descriptor_heap_count;
        }
        if (res.sampler_heap)
        {
            descriptor_heaps[descriptor_heap_count] = res.sampler_heap;
            ++descriptor_heap_count;
        }
        // TODO
        if (descriptor_heap_count > 0)
        {
            ctx_.mCommandList->SetDescriptorHeaps(descriptor_heap_count, descriptor_heaps);
        }

        for (uint32_t i = 0; i < res.active_tables.size(); ++i)
        {
            descriptor_table* descriptor_table = res.active_tables[i];

            if (descriptor_table->root_parameter_index == UINT32_MAX)
            {
                FAY_LOG(ERROR) << "overflow";
            }

            if (descriptor_table->type == descriptor_type::sampler)
            {
                D3D12_GPU_DESCRIPTOR_HANDLE handle =
                    res.sampler_heap->GetGPUDescriptorHandleForHeapStart();
                UINT handle_inc_size = ctx_.mDevice->GetDescriptorHandleIncrementSize(
                    D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
                handle.ptr += descriptor_table->heap_offset * handle_inc_size;

                ctx_.mCommandList->SetGraphicsRootDescriptorTable(descriptor_table->root_parameter_index,
                    handle);
            }
            else
            {
                D3D12_GPU_DESCRIPTOR_HANDLE handle =
                    res.cbvsrvuav_heap->GetGPUDescriptorHandleForHeapStart();
                UINT handle_inc_size =
                    ctx_.mDevice->GetDescriptorHandleIncrementSize(
                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                handle.ptr += descriptor_table->heap_offset * handle_inc_size;

                ctx_.mCommandList->SetGraphicsRootDescriptorTable(
                    descriptor_table->root_parameter_index, handle);

                // TODO
                /*
                if (p_pipeline->type == tr_pipeline_type_graphics)
                {
                    ctx_.mCommandList->SetGraphicsRootDescriptorTable(
                        descriptor_table->root_parameter_index, handle);
                }
                else if (p_pipeline->type == tr_pipeline_type_compute)
                {
                    ctx_.mCommandList->SetComputeRootDescriptorTable(
                        descriptor_table->root_parameter_index, handle);
                }
                */
            }
        }
    }

    void transition_buffer(const buffer& buf, buffer_type old_type, buffer_type new_type)
    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = buf.resource;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(buffer_type_map.at(old_type).d3d12);
        barrier.Transition.StateAfter  = static_cast<D3D12_RESOURCE_STATES>(buffer_type_map.at(new_type).d3d12);

        ctx_.mCommandList->ResourceBarrier(1, &barrier);
    }

    void transition_texture(const texture& tex, texture_usage_ old_usage, texture_usage_ new_usage)
    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = tex.resource;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(texture_usage_map.at(old_usage).d3d12);
        barrier.Transition.StateAfter  = static_cast<D3D12_RESOURCE_STATES>(texture_usage_map.at(new_usage).d3d12);

        ctx_.mCommandList->ResourceBarrier(1, &barrier);
    }

    void transition_render_target(texture_usage_ old_usage, texture_usage_ new_usage)
    {
        bool use_msaa = false;
        if (use_msaa)
        {
            // TODO
        }
        else
        {
            if (!cmd_.is_offscreen)
            {
                if (cmd_.frm_desc.render_targets.size() == 1)
                {
                    bool render = 
                        old_usage == texture_usage_::present &&
                        new_usage == texture_usage_::color_attachment;

                    bool present = 
                        old_usage == texture_usage_::color_attachment &&
                        new_usage == texture_usage_::present;

                    if (render || present)
                    {
                        auto tex_id = cmd_.frm_desc.render_targets[0].tex_id;
                        transition_texture(pool_[tex_id], old_usage, new_usage);
                    }
                }
            }
        }
    }

    void transition_depth_stencil(texture_usage_ old_usage, texture_usage_ new_usage)
    {
        bool use_msaa = false;
        if (use_msaa)
        {
            // TODO
        }
        else
        {
        }
    }



    void queue_submit()
    {
        ID3D12CommandList* cmds[]{ ctx_.mCommandList };
        ctx_.mGraphicsQueue.queue->ExecuteCommandLists(1, cmds);
    }

    void queue_present()
    {
        UINT sync_interval = 1;
        UINT flags = 0;
        ctx_.mSwapchain->Present(sync_interval, flags);
    }

    void queue_wait_idle()
    {
        // TODO
        // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.

        // Signal and increment the fence value.
        auto& gfx_queue = ctx_.mGraphicsQueue;

        const UINT64 fence_value = gfx_queue.fence_value;
        ThrowIfFailed(ctx_.mGraphicsQueue.queue->Signal(gfx_queue.fence, fence_value));
        ++gfx_queue.fence_value;

        // Wait until the previous frame is finished.
        if (gfx_queue.fence->GetCompletedValue() < fence_value)
        {
            ThrowIfFailed(gfx_queue.fence->SetEventOnCompletion(fence_value, gfx_queue.fence_event));
            WaitForSingleObject(gfx_queue.fence_event, INFINITE);
        }
    }

    // low level api, mainly for d3d12, vulkan and metal
    virtual void acquire_next_image() override
    {
        //ctx_.mCurrentFrameIndex = ctx_.mSwapchain->GetCurrentBackBufferIndex();
    }



    // compute
    void dispatch()
    {
        //ctx_.mCommandList->Dispatch(group_count_x, group_count_y, group_count_z);
    }

    void copy_buffer_to_texture()
    {
    }

#pragma endregion render command

protected:
    // interface provided for render_device
    virtual render_desc_pool& get_render_desc_pool() override { return pool_; }

}; // class backend_d3d12

} // namespace fay::d3d



namespace fay
{

render_backend_ptr create_backend_d3d12(const render_desc& desc)
{
    return std::make_unique<fay::d3d12::backend_d3d12>(desc);
}

} // namespace fay

#endif // FAY_IN_WINDOWS