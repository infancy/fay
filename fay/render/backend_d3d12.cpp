#include "fay/render/backend.h"

#ifdef FAY_IN_WINDOWS

#include "backend_d3d_.h"
#include <d3d12.h>

// https://www.3dgep.com/learning-directx-12-1/

// https://github.com/alaingalvan/directx12-seed
// https://github.com/vinjn/vgfx
// https://github.com/microsoft/DirectX-Graphics-Samples

namespace fay::d3d12
{

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

struct shader
{
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

class backend_d3d12 : public render_backend
{
private:
    struct context
    {
        // Initialization
        IDXGIFactory4* mFactory;
        IDXGIAdapter1* mAdapter;
    #ifdef FAY_DEBUG
        ID3D12Debug1* mDebugController;
        ID3D12DebugDevice* mDebugDevice;
    #endif
        ID3D12Device* mDevice;
        ID3D12CommandQueue* mCommandQueue;
        ID3D12CommandAllocator* mCommandAllocator;
        ID3D12GraphicsCommandList* mCommandList;

        // Current Frame
        UINT mCurrentBuffer;
        ID3D12DescriptorHeap* mRtvHeap;
        ID3D12Resource* mRenderTargets[2]; // TODO
        IDXGISwapChain3* mSwapchain;

        // Resources
        D3D12_VIEWPORT mViewport;
        D3D12_RECT mSurfaceSize;

        ID3D12Resource* mVertexBuffer;
        ID3D12Resource* mIndexBuffer;

        ID3D12Resource* mUniformBuffer;
        ID3D12DescriptorHeap* mUniformBufferHeap;
        UINT8* mMappedUniformBuffer;

        D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
        D3D12_INDEX_BUFFER_VIEW mIndexBufferView;

        UINT mRtvDescriptorSize;
        ID3D12RootSignature* mRootSignature;
        ID3D12PipelineState* mPipelineState;

        // Sync
        UINT mFrameIndex;
        HANDLE mFenceEvent;
        ID3D12Fence* mFence;
        UINT64 mFenceValue;
    };
    context ctx_{};

    using render_pool = resource_pool<buffer, texture, shader, pipeline, frame>;
    render_pool pool_{};

public:
    backend_d3d12(const render_desc& desc) : 
        render_backend(desc)
    {

    }
    ~backend_d3d12()
    {
    }

    buffer_id   create(const   buffer_desc& desc) override
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
    virtual   shader_id create(const   shader_desc& desc) override
    {
        auto pid = pool_.insert(desc);
        shader& buf = pool_[pid];



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
        frame& buf = pool_[pid];



        return pid;
    }

    virtual void update(buffer_id id, const void* data, int size) override {}
    virtual void update(texture_id id, const void* data) override {}

    virtual void destroy(buffer_id id) override {}
    virtual void destroy(texture_id id) override {}
    virtual void destroy(shader_id id) override {}
    virtual void destroy(pipeline_id id) override {}
    virtual void destroy(frame_id id) override {}

    // render

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

    virtual void draw(uint count, uint first, uint instance_count) override {}
    virtual void draw_index(uint count, uint first, uint instance_count) override {}

protected:
    // low level api, mainly for d3d12, vulkan and metal
    virtual void acquire_next_image() override {}

protected:
    // interface provided for render_device
    virtual render_desc_pool& get_render_desc_pool() override { return pool_; }

}; // class backend_d3d12

} // namespace fay::d3d12



namespace fay
{

render_backend_ptr create_backend_d3d12(const render_desc& desc)
{
    return std::make_unique<fay::d3d12::backend_d3d12>(desc);
}

} // namespace fay

#endif // FAY_IN_WINDOWS