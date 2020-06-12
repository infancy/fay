#include "fay/render/backend.h"

#ifdef FAY_IN_WINDOWS

#include "backend_d3d_.h"
#include <d3d12.h>

// https://www.3dgep.com/learning-directx-12-1/

// https://github.com/alaingalvan/directx12-seed
// https://github.com/vinjn/vgfx
// https://github.com/microsoft/DirectX-Graphics-Samples

namespace fay::d3d
{

inline namespace enum_
{



}

inline namespace type
{

struct buffer
{
    buffer() {}
    buffer(buffer_desc desc)
    {

    }
};

struct texture
{
    texture() {}
    texture(texture_desc desc)
    {

    }
};



enum descriptor_type
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

struct descriptor
{
    descriptor_type type;
    uint32_t binding;
    uint32_t count;
    shader_stage shader_stages;

    /*
    tr_buffer* uniform_buffers[tr_max_descriptor_entries];
    tr_texture* textures[tr_max_descriptor_entries];
    tr_sampler* samplers[tr_max_descriptor_entries];
    tr_buffer* buffers[tr_max_descriptor_entries];
    */

    union
    {
        buffer uniform_buffer;
        texture texture;


        buffer_id uniform_buffer_id;
        texture_id texture_id;
    };

    uint32_t heap_offset;
    uint32_t root_parameter_index;
};

// RootSignature
struct respack_layout
{
    std::vector<descriptor> uniform_descs;
    std::vector<descriptor> sampler_descs;

    ID3D12DescriptorHeapPtr uniform_array;
    ID3D12DescriptorHeapPtr sampler_array;

    ID3D12RootSignature* root_signature;
};

// static_respack, dynamic_respack
// material_data
// shader_params
struct respack
{
    std::vector<descriptor> uniform_descs;
    std::vector<descriptor> sampler_descs;

    ID3D12DescriptorHeapPtr uniform_array;
    ID3D12DescriptorHeapPtr sampler_array;

    ID3D12RootSignature* respack_layout;

    respack() {}
    respack(const respack_desc& desc)
    {
        uniform_descs.resize(desc.uniform_buffers.size());
        sampler_descs.resize(desc.textures.size());
    }
};

struct shader
{
    std::vector<respack_layout> respack_layouts; // TODO: cache them

    shader() {}
    shader(shader_desc desc)
    {

    }
};



struct pipeline
{
    pipeline() {}
    pipeline(pipeline_desc desc)
    {

    }
};

struct frame
{
    frame() {}
    frame(frame_desc desc)
    {

    }
};

}

#define ThrowIfFailed D3D_CHECK

class backend_d3d12 : public render_backend
{
private:
    struct IDXGIAdapter_
    {
        IDXGIAdapter3Ptr adapter;
        D3D_FEATURE_LEVEL featureLevel;
    };

    struct Queue_
    {
        ID3D12CommandQueuePtr queue;

        // sync
        ID3D12Fence1Ptr wait_idle_fence;
        UINT64          wait_idle_fence_value;
        HANDLE          wait_idle_fence_event;
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
        ID3D12Device6Ptr mDevice;
        Queue_ mGraphicsQueue; // use the graphics queue as present queue
        //Queue_ mPresentQueue;
        ID3D12CommandAllocatorPtr mCommandAllocator;
        ID3D12GraphicsCommandListPtr mCommandList;


        // swap chain
        IDXGISwapChain4Ptr mSwapchain;
        static const uint SwapChainBufferCount{ 2 };
        // Current Frame
        UINT mCurrentBuffer;
        ID3D12DescriptorHeapPtr mRtvHeap;
        ID3D12ResourcePtr mRenderTargets[SwapChainBufferCount]; // TODO


        // Resources
        D3D12_VIEWPORT mViewport;
        D3D12_RECT mScissorRect;

        ID3D12ResourcePtr mVertexBuffer;
        ID3D12ResourcePtr mIndexBuffer;

        ID3D12ResourcePtr mUniformBuffer;
        ID3D12DescriptorHeapPtr mUniformBufferHeap;
        UINT8* mMappedUniformBuffer;

        D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
        D3D12_INDEX_BUFFER_VIEW mIndexBufferView;

        UINT mRtvDescriptorSize;
        ID3D12RootSignaturePtr mRootSignature;
        ID3D12PipelineStatePtr mPipelineState;


        // sync
        UINT mFrameIndex;
        HANDLE mFenceEvent;
        ID3D12Fence1Ptr mFence;
        UINT64 mFenceValue;
    };
    context ctx_{};

    using render_pool = resource_pool<buffer, texture, respack, shader, pipeline, frame>;
    render_pool pool_{};

public:
    backend_d3d12(const render_desc& desc) : 
        render_backend(desc)
    {
        create_device();
        create_swapchain();

        resize_render_target();
    }
    ~backend_d3d12()
    {
    }

private:
    void create_device()
    {
        UINT dxgiFactoryFlags = 0;
#ifdef FAY_DEBUG
        // enable d3d12 debug layer

        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

        ID3D12Debug3Ptr debugController;
        ThrowIfFailed(
            D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));

        // ID3D12Debug3Ptr debugController1;
        // https://docs.microsoft.com/en-us/windows/win32/direct3d12/using-d3d12-debug-layer-gpu-based-validation
        ThrowIfFailed(
            debugController->QueryInterface(IID_PPV_ARGS(&ctx_.mDebugController)));
        ctx_.mDebugController->EnableDebugLayer();
        ctx_.mDebugController->SetEnableGPUBasedValidation(true);
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
            [](IDXGIAdapter_ a, IDXGIAdapter_ b)
            {
                return a.featureLevel > b.featureLevel;
            });
        ctx_.mAdapter = ctx_.mAdapterList[0].adapter;
        ctx_.mFeatureLevel = ctx_.mAdapterList[0].featureLevel;


        // Create Logical Device actually
        ThrowIfFailed(D3D12CreateDevice(ctx_.mAdapter, ctx_.mFeatureLevel,
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
        // Sync
        ThrowIfFailed(
            ctx_.mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&gfxQueue.wait_idle_fence)));
        gfxQueue.wait_idle_fence_value = 1;
        gfxQueue.wait_idle_fence_event = CreateEvent(NULL, FALSE, FALSE, NULL);
        FAY_CHECK(gfxQueue.wait_idle_fence_event != nullptr);
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
        desc.BufferCount = ctx_.SwapChainBufferCount; // TODO
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
        D3D_CHECK(ctx_.mFactory->CreateSwapChainForHwnd(
            ctx_.mGraphicsQueue.queue, 
            static_cast<HWND>(render_desc_.d3d_handle), 
            &desc, fullscreenDesc, output, &swapchain));

        D3D_CHECK(ctx_.mFactory->MakeWindowAssociation(
            static_cast<HWND>(render_desc_.d3d_handle),
            DXGI_MWA_NO_ALT_ENTER));

        D3D_CHECK(swapchain->QueryInterface(IID_PPV_ARGS(&ctx_.mSwapchain)));
    }

    // int left, int top, uint width, uint height, int min_depth, int max_depth
    void resize_swapchain(uint width, uint height)
    {
        assert(ctx_.mSwapchain != nullptr);

        ctx_.mScissorRect.left = 0;
        ctx_.mScissorRect.top = 0;
        ctx_.mScissorRect.right = static_cast<LONG>(width);
        ctx_.mScissorRect.bottom = static_cast<LONG>(height);

        ctx_.mViewport.TopLeftX = 0.f;
        ctx_.mViewport.TopLeftY = 0.f;
        ctx_.mViewport.Width = static_cast<float>(width);
        ctx_.mViewport.Height = static_cast<float>(height);
        ctx_.mViewport.MinDepth = 0.1f;
        ctx_.mViewport.MaxDepth = 1000.f;

        ctx_.mSwapchain->ResizeBuffers(ctx_.SwapChainBufferCount, width, height,
            DXGI_FORMAT_R8G8B8A8_UNORM, 0);
        ctx_.mFrameIndex = ctx_.mSwapchain->GetCurrentBackBufferIndex();
    }

    /*
    // default frame
    void create_swapchain_frame()
    {
        assert(ctx_.mDevice != nullptr);
        assert(ctx_.mSwapchain != nullptr);

        std::vector<ID3D12Resource*> swapchain_images(ctx_.SwapChainBufferCount);
        for (uint32_t i = 0; i < ctx_.SwapChainBufferCount; ++i)
        {
            D3D_CHECK(ctx_.mSwapchain->GetBuffer(i, __uuidof(*swapchain_images.data()),
                (void**)&(swapchain_images[i])));
        }

        // Populate the vk_image field and create the Vulkan texture objects
        for (size_t i = 0; i < ctx_.SwapChainBufferCount; ++i)
        {
            tr_render_target* render_target = p_renderer->swapchain_render_targets[i];
            render_target->color_attachments[0]->dx_resource = swapchain_images[i];

            create_texture(p_renderer, render_target->color_attachments[0]);
            if (p_renderer->settings.swapchain.sample_count > tr_sample_count_1)
            {
                tr_internal_dx_create_texture(p_renderer,
                    render_target->color_attachments_multisample[0]);
            }

            if (NULL != render_target->depth_stencil_attachment)
            {
                tr_internal_dx_create_texture(p_renderer, render_target->depth_stencil_attachment);

                if (p_renderer->settings.swapchain.sample_count > tr_sample_count_1)
                {
                    tr_internal_dx_create_texture(p_renderer,
                        render_target->depth_stencil_attachment_multisample);
                }
            }
        }

        // Initialize Vulkan render target objects
        for (uint32_t i = 0; i < ctx_.SwapChainBufferCount; ++i)
        {
            tr_render_target* render_target = p_renderer->swapchain_render_targets[i];
            tr_internal_dx_create_render_target(p_renderer, false, render_target);
        }
    }
    */

    void resize_swapchain_frame()
    {
        // destory default frame
        for (size_t i = 0; i < ctx_.SwapChainBufferCount; ++i)
        {
            if (ctx_.mRenderTargets[i])
            {
                ctx_.mRenderTargets[i]->Release();
                ctx_.mRenderTargets[i] = 0;
            }
        }
        if (ctx_.mRtvHeap)
        {
            ctx_.mRtvHeap->Release();
            ctx_.mRtvHeap = nullptr;
        }


        // create default frame
        ctx_.mCurrentBuffer = ctx_.mSwapchain->GetCurrentBackBufferIndex();

        // Create descriptor heaps
        {
            // create a render target view (RTV) descriptor heap.
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
            rtvHeapDesc.NumDescriptors = ctx_.SwapChainBufferCount;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            ThrowIfFailed(ctx_.mDevice->CreateDescriptorHeap(&rtvHeapDesc,
                IID_PPV_ARGS(&ctx_.mRtvHeap)));

            ctx_.mRtvDescriptorSize = ctx_.mDevice->GetDescriptorHandleIncrementSize(
                D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        }

        // Create RTV/frame resources by descriptor heaps
        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(
                ctx_.mRtvHeap->GetCPUDescriptorHandleForHeapStart());

            // Create a RTV for each frame.
            for (UINT n = 0; n < ctx_.SwapChainBufferCount; n++)
            {
                ThrowIfFailed(
                    ctx_.mSwapchain->GetBuffer(n, IID_PPV_ARGS(&ctx_.mRenderTargets[n])));
                ctx_.mDevice->CreateRenderTargetView(ctx_.mRenderTargets[n], nullptr,
                    rtvHandle);
                rtvHandle.ptr += (1 * ctx_.mRtvDescriptorSize);
            }
        }

        // TODO: DSV
    }

    bool resize_render_target()
    {
    }

public:

    virtual   buffer_id create(const   buffer_desc& desc) override
    {
        auto pid = pool_.insert(desc);
        buffer& buf = pool_[pid];



        return pid;
    }
    virtual  texture_id create(const  texture_desc& desc) override
    {
        auto pid = pool_.insert(desc);
        texture& buf = pool_[pid];



        return pid;
    }
    // only descriptor_set, doesn't need contain vertex_layout
    // create_descriptor_set
    virtual respack_id create(const respack_desc& desc) override
    {
        auto pid = pool_.insert(desc);
        respack& pack = pool_[pid];

        // TODO
        UINT cbvsrvuav_count = desc.uniform_buffers.size();
        if (cbvsrvuav_count > 0)
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.NumDescriptors = cbvsrvuav_count;
            desc.NodeMask = 0;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

            HRESULT hres = ctx_.mDevice->CreateDescriptorHeap(
                &desc, IID_PPV_ARGS(&pack.uniform_array));
            assert(SUCCEEDED(hres));

            for (int i = 0; i < pack.uniform_descs.size(); ++i)
            {
                pack.uniform_descs[i].heap_offset = i;
            }
        }

        UINT sampler_count = desc.textures.size();
        if (sampler_count > 0)
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
            desc.NumDescriptors = sampler_count;
            desc.NodeMask = 0;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

            HRESULT hres = ctx_.mDevice->CreateDescriptorHeap(
                &desc, IID_PPV_ARGS(&pack.sampler_array));
            assert(SUCCEEDED(hres));

            for (int i = 0; i < pack.sampler_descs.size(); ++i)
            {
                pack.sampler_descs[i].heap_offset = i;
            }
        }

        return pid;
    }
    virtual   shader_id create(const   shader_desc& desc) override
    {
        auto pid = pool_.insert(desc);
        shader& shd = pool_[pid];



        return pid;
    }
    virtual pipeline_id create(const pipeline_desc& desc) override
    {
        auto pid = pool_.insert(desc);
        pipeline& buf = pool_[pid];



        return pid;
    }
    virtual    frame_id create(const    frame_desc& desc) override
    {
        auto pid = pool_.insert(desc);
        frame& frm = pool_[pid];



        return pid;
    }

    virtual void update(buffer_id id, const void* data, int size) override {}
    virtual void update(texture_id id, const void* data) override {}

    virtual void destroy(buffer_id id) override {}
    virtual void destroy(texture_id id) override {}
    virtual void destroy(shader_id id) override {}
    virtual void destroy(pipeline_id id) override {}
    virtual void destroy(frame_id id) override {}

private:



public:
    // render command

    // WARNNING: use 0 as default frame(rather than invalid value) by limitations of command_list
    virtual void begin_frame(frame_id id) override {}
    virtual void end_frame() override {}

    virtual void clear_color(glm::vec4 rgba, std::vector<uint> targets) const override {}
    virtual void clear_depth(float depth) const override {}
    virtual void clear_stencil(uint stencil) const override {}

    virtual void set_viewport(uint x, uint y, uint width, uint height) override {}
    virtual void set_scissor(uint x, uint y, uint width, uint height) override {}



    virtual void apply_pipeline(const pipeline_id id, std::array<bool, 4>) override {}
    virtual void apply_shader(const shader_id id) override {}



    virtual void bind_index(const buffer_id id) override {}
    virtual void bind_vertex(const buffer_id id, std::vector<size_t> attrs, std::vector<size_t> slots, size_t instance_rate) override {}

    virtual void bind_uniform(const std::string& name, command::uniform uniform, shader_stage stage = shader_stage::none) override {}
    virtual void bind_uniform(uint ub_index, const void* data, uint size, shader_stage stage = shader_stage::none) override {}
    virtual void bind_texture(const texture_id id, int tex_index, const std::string& sampler, shader_stage stage = shader_stage::none) override {}

    //virtual void bind_resource() override {}

    // bind resource, create RootSignature, ShaderInputLayout
    void bind_respack(const respack_desc& desc)
    {
        // by currenty cmds_
        // actually we pack resource descriptor up, not resource themselves
    }



    virtual void draw(uint count, uint first, uint instance_count) override {}
    virtual void draw_index(uint count, uint first, uint instance_count) override {}

protected:
    // low level api, mainly for d3d12, vulkan and metal
    virtual void acquire_next_image() override {}

protected:
    // interface provided for render_device
    virtual render_desc_pool& get_render_desc_pool() override { return pool_; }

}; // class backend_d3d12

} // namespace fay::d3d



namespace fay
{

render_backend_ptr create_backend_d3d12(const render_desc& desc)
{
    return std::make_unique<fay::d3d::backend_d3d12>(desc);
}

} // namespace fay

#endif // FAY_IN_WINDOWS