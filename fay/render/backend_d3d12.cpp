#include <unordered_set>
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
        // TODO: *((buffer_desc*)this) = desc

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

    uniform_cbv,              // CBV | VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
    
    texture_srv,              // SRV | VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
    texture_uav,              // UAV | VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
    
    storage_buffer_srv,       // SRV | VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
    storage_buffer_uav,       // UAV | VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
    
    uniform_texel_buffer_srv, // SRV | VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
    storage_texel_buffer_uav, // UAV | VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
};

struct descriptor_table
{
    descriptor_type type{};
    uint32_t base_binding{}; // base shader register binding point, like b0/t0...
    uint32_t count{}; // b0~bn/t0~tn
    //shader_stage shader_stages;

    // std::vector<size_t> render_ids;
    uint32_t heap_offset{};
    uint32_t root_parameter_index{};
};

// material_data(shader, shader_params, resource) 
// shader_params
// static_respack, dynamic_respack
struct respack
{
    ID3D12RootSignaturePtr root_signature{}; // resource_signature, respack_layout

    ID3D12DescriptorHeapPtr sampler_heap{};
    ID3D12DescriptorHeapPtr cbvsrvuav_heap{};

    descriptor_table sampler_table{};
    descriptor_table uniform_cbv_table{};
    descriptor_table texture_srv_table{};

    std::vector<descriptor_table*> active_tables{};

    respack() {}
    respack(const respack_desc& desc)
    {
        // TODO: base_binding
        sampler_table     = { descriptor_type::sampler,     0, (uint32_t)desc.textures.size() };
        uniform_cbv_table = { descriptor_type::uniform_cbv, 0, (uint32_t)desc.uniforms.size() };
        texture_srv_table = { descriptor_type::texture_srv, 0, (uint32_t)desc.textures.size() };

        descriptor_table* raw_tables[] =
        {
            &sampler_table,
            &uniform_cbv_table,
            &texture_srv_table,
        };
        for (auto table : raw_tables)
        {
            if (table->count > 0)
            {
                active_tables.emplace_back(table);
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
    string entry;
    string name;
    string target;

    ID3DBlobPtr blob_ptr{};
    D3D12_SHADER_BYTECODE bytecode{};

    // DXC
    IDxcBlobPtr dxc_blob_ptr{};
};

/// Storage for DXIL libraries and their exported symbols
struct shader_lib
{
    shader_lib(IDxcBlobPtr dxil, const std::vector<std::wstring>& exportedSymbols) :
        m_dxil(dxil), m_exportedSymbols(exportedSymbols), m_exports(exportedSymbols.size())
    {
        // Create one export descriptor per symbol
        for (size_t i = 0; i < m_exportedSymbols.size(); i++)
        {
            m_exports[i] = {};
            m_exports[i].Name = m_exportedSymbols[i].c_str();
            m_exports[i].ExportToRename = nullptr;
            m_exports[i].Flags = D3D12_EXPORT_FLAG_NONE;
        }

        // Create a library descriptor combining the DXIL code and the export names
        m_libDesc.DXILLibrary.BytecodeLength = dxil->GetBufferSize();
        m_libDesc.DXILLibrary.pShaderBytecode = dxil->GetBufferPointer();
        m_libDesc.NumExports = static_cast<UINT>(m_exportedSymbols.size());
        m_libDesc.pExports = m_exports.data();
    }
    shader_lib(const shader_lib& source) : shader_lib(source.m_dxil, source.m_exportedSymbols)
    {
    }
    shader_lib& operator=(const shader_lib& source)
    {
        shader_lib tmp(source);
        std::swap(*this, tmp);

        return *this;
    }

    IDxcBlob* m_dxil;
    const std::vector<std::wstring> m_exportedSymbols;

    std::vector<D3D12_EXPORT_DESC> m_exports;
    D3D12_DXIL_LIBRARY_DESC m_libDesc;
};

struct shader //: public shader_desc // shader_program
{
    //using shader_desc::shader_desc;

    shader_unit vs{};
    shader_unit hs{};
    shader_unit ds{};
    shader_unit gs{};
    shader_unit ps{};
    shader_unit cs{};

    shader_unit ray_gen{};
    shader_unit ray_miss{};
    shader_unit ray_intersect{};
    shader_unit ray_any_hit{};
    shader_unit ray_hit{};

    //std::vector<respack_layout> respack_layouts; // TODO: cache them

    std::vector<shader_unit*> active_units{};
    std::vector<shader_lib> raytracing_libs{};

    shader() {}
    shader(const shader_desc& desc) //: shader_desc(desc)
    {
        vs = { shader_stage::vertex,   desc.vs, "vs", };
        hs = { shader_stage::hull,     desc.hs, "hs", };
        ds = { shader_stage::domn,     desc.ds, "ds", };
        gs = { shader_stage::geometry, desc.gs, "gs", };
        ps = { shader_stage::fragment, desc.fs, "ps", };

        cs = { shader_stage::compute,  desc.compute, "cs", };

        ray_gen       = { shader_stage::ray_gen, desc.ray_gen, "ray_gen", };
        ray_miss      = { shader_stage::ray_miss, desc.ray_miss, "ray_miss", };
        ray_intersect = { shader_stage::ray_intersect, desc.ray_intersect, "ray_intersect", };
        ray_any_hit   = { shader_stage::ray_any_hit, desc.ray_any_hit, "ray_any_hit", };
        ray_hit       = { shader_stage::ray_hit, desc.ray_hit, "ray_hit", };

        shader_unit* raw_stages[]
        { 
            &vs, &hs, &ds, &gs, &ps, 
            &cs, 
            &ray_gen, &ray_miss, &ray_intersect, &ray_any_hit, &ray_hit 
        };
        for (auto& stage : raw_stages)
        {
            if (!stage->source.empty())
            {
                // https://github.com/Microsoft/DirectXShaderCompiler/wiki/Shader-Model
                stage->target = stage->target + "_5_1";
                stage->name   = desc.name + "_" + stage->target;

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

inline namespace generator
{


class RootSignatureGenerator
{
public:
    /// Add a set of heap range descriptors as a parameter of the root signature.
    void AddHeapRangesParameter(const std::vector<D3D12_DESCRIPTOR_RANGE>& ranges)
    {
        m_ranges.push_back(ranges);

        // A set of ranges on the heap is a descriptor table parameter
        D3D12_ROOT_PARAMETER param = {};
        param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param.DescriptorTable.NumDescriptorRanges = static_cast<UINT>(ranges.size());
        // The range pointer is kept null here, and will be resolved when generating the root signature
        // (see explanation of m_rangeLocations below)
        param.DescriptorTable.pDescriptorRanges = nullptr;

        // All parameters (heap ranges and root parameters) are added to the same parameter list to
        // preserve order
        m_parameters.push_back(param);

        // The descriptor table descriptor ranges require a pointer to the descriptor ranges. Since new
        // ranges can be dynamically added in the vector, we separately store the index of the range set.
        // The actual address will be solved when generating the actual root signature
        m_rangeLocations.push_back(static_cast<UINT>(m_ranges.size() - 1));
    }

    /// Add a set of heap ranges as a parameter of the root signature. Each range
    /// is defined as follows:
    /// - UINT BaseShaderRegister: the first register index in the range, e.g. the
    /// register of a UAV with BaseShaderRegister==0 is defined in the HLSL code
    /// as register(u0)
    /// - UINT NumDescriptors: number of descriptors in the range. Those will be
    /// mapped to BaseShaderRegister, BaseShaderRegister+1 etc. UINT
    /// RegisterSpace: Allows using the same register numbers multiple times by
    /// specifying a space where they are defined, similarly to a namespace. For
    /// example, a UAV with BaseShaderRegister==0 and RegisterSpace==1 is accessed
    /// in HLSL using the syntax register(u0, space1)
    /// - D3D12_DESCRIPTOR_RANGE_TYPE RangeType: The range type, such as
    /// D3D12_DESCRIPTOR_RANGE_TYPE_CBV for a constant buffer
    /// - UINT OffsetInDescriptorsFromTableStart: The first slot in the heap
    /// corresponding to the buffers mapped by the root signature. This can either
    /// be explicit, or implicit using D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND. In
    /// this case the index in the heap is the one directly following the last
    /// parameter range (or 0 if it's the first)
    void AddHeapRangesParameter(std::vector<std::tuple<UINT,                        // BaseShaderRegister,
        UINT,                        // NumDescriptors
        UINT,                        // RegisterSpace
        D3D12_DESCRIPTOR_RANGE_TYPE, // RangeType
        UINT                         // OffsetInDescriptorsFromTableStart
        >>
        ranges)
    {
        // Build and store the set of descriptors for the ranges
        std::vector<D3D12_DESCRIPTOR_RANGE> rangeStorage;
        for (const auto& input : ranges)
        {
            D3D12_DESCRIPTOR_RANGE r = {};
            r.BaseShaderRegister = std::get<RSC_BASE_SHADER_REGISTER>(input);
            r.NumDescriptors = std::get<RSC_NUM_DESCRIPTORS>(input);
            r.RegisterSpace = std::get<RSC_REGISTER_SPACE>(input);
            r.RangeType = std::get<RSC_RANGE_TYPE>(input);
            r.OffsetInDescriptorsFromTableStart = std::get<RSC_OFFSET_IN_DESCRIPTORS_FROM_TABLE_START>(input);
            rangeStorage.push_back(r);
        }

        // Add those ranges to the heap parameters
        AddHeapRangesParameter(rangeStorage);
    }

    /// Add a root parameter to the shader, defined by its type: constant buffer (CBV), shader
    /// resource (SRV), unordered access (UAV), or root constant (CBV, directly defined by its value
    /// instead of a buffer). The shaderRegister and registerSpace indicate how to access the
    /// parameter in the HLSL code, e.g a SRV with shaderRegister==1 and registerSpace==0 is
    /// accessible via register(t1, space0).
    /// In case of a root constant, the last parameter indicates how many successive 32-bit constants
    /// will be bound.
    void AddRootParameter(D3D12_ROOT_PARAMETER_TYPE type, UINT shaderRegister = 0, UINT registerSpace = 0,
        UINT numRootConstants = 1)
    {
        D3D12_ROOT_PARAMETER param = {};
        param.ParameterType = type;
        // The descriptor is an union, so specific values need to be set in case the parameter is a
        // constant instead of a buffer.
        if (type == D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS)
        {
            param.Constants.Num32BitValues = numRootConstants;
            param.Constants.RegisterSpace = registerSpace;
            param.Constants.ShaderRegister = shaderRegister;
        }
        else
        {
            param.Descriptor.RegisterSpace = registerSpace;
            param.Descriptor.ShaderRegister = shaderRegister;
        }

        // We default the visibility to all shaders
        param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        // Add the root parameter to the set of parameters,
        m_parameters.push_back(param);
        // and indicate that there will be no range
        // location to indicate since this parameter is not part of the heap
        m_rangeLocations.push_back(~0u);
    }

    /// Create the root signature from the set of parameters, in the order of the addition calls
    ID3D12RootSignature* Generate(ID3D12Device* device, bool isLocal)
    {
        // Go through all the parameters, and set the actual addresses of the heap range descriptors based
        // on their indices in the range set array
        for (size_t i = 0; i < m_parameters.size(); i++)
        {
            if (m_parameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
            {
                m_parameters[i].DescriptorTable.pDescriptorRanges = m_ranges[m_rangeLocations[i]].data();
            }
        }
        // Specify the root signature with its set of parameters
        D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
        rootDesc.NumParameters = static_cast<UINT>(m_parameters.size());
        rootDesc.pParameters = m_parameters.data();
        // Set the flags of the signature. By default root signatures are global, for example for vertex
        // and pixel shaders. For raytracing shaders the root signatures are local.
        rootDesc.Flags = isLocal ? D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE : D3D12_ROOT_SIGNATURE_FLAG_NONE;

        // Create the root signature from its descriptor
        ID3DBlob* pSigBlob;
        ID3DBlob* pErrorBlob;
        HRESULT hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &pSigBlob, &pErrorBlob);
        if (FAILED(hr))
        {
            throw std::logic_error("Cannot serialize root signature");
        }
        ID3D12RootSignature* pRootSig;
        hr = device->CreateRootSignature(0, pSigBlob->GetBufferPointer(), pSigBlob->GetBufferSize(),
            IID_PPV_ARGS(&pRootSig));
        if (FAILED(hr))
        {
            throw std::logic_error("Cannot create root signature");
        }
        return pRootSig;
    }

private:
    /// Heap range descriptors
    std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> m_ranges;
    /// Root parameter descriptors
    std::vector<D3D12_ROOT_PARAMETER> m_parameters;

    /// For each entry of m_parameter, indicate the index of the range array in m_ranges, and ~0u if
    /// the parameter is not a heap range descriptor
    std::vector<UINT> m_rangeLocations;

    enum
    {
        RSC_BASE_SHADER_REGISTER = 0,
        RSC_NUM_DESCRIPTORS = 1,
        RSC_REGISTER_SPACE = 2,
        RSC_RANGE_TYPE = 3,
        RSC_OFFSET_IN_DESCRIPTORS_FROM_TABLE_START = 4
    };
};

/// Helper class to create and maintain a Shader Binding Table
class ShaderBindingTable
{
public:
    /// Add a ray generation program by name, with its list of data pointers or values according to
    /// the layout of its root signature
    void AddRayGenerationProgram(const std::wstring& entryPoint, const std::vector<void*>& inputData) {
        m_rayGen.emplace_back(SBTEntry(entryPoint, inputData));
    }

    /// Add a miss program by name, with its list of data pointers or values according to
    /// the layout of its root signature
    void AddMissProgram(const std::wstring& entryPoint, const std::vector<void*>& inputData) {
        m_miss.emplace_back(SBTEntry(entryPoint, inputData));
    }

    /// Add a hit group by name, with its list of data pointers or values according to
    /// the layout of its root signature
    void AddHitGroup(const std::wstring& entryPoint, const std::vector<void*>& inputData) {
        m_hitGroup.emplace_back(SBTEntry(entryPoint, inputData));
    }

    /// Compute the size of the SBT based on the set of programs and hit groups it contains
    uint32_t ComputeSBTSize()
    {
        // Size of a program identifier
        m_progIdSize = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;
        // Compute the entry size of each program type depending on the maximum number of parameters in
        // each category
        m_rayGenEntrySize = GetEntrySize(m_rayGen);
        m_missEntrySize = GetEntrySize(m_miss);
        m_hitGroupEntrySize = GetEntrySize(m_hitGroup);

        // The total SBT size is the sum of the entries for ray generation, miss and hit groups, aligned
        // on 256 bytes
        uint32_t sbtSize = round_up(m_rayGenEntrySize * static_cast<UINT>(m_rayGen.size()) +
            m_missEntrySize * static_cast<UINT>(m_miss.size()) +
            m_hitGroupEntrySize * static_cast<UINT>(m_hitGroup.size()),
            256);
        return sbtSize;
    }

    /// Build the SBT and store it into sbtBuffer, which has to be pre-allocated on the upload heap.
    /// Access to the raytracing pipeline object is required to fetch program identifiers using their
    /// names
    void Generate(ID3D12Resource* sbtBuffer, ID3D12StateObjectProperties* raytracingPipeline)
    {
        // Map the SBT
        uint8_t* pData;
        HRESULT hr = sbtBuffer->Map(0, nullptr, reinterpret_cast<void**>(&pData));
        if (FAILED(hr))
        {
            throw std::logic_error("Could not map the shader binding table");
        }
        // Copy the shader identifiers followed by their resource pointers or root constants: first the
        // ray generation, then the miss shaders, and finally the set of hit groups
        uint32_t offset = 0;

        offset = CopyShaderData(raytracingPipeline, pData, m_rayGen, m_rayGenEntrySize);
        pData += offset;

        offset = CopyShaderData(raytracingPipeline, pData, m_miss, m_missEntrySize);
        pData += offset;

        offset = CopyShaderData(raytracingPipeline, pData, m_hitGroup, m_hitGroupEntrySize);

        // Unmap the SBT
        sbtBuffer->Unmap(0, nullptr);
    }

    /// Reset the sets of programs and hit groups
    void Reset()
    {
        m_rayGen.clear();
        m_miss.clear();
        m_hitGroup.clear();

        m_rayGenEntrySize = 0;
        m_missEntrySize = 0;
        m_hitGroupEntrySize = 0;
        m_progIdSize = 0;
    }

    /// The following getters are used to simplify the call to DispatchRays where the offsets of the
    /// shader programs must be exactly following the SBT layout

    /// Get the size in bytes of the SBT section dedicated to ray generation programs
    UINT GetRayGenSectionSize() const {
        return m_rayGenEntrySize * static_cast<UINT>(m_rayGen.size());
    }
    /// Get the size in bytes of one ray generation program entry in the SBT
    UINT GetRayGenEntrySize() const { return m_rayGenEntrySize; }

    /// Get the size in bytes of the SBT section dedicated to miss programs
    UINT GetMissSectionSize() const {
        return m_missEntrySize * static_cast<UINT>(m_miss.size());
    }

    /// Get the size in bytes of one miss program entry in the SBT
    UINT GetMissEntrySize() { return m_missEntrySize; }

    /// Get the size in bytes of the SBT section dedicated to hit groups
    UINT GetHitGroupSectionSize() const {
        return m_hitGroupEntrySize * static_cast<UINT>(m_hitGroup.size());
    }
    /// Get the size in bytes of hit group entry in the SBT
    UINT GetHitGroupEntrySize() const { return m_hitGroupEntrySize; }

private:
    /// Wrapper for SBT entries, each consisting of the name of the program and a list of values,
    /// which can be either pointers or raw 32-bit constants
    struct SBTEntry
    {
        SBTEntry(std::wstring entryPoint, std::vector<void*> inputData) :
            m_entryPoint(std::move(entryPoint)), m_inputData(std::move(inputData))
        {
        }

        const std::wstring m_entryPoint;
        const std::vector<void*> m_inputData;
    };

    /// For each entry, copy the shader identifier followed by its resource pointers and/or root
    /// constants in outputData, with a stride in bytes of entrySize, and returns the size in bytes
    /// actually written to outputData.
    uint32_t CopyShaderData(ID3D12StateObjectProperties* raytracingPipeline, uint8_t* outputData,
        const std::vector<SBTEntry>& shaders, uint32_t entrySize)
    {
        uint8_t* pData = outputData;
        for (const auto& shader : shaders)
        {
            // Get the shader identifier, and check whether that identifier is known
            void* id = raytracingPipeline->GetShaderIdentifier(shader.m_entryPoint.c_str());
            if (!id)
            {
                std::wstring errMsg(std::wstring(L"Unknown shader identifier used in the SBT: ") + shader.m_entryPoint);
                throw std::logic_error(std::string(errMsg.begin(), errMsg.end()));
            }
            // Copy the shader identifier
            memcpy(pData, id, m_progIdSize);
            // Copy all its resources pointers or values in bulk
            memcpy(pData + m_progIdSize, shader.m_inputData.data(), shader.m_inputData.size() * 8);

            pData += entrySize;
        }
        // Return the number of bytes actually written to the output buffer
        return static_cast<uint32_t>(shaders.size()) * entrySize;
    }

    /// Compute the size of the SBT entries for a set of entries, which is determined by the maximum
    /// number of parameters of their root signature
    uint32_t GetEntrySize(const std::vector<SBTEntry>& entries)
    {
        // Find the maximum number of parameters used by a single entry
        size_t maxArgs = 0;
        for (const auto& shader : entries)
        {
            maxArgs = max(maxArgs, shader.m_inputData.size());
        }
        // A SBT entry is made of a program ID and a set of parameters, taking 8 bytes each. Those
        // parameters can either be 8-bytes pointers, or 4-bytes constants
        uint32_t entrySize = m_progIdSize + 8 * static_cast<uint32_t>(maxArgs);

        // The entries of the shader binding table must be 16-bytes-aligned
        entrySize = round_up(entrySize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);

        return entrySize;
    }

    std::vector<SBTEntry> m_rayGen;
    std::vector<SBTEntry> m_miss;
    std::vector<SBTEntry> m_hitGroup;

    /// For each category, the size of an entry in the SBT depends on the maximum number of resources
    /// used by the shaders in that category.The helper computes those values automatically in
    /// GetEntrySize()
    uint32_t m_rayGenEntrySize;
    uint32_t m_missEntrySize;
    uint32_t m_hitGroupEntrySize;

    /// The program names are translated into program identifiers.The size in bytes of an identifier
    /// is provided by the device and is the same for all categories.
    UINT m_progIdSize;
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
        ID3D12Device5Ptr mDevice;
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
        ID3D12GraphicsCommandList4Ptr mCommandList{};
        ID3D12GraphicsCommandList4Ptr mFrameCommandList[SwapChainBufferCount]{};
        respack_id res_id{};
    };
    context ctx_{};
    ID3D12Device5Ptr device_{};
    ID3D12GraphicsCommandList4Ptr list_{};

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
    command_list_context cmd_{}; // TODO: rename

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
        device_ = ctx_.mDevice;


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

        // enable
        enable_raytracing();
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

    virtual bool enable_raytracing()
    {
        D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
        ThrowIfFailed(device_->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5,
            &options5, sizeof(options5)));
        if (options5.RaytracingTier < D3D12_RAYTRACING_TIER_1_0)
        {
            throw std::runtime_error("Raytracing not supported on device");
            return false;
        }

        return true;
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

#pragma region create (resource, state), update and destroy
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

            if (buf_desc.data)
            {
                std::memcpy(buffer.mapped_address, buf_desc.data, buf_desc.btsz);
            }
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
                buffer.cbv_view_desc.SizeInBytes = buffer.btsz;
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



        // create texture
        {
            D3D12_HEAP_PROPERTIES heap_props = {};
            heap_props.Type = D3D12_HEAP_TYPE_DEFAULT;
            heap_props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            heap_props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            heap_props.CreationNodeMask = 1;
            heap_props.VisibleNodeMask = 1;

            D3D12_HEAP_FLAGS heap_flags = D3D12_HEAP_FLAG_NONE;



            D3D12_RESOURCE_DESC res_desc = {};
            res_desc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(texture_type_map.at(tex_desc.type).d3d12);
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
            if (enum_have(tex_desc.usage_, texture_usage_::storage_texture))
            {
                res_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            }

            D3D12_RESOURCE_STATES res_states = 
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
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



            D3D12_CHECK2(ctx_.mDevice->CreateCommittedResource(
                &heap_props, heap_flags, &res_desc, res_states, p_clear_value,
                IID_PPV_ARGS(&tex.resource)), tex.resource);
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


            tex.srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
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

        // create sampler desc
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

        // update texture
        if (tex_desc.data.size() > 0)
        {
            update(pid, tex_desc, tex);
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

            D3D12_CHECK2(ctx_.mDevice->CreateDescriptorHeap(
                &desc, IID_PPV_ARGS(&res.sampler_heap)), res.sampler_heap);

            res.sampler_table.heap_offset = 0;
        }
        // TODO
        UINT cbvsrvuav_count = res_desc.uniforms.size() + res_desc.textures.size();
        if (cbvsrvuav_count > 0)
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.NumDescriptors = cbvsrvuav_count;
            desc.NodeMask = 0;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

            D3D12_CHECK2(ctx_.mDevice->CreateDescriptorHeap(
                &desc, IID_PPV_ARGS(&res.cbvsrvuav_heap)), res.cbvsrvuav_heap);

            // TODO
            res.uniform_cbv_table.heap_offset = 0;
            res.texture_srv_table.heap_offset = res_desc.uniforms.size();
        }



        //update(pid, res_desc);
        // create cbv
        if (res_desc.uniforms.size() > 0)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE handle =
                res.cbvsrvuav_heap->GetCPUDescriptorHandleForHeapStart();
            UINT handle_inc_size = ctx_.mDevice->GetDescriptorHandleIncrementSize(
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            handle.ptr += res.uniform_cbv_table.heap_offset * handle_inc_size;

            for (uint32_t i = 0; i < res_desc.uniforms.size(); ++i)
            {
                auto buf_id = res_desc.uniforms[i];
                const auto& buf = pool_[buf_id];

                ctx_.mDevice->CreateConstantBufferView(&buf.cbv_view_desc, handle);
                handle.ptr += handle_inc_size;
            }
        }
        // create sampler/srv
        if (res_desc.textures.size() > 0)
        {
            {
                D3D12_CPU_DESCRIPTOR_HANDLE handle =
                    res.sampler_heap->GetCPUDescriptorHandleForHeapStart();
                UINT handle_inc_size = ctx_.mDevice->GetDescriptorHandleIncrementSize(
                    D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
                handle.ptr += res.sampler_table.heap_offset * handle_inc_size;

                for (uint32_t i = 0; i < res_desc.textures.size(); ++i)
                {
                    auto tex_id = res_desc.textures[i];
                    const auto& tex = pool_[tex_id];

                    ctx_.mDevice->CreateSampler(&tex.sampler_desc, handle);
                    handle.ptr += handle_inc_size;
                }
            }

            {
                D3D12_CPU_DESCRIPTOR_HANDLE handle =
                    res.cbvsrvuav_heap->GetCPUDescriptorHandleForHeapStart();
                UINT handle_inc_size = ctx_.mDevice->GetDescriptorHandleIncrementSize(
                    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                handle.ptr += res.texture_srv_table.heap_offset * handle_inc_size;

                for (uint32_t i = 0; i < res_desc.textures.size(); ++i)
                {
                    auto tex_id = res_desc.textures[i];
                    const auto& tex = pool_[tex_id];

                    ctx_.mDevice->CreateShaderResourceView(tex.resource, &tex.srv_desc, handle);
                    handle.ptr += handle_inc_size;
                }
            }
        }
        


        create_shader_input_layout(res);


        return pid;
    }
    virtual   shader_id create2(const   shader_desc& shd_desc)// override
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
    virtual   shader_id create(const   shader_desc& shd_desc) override
    {
        auto pid = pool_.insert(shd_desc);
        shader& shd = pool_[pid];

        for (auto unit_ptr : shd.active_units)
        {
            auto& unit = *unit_ptr;

            unit.dxc_blob_ptr = CompileShaderLibrary(unit.name, unit.source, unit.entry);

            shd.raytracing_libs.emplace_back(shader_lib{ unit.dxc_blob_ptr, { std::wstring((LPCWSTR)unit.entry.c_str()) } });
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

    virtual void update(buffer_id  id, const void* data, int size) override
    {
        const auto& buf = pool_[id];
        std::memcpy(buf.mapped_address, data, buf.btsz);
    }
    virtual void update(texture_id id, const void* data) override {}

    virtual void update(texture_id id, const texture_desc& update_desc, texture& tex)
    {
        int mipmaps = update_desc.mipmaps;
        FAY_DCHECK(mipmaps == 1);


        std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> subres_layouts(mipmaps);
        std::vector<UINT> subres_row_counts(mipmaps);
        std::vector<UINT64> subres_row_strides(mipmaps);
        UINT64 uploadBufferSize = 0;

        D3D12_RESOURCE_DESC stDestDesc = tex.resource->GetDesc();
        device_->GetCopyableFootprints(
            &stDestDesc, 0, mipmaps, 0,
            subres_layouts.data(), subres_row_counts.data(), subres_row_strides.data(),
            &uploadBufferSize);


        // TODO: create_image_mipmap



        buffer_desc upload_desc;
        upload_desc.type = buffer_type::transfer_src_;
        upload_desc.data = update_desc.data[0]; // TODO
        upload_desc.btsz = uploadBufferSize;
        auto upload_id = create(upload_desc);
        auto upload_buffer = pool_[upload_id];


        // TODO
        begin_cmdlist();
        transition_texture(tex, texture_usage_::sampled_texture, texture_usage_::transfer_dst_);
        for (uint32_t mip_level = 0; mip_level < mipmaps; ++mip_level)
        {
            D3D12_TEXTURE_COPY_LOCATION src = {};
            src.pResource = upload_buffer.resource;
            src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            src.PlacedFootprint = subres_layouts[mip_level];
            
            D3D12_TEXTURE_COPY_LOCATION dst = {};
            dst.pResource = tex.resource;
            dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            dst.SubresourceIndex = mip_level;

            ctx_.mCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, NULL);
        }
        transition_texture(tex, texture_usage_::transfer_dst_, texture_usage_::sampled_texture);
        end_cmdlist();
        queue_submit();
        queue_wait_idle();
    }

    // directly create a new respack???
    virtual void update(respack_id id, const respack_desc& update_desc) override
    {
        /*
        const auto& origin_desc = pool_.desc(id);
        respack& res = pool_[id];



        for (uint32_t i = 0; i < res.active_tables.size(); ++i)
        {
            descriptor_table* descriptor_table = res.active_tables[i];

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

                case descriptor_type::uniform_cbv:
                {
                    D3D12_CPU_DESCRIPTOR_HANDLE handle =
                        res.cbvsrvuav_heap->GetCPUDescriptorHandleForHeapStart();
                    UINT handle_inc_size = ctx_.mDevice->GetDescriptorHandleIncrementSize(
                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                    handle.ptr += descriptor_table->heap_offset * handle_inc_size;

                    for (uint32_t i = 0; i < descriptor_table->count; ++i)
                    {
                        ID3D12Resource* resource = descriptor_table->uniforms[i]->dx_resource;
                        D3D12_CONSTANT_BUFFER_VIEW_DESC* view_desc =
                            &(descriptor_table->uniforms[i]->dx_cbv_view_desc);
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
        }
        */
    }

    virtual void destroy(  buffer_id id) override {}
    virtual void destroy( texture_id id) override {}
    virtual void destroy(  shader_id id) override {}
    virtual void destroy(pipeline_id id) override {}
    virtual void destroy(   frame_id id) override {}

private:
    // create root signature
    // TODO: repalce res with shd
    void create_shader_input_layout(respack& res)
    {
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

                case descriptor_type::uniform_cbv:
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
                range_11->BaseShaderRegister = descriptor->base_binding;
                range_11->NumDescriptors = descriptor->count;
                range_11->RegisterSpace = 0;
                range_11->Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
                range_11->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

                range_10->BaseShaderRegister = descriptor->base_binding;
                range_10->NumDescriptors = descriptor->count;
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

            D3D12_CHECK(D3D12SerializeVersionedRootSignature(&desc, &sig_blob, &error_msgs));
        }
        else if (feature_data.HighestVersion == D3D_ROOT_SIGNATURE_VERSION_1_0)
        {
            desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_0;
            desc.Desc_1_0.NumParameters = parameter_count;
            desc.Desc_1_0.pParameters = parameters_10.data();
            desc.Desc_1_0.NumStaticSamplers = 0;
            desc.Desc_1_0.pStaticSamplers = NULL;
            desc.Desc_1_0.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

            D3D12_CHECK(D3D12SerializeRootSignature(&(desc.Desc_1_0), D3D_ROOT_SIGNATURE_VERSION_1_0,
                &sig_blob, &error_msgs));
        }
        D3D12_CHECK(ctx_.mDevice->CreateRootSignature(
            0, sig_blob->GetBufferPointer(), sig_blob->GetBufferSize(),
            IID_PPV_ARGS(&res.root_signature)), res.root_signature);
    }

    // TODO: id
    // TODO: remove buf and res, only need shd(Programmable), pipe(fixed) and frm(target)
    ID3D12PipelineStatePtr create_rasterization_state(
        const buffer& buf, const respack& res, 
        const shader& shd, const pipeline& pipe, const frame& frm)
    {
        D3D12_CACHED_PIPELINE_STATE cached_pso_desc = {};
        cached_pso_desc.pCachedBlob = NULL;
        cached_pso_desc.CachedBlobSizeInBytes = 0;

        D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_state_desc = {};
        pipeline_state_desc.pRootSignature = res.root_signature;
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
        pipeline_state_desc.pRootSignature = res.root_signature;
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

#pragma region create shader

#pragma endregion

#pragma region raytracing
    // https://developer.nvidia.com/rtx/raytracing/dxr/DX12-Raytracing-tutorial-Part-1

    static constexpr D3D12_HEAP_PROPERTIES kUploadHeapProps{ D3D12_HEAP_TYPE_UPLOAD, D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
                                                        D3D12_MEMORY_POOL_UNKNOWN, 0, 0 };

    // Specifies the default heap. This heap type experiences the most bandwidth for
    // the GPU, but cannot provide CPU access.
    static constexpr D3D12_HEAP_PROPERTIES kDefaultHeapProps{ D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
                                                             D3D12_MEMORY_POOL_UNKNOWN, 0, 0 };

    ID3D12Resource* CreateBuffer(uint64_t size, D3D12_RESOURCE_FLAGS flags,
        D3D12_RESOURCE_STATES initState, const D3D12_HEAP_PROPERTIES& heapProps)
    {
        D3D12_RESOURCE_DESC bufDesc = {};
        bufDesc.Alignment = 0;
        bufDesc.DepthOrArraySize = 1;
        bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufDesc.Flags = flags;
        bufDesc.Format = DXGI_FORMAT_UNKNOWN;
        bufDesc.Height = 1;
        bufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        bufDesc.MipLevels = 1;
        bufDesc.SampleDesc.Count = 1;
        bufDesc.SampleDesc.Quality = 0;
        bufDesc.Width = size;

        ID3D12Resource* pBuffer;
        ThrowIfFailed(device_->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufDesc, initState, nullptr,
            IID_PPV_ARGS(&pBuffer)));
        return pBuffer;
    }





    // bounds
    struct accel
    {
        ID3D12ResourcePtr pScratch;              // Scratch memory for AS builder
        ID3D12ResourcePtr pResult;            // Where the AS is

        ID3D12ResourcePtr pInstanceDesc;         // Hold the matrices of the instances
    };

    accel create_bottom_accel(const bottom_accel_desc& desc)
    {
        accel accel;



        // 

        const auto& shapes = desc.shapes;
        std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> descriptors(shapes.size());

        for (int i = 0; i < shapes.size(); ++i)
        {
            const auto& desc = shapes[i];
            const auto& vertex = pool_[desc.vertex];
            const auto& vertex_desc = pool_.desc(desc.vertex); // TODO
            const auto& index = pool_[desc.index];
            const auto& index_desc = pool_.desc(desc.index);

            int vertexOffsetInBytes = 0;
            int indexOffsetInBytes = 0;
            D3D12_RAYTRACING_GEOMETRY_DESC& descriptor = descriptors[i];
            descriptor.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;

            descriptor.Triangles.VertexBuffer.StartAddress = vertex.resource->GetGPUVirtualAddress() + vertexOffsetInBytes;
            descriptor.Triangles.VertexBuffer.StrideInBytes = vertex_desc.layout.stride();
            descriptor.Triangles.VertexCount = vertex_desc.size;
            descriptor.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT; // only position

            descriptor.Triangles.IndexBuffer = index.resource->GetGPUVirtualAddress() + indexOffsetInBytes;
            descriptor.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
            descriptor.Triangles.IndexCount = index_desc.size;
            descriptor.Triangles.Transform3x4 = 0;
            descriptor.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
        }



        // compute bottom accels bufer size

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;

        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS prebuildDesc;
        prebuildDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
        prebuildDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        prebuildDesc.NumDescs = static_cast<UINT>(descriptors.size());
        prebuildDesc.pGeometryDescs = descriptors.data();
        prebuildDesc.Flags = flags;

        // This structure is used to hold the sizes of the required scratch memory and
        // resulting AS
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};

        device_->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildDesc, &info);

        UINT64 scratchSizeInBytes = round_up(info.ScratchDataSizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
        accel.pScratch = CreateBuffer(scratchSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_COMMON, kDefaultHeapProps);

        UINT64 resultSizeInBytes = round_up(info.ResultDataMaxSizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
        accel.pResult = CreateBuffer(resultSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, kDefaultHeapProps);



        // Generate bottom accels

        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc;
        buildDesc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
        buildDesc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        buildDesc.Inputs.NumDescs = static_cast<UINT>(descriptors.size());
        buildDesc.Inputs.pGeometryDescs = descriptors.data();
        buildDesc.DestAccelerationStructureData = { accel.pResult->GetGPUVirtualAddress() };
        buildDesc.ScratchAccelerationStructureData = { accel.pScratch->GetGPUVirtualAddress() };
        buildDesc.SourceAccelerationStructureData = 0;
        buildDesc.Inputs.Flags = flags;
        list_->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

        D3D12_RESOURCE_BARRIER uavBarrier;
        uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        uavBarrier.UAV.pResource = accel.pResult;
        uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        list_->ResourceBarrier(1, &uavBarrier);



        return accel;
    }
    
    accel create_top_accel(const accel_desc& desc, const std::vector<accel>& bottoms)
    {
        accel accel;



        // 

        const auto& instances = desc.instances;

        struct accel_instance
        {
            /// Bottom-level AS
            ID3D12Resource* bottomLevelAS;
            /// Transform matrix
            render_matrix transform;
            /// Instance ID visible in the shader
            UINT instanceID;
            /// Hit group index used to fetch the shaders from the SBT
            UINT hitGroupIndex;
        };
        std::vector<accel_instance> accel_instances(instances.size());

        for (int i = 0; i < instances.size(); ++i)
        {
            const auto& instance = instances[i];
            accel_instances[i] = accel_instance{ bottoms[instance.bottom_index].pResult, instance.mat, 0, 1 };
        }



        // compute bottom accels bufer size

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS flags = desc.allowUpdate ? D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE
            : D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;

        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS prebuildDesc{};
        prebuildDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
        prebuildDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        prebuildDesc.NumDescs = static_cast<UINT>(instances.size());
        prebuildDesc.Flags = flags;
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};
        device_->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildDesc, &info);

        UINT64 scratchSizeInBytes = round_up(info.ScratchDataSizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
        UINT64 resultSizeInBytes = round_up(info.ResultDataMaxSizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
        UINT64 instanceDescsSizeInBytes = round_up(sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * static_cast<UINT64>(instances.size()), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

        accel.pScratch      = CreateBuffer(scratchSizeInBytes,       D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON, kDefaultHeapProps);
        accel.pResult       = CreateBuffer(resultSizeInBytes,        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, kDefaultHeapProps);
        accel.pInstanceDesc = CreateBuffer(instanceDescsSizeInBytes, D3D12_RESOURCE_FLAG_NONE,                   D3D12_RESOURCE_STATE_GENERIC_READ, kUploadHeapProps);
        ID3D12Resource* scratchBuffer = accel.pScratch;
        ID3D12Resource* resultBuffer = accel.pResult;
        ID3D12Resource* descriptorsBuffer = accel.pInstanceDesc;



        // Generate bottom accels

        // Copy the descriptors in the target descriptor buffer
        D3D12_RAYTRACING_INSTANCE_DESC* instanceDescs;
        descriptorsBuffer->Map(0, nullptr, reinterpret_cast<void**>(&instanceDescs));
        if (!instanceDescs)
        {
            throw std::logic_error("Cannot map the instance descriptor buffer - is it "
                "in the upload heap?");
        }

        auto instanceCount = static_cast<UINT>(instances.size());

        // Initialize the memory to zero on the first time only
        if (!desc.allowUpdate)
        {
            ZeroMemory(instanceDescs, instanceDescsSizeInBytes);
        }

        // Create the description for each instance
        for (uint32_t i = 0; i < instanceCount; i++)
        {
            // Instance ID visible in the shader in InstanceID()
            instanceDescs[i].InstanceID = accel_instances[i].instanceID;
            // Index of the hit group invoked upon intersection
            instanceDescs[i].InstanceContributionToHitGroupIndex = accel_instances[i].hitGroupIndex;
            // Instance flags, including backface culling, winding, etc - TODO: should
            // be accessible from outside
            instanceDescs[i].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
            memcpy(instanceDescs[i].Transform, &accel_instances[i].transform, sizeof(instanceDescs[i].Transform));
            // Get access to the bottom level
            instanceDescs[i].AccelerationStructure = accel_instances[i].bottomLevelAS->GetGPUVirtualAddress();
            // Visibility mask, always visible here - TODO: should be accessible from
            // outside
            instanceDescs[i].InstanceMask = 0xFF;
        }
        descriptorsBuffer->Unmap(0, nullptr);



        // If this in an update operation we need to provide the source buffer
        D3D12_GPU_VIRTUAL_ADDRESS pSourceAS = 0;

        if (flags == D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE && desc.allowUpdate)
        {
            flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
        }

        // Create a descriptor of the requested builder work, to generate a top-level
        // AS from the input parameters
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
        buildDesc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
        buildDesc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        buildDesc.Inputs.InstanceDescs = descriptorsBuffer->GetGPUVirtualAddress();
        buildDesc.Inputs.NumDescs = instanceCount;
        buildDesc.DestAccelerationStructureData = { resultBuffer->GetGPUVirtualAddress() };
        buildDesc.ScratchAccelerationStructureData = { scratchBuffer->GetGPUVirtualAddress() };
        buildDesc.SourceAccelerationStructureData = pSourceAS;
        buildDesc.Inputs.Flags = flags;

        // Build the top-level AS
        list_->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

        // Wait for the builder to complete by setting a barrier on the resulting
        // buffer. This can be important in case the rendering is triggered
        // immediately afterwards, without executing the command list
        D3D12_RESOURCE_BARRIER uavBarrier;
        uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        uavBarrier.UAV.pResource = resultBuffer;
        uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        list_->ResourceBarrier(1, &uavBarrier);



        return accel;
    }

    accel create_accel(const accel_desc& desc)
    {
        //begin();

        std::vector<accel> bottoms(desc.bottoms.size());
        for (int i = 0; i < desc.bottoms.size(); ++i)
        {
            bottoms[i] = create_bottom_accel(desc.bottoms[i]);
        }

        accel accel = create_top_accel(desc, bottoms);

        //end();

        return accel;
    }







    /// Storage for the hit groups, binding the hit group name with the underlying intersection, any
    /// hit and closest hit symbols
    struct hit_group
    {
        hit_group(std::wstring hitGroupName, std::wstring closestHitSymbol, std::wstring anyHitSymbol = L"",
            std::wstring intersectionSymbol = L"") :
            m_hitGroupName(std::move(hitGroupName)),
            m_closestHitSymbol(std::move(closestHitSymbol)), m_anyHitSymbol(std::move(anyHitSymbol)),
            m_intersectionSymbol(std::move(intersectionSymbol))
        {
            // Indicate which shader program is used for closest hit, leave the other
            // ones undefined (default behavior), export the name of the group
            m_desc.HitGroupExport = m_hitGroupName.c_str();
            m_desc.ClosestHitShaderImport = m_closestHitSymbol.empty() ? nullptr : m_closestHitSymbol.c_str();
            m_desc.AnyHitShaderImport = m_anyHitSymbol.empty() ? nullptr : m_anyHitSymbol.c_str();
            m_desc.IntersectionShaderImport = m_intersectionSymbol.empty() ? nullptr : m_intersectionSymbol.c_str();
        }

        /*
        
        hit_group(const hit_group& source) :
            HitGroup(source.m_hitGroupName, source.m_closestHitSymbol, source.m_anyHitSymbol, source.m_intersectionSymbol)
        {
        }
        */

        std::wstring m_hitGroupName;
        std::wstring m_closestHitSymbol;
        std::wstring m_anyHitSymbol;
        std::wstring m_intersectionSymbol;
        D3D12_HIT_GROUP_DESC m_desc = {};
    };

    /// Storage for the association between shaders and root signatures
    struct RootSignatureAssociation
    {
        RootSignatureAssociation(ID3D12RootSignature* rootSignature, const std::vector<std::wstring>& symbols) :
            m_rootSignature(rootSignature),
            m_symbols(symbols), m_symbolPointers(symbols.size())
        {
            for (size_t i = 0; i < m_symbols.size(); i++)
            {
                m_symbolPointers[i] = m_symbols[i].c_str();
            }
            m_rootSignaturePointer = m_rootSignature;
        }
        //RootSignatureAssociation(const RootSignatureAssociation& source);

        ID3D12RootSignature* m_rootSignature;
        ID3D12RootSignature* m_rootSignaturePointer;
        std::vector<std::wstring> m_symbols;
        std::vector<LPCWSTR> m_symbolPointers;
        D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION m_association = {};
    };

    ID3D12RootSignature* m_dummyLocalRootSignature;
    ID3D12RootSignature* m_dummyGlobalRootSignature;
    void CreateDummyRootSignatures()
    {
        // Creation of the global root signature
        D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
        rootDesc.NumParameters = 0;
        rootDesc.pParameters = nullptr;
        // A global root signature is the default, hence this flag
        rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

        HRESULT hr = 0;

        ID3DBlob* serializedRootSignature;
        ID3DBlob* error;

        // Create the empty global root signature
        hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSignature, &error);
        if (FAILED(hr))
        {
            throw std::logic_error("Could not serialize the global root signature");
        }
        hr = device_->CreateRootSignature(0, serializedRootSignature->GetBufferPointer(),
            serializedRootSignature->GetBufferSize(),
            IID_PPV_ARGS(&m_dummyGlobalRootSignature));

        serializedRootSignature->Release();
        if (FAILED(hr))
        {
            throw std::logic_error("Could not create the global root signature");
        }

        // Create the local root signature, reusing the same descriptor but altering the creation flag
        rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
        hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSignature, &error);
        if (FAILED(hr))
        {
            throw std::logic_error("Could not serialize the local root signature");
        }
        hr = device_->CreateRootSignature(0, serializedRootSignature->GetBufferPointer(),
            serializedRootSignature->GetBufferSize(),
            IID_PPV_ARGS(&m_dummyLocalRootSignature));

        serializedRootSignature->Release();
        if (FAILED(hr))
        {
            throw std::logic_error("Could not create the local root signature");
        }
    }


    ID3D12RootSignaturePtr CreateRayGenSignature()
    {
        RootSignatureGenerator rsc;
        rsc.AddHeapRangesParameter(
            { { 0 /*u0*/, 1 /*1 descriptor */, 0 /*use the implicit register space 0*/,
                D3D12_DESCRIPTOR_RANGE_TYPE_UAV /* UAV representing the output buffer*/,
                0 /*heap slot where the UAV is defined*/ },
              { 0 /*t0*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV /*Top-level acceleration structure*/, 1 } });

        return rsc.Generate(device_, true);
    }
    ID3D12RootSignaturePtr CreateHitSignature()
    {
        RootSignatureGenerator rsc;
        rsc.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_SRV);
        return rsc.Generate(device_, true);
    }
    ID3D12RootSignaturePtr CreateMissSignature()
    {
        RootSignatureGenerator rsc;
        return rsc.Generate(device_, true);
    }
    ID3D12RootSignaturePtr m_rayGenSignature;
    ID3D12RootSignaturePtr m_hitSignature;
    ID3D12RootSignaturePtr m_missSignature;


    // TODO: internal lambda
    // Compile a HLSL file by DXC into a DXIL library
    IDxcBlobPtr CompileShaderLibrary(const string& fileName, const string& source, const string& entry_point)
    {
        // TODO: static init
        static IDxcCompiler* pCompiler = nullptr;
        static IDxcLibrary* pLibrary = nullptr;
        static IDxcIncludeHandler* dxcIncludeHandler;

        HRESULT hr;

        // Initialize the DXC compiler and compiler helper
        if (!pCompiler)
        {
            ThrowIfFailed(DxcCreateInstance(CLSID_DxcCompiler, __uuidof(IDxcCompiler), (void**)&pCompiler));
            ThrowIfFailed(DxcCreateInstance(CLSID_DxcLibrary, __uuidof(IDxcLibrary), (void**)&pLibrary));
            ThrowIfFailed(pLibrary->CreateIncludeHandler(&dxcIncludeHandler));
        }



        // Create blob from the string
        IDxcBlobEncoding* pTextBlob;
        ThrowIfFailed(
            pLibrary->CreateBlobWithEncodingFromPinned((LPBYTE)source.c_str(), (uint32_t)source.size(), 0, &pTextBlob));

        // Compile
        IDxcOperationResult* pResult;
        ThrowIfFailed(
            pCompiler->Compile(pTextBlob, (LPCWSTR)fileName.c_str(), (LPCWSTR)entry_point.c_str(), L"lib_6_3", nullptr, 0, nullptr, 0, dxcIncludeHandler, &pResult));

        // error check
        HRESULT resultCode;
        ThrowIfFailed(pResult->GetStatus(&resultCode));
        if (FAILED(resultCode))
        {
            IDxcBlobEncoding* pError;
            hr = pResult->GetErrorBuffer(&pError);
            if (FAILED(hr))
            {
                throw std::logic_error("Failed to get shader compiler error");
            }

            // Convert error blob to a string
            std::vector<char> infoLog(pError->GetBufferSize() + 1);
            memcpy(infoLog.data(), pError->GetBufferPointer(), pError->GetBufferSize());
            infoLog[pError->GetBufferSize()] = 0;

            std::string errorMsg = "Shader Compiler Error:\n";
            errorMsg.append(infoLog.data());

            MessageBoxA(nullptr, errorMsg.c_str(), "Error!", MB_OK);
            throw std::logic_error("Failed compile shader");
        }

        IDxcBlob* pBlob;
        ThrowIfFailed(pResult->GetResult(&pBlob));
        return pBlob;
    }


    std::vector<shader_lib> m_libraries = {};
    std::vector<hit_group> m_hitGroups = {};
    std::vector<RootSignatureAssociation> m_rootSignatureAssociations = {};
    std::vector<std::wstring> BuildShaderExportList()
    {
        std::vector<std::wstring> exportedSymbols{};

        // Get all names from libraries
        // Get names associated to hit groups
        // Return list of libraries+hit group names - shaders in hit groups

        std::unordered_set<std::wstring> exports;

        // Add all the symbols exported by the libraries
        for (const shader_lib& lib : m_libraries)
        {
            for (const auto& exportName : lib.m_exportedSymbols)
            {
#ifdef _DEBUG
                // Sanity check in debug mode: check that no name is exported more than once
                if (exports.find(exportName) != exports.end())
                {
                    throw std::logic_error("Multiple definition of a symbol in the imported DXIL libraries");
                }
#endif
                exports.insert(exportName);
            }
        }

#ifdef _DEBUG
        // Sanity check in debug mode: verify that the hit groups do not reference an unknown shader name
        std::unordered_set<std::wstring> all_exports = exports;

        for (const auto& hitGroup : m_hitGroups)
        {
            if (!hitGroup.m_anyHitSymbol.empty() && exports.find(hitGroup.m_anyHitSymbol) == exports.end())
            {
                throw std::logic_error("Any hit symbol not found in the imported DXIL libraries");
            }

            if (!hitGroup.m_closestHitSymbol.empty() && exports.find(hitGroup.m_closestHitSymbol) == exports.end())
            {
                throw std::logic_error("Closest hit symbol not found in the imported DXIL libraries");
            }

            if (!hitGroup.m_intersectionSymbol.empty() && exports.find(hitGroup.m_intersectionSymbol) == exports.end())
            {
                throw std::logic_error("Intersection symbol not found in the imported DXIL libraries");
            }

            all_exports.insert(hitGroup.m_hitGroupName);
        }

        // Sanity check in debug mode: verify that the root signature associations do not reference an
        // unknown shader or hit group name
        for (const auto& assoc : m_rootSignatureAssociations)
        {
            for (const auto& symb : assoc.m_symbols)
            {
                if (!symb.empty() && all_exports.find(symb) == all_exports.end())
                {
                    throw std::logic_error("Root association symbol not found in the "
                        "imported DXIL libraries and hit group names");
                }
            }
        }
#endif

        // Go through all hit groups and remove the symbols corresponding to intersection, any hit and
        // closest hit shaders from the symbol set
        for (const auto& hitGroup : m_hitGroups)
        {
            if (!hitGroup.m_anyHitSymbol.empty())
            {
                exports.erase(hitGroup.m_anyHitSymbol);
            }
            if (!hitGroup.m_closestHitSymbol.empty())
            {
                exports.erase(hitGroup.m_closestHitSymbol);
            }
            if (!hitGroup.m_intersectionSymbol.empty())
            {
                exports.erase(hitGroup.m_intersectionSymbol);
            }
            exports.insert(hitGroup.m_hitGroupName);
        }

        // Finally build a vector containing ray generation and miss shaders, plus the hit group names
        for (const auto& name : exports)
        {
            exportedSymbols.push_back(name);
        }

        return exportedSymbols;
    }


    ID3D12StateObjectPtr m_rtStateObject;
    // Ray tracing pipeline state properties, retaining the shader identifiers
    // to use in the Shader Binding Table
    ID3D12StateObjectPropertiesPtr m_rtStateObjectProps;
    // TODO
    ID3D12StateObjectPtr create_raytracing_state(/*respack& ray_res, const shader& ray_shd, const pipeline& ray_pipe, const frame& ray_frm*/)
    {
        CreateDummyRootSignatures();
        // CreateRayRootSignature()

        m_rayGenSignature = CreateRayGenSignature();
        m_missSignature = CreateMissSignature();
        m_hitSignature = CreateHitSignature();

        m_hitGroups.emplace_back(hit_group(L"HitGroup", L"ray_git"));

        m_rootSignatureAssociations.emplace_back(RootSignatureAssociation(m_rayGenSignature, { L"ray_gen" }));
        m_rootSignatureAssociations.emplace_back(RootSignatureAssociation(m_missSignature, { L"ray_miss" }));
        m_rootSignatureAssociations.emplace_back(RootSignatureAssociation(m_hitSignature, { L"HitGroup" }));



        //create_shader_input_layout(ray_res);



        UINT m_maxPayLoadSizeInBytes = 4 * sizeof(float); // RGB + distance
        /// Attribute size, initialized to 2 for the barycentric coordinates used by the built-in triangle
        /// intersection shader
        UINT m_maxAttributeSizeInBytes = 2 * sizeof(float); // barycentric coordinates
        /// Maximum recursion depth, initialized to 1 to at least allow tracing primary rays
        UINT m_maxRecursionDepth = 1;



        // The pipeline is made of a set of sub-objects, representing the DXIL libraries, hit group
        // declarations, root signature associations, plus some configuration objects
        UINT64 subobjectCount = 
            m_libraries.size() +                     // DXIL libraries
            m_hitGroups.size() +                     // Hit group declarations
            1 +                                      // Shader configuration
            1 +                                      // Shader payload
            2 * m_rootSignatureAssociations.size() + // Root signature declaration + association
            2 +                                      // Empty global and local root signatures
            1;                                       // Final pipeline subobject

        // Initialize a vector with the target object count. It is necessary to make the allocation before
        // adding subobjects as some subobjects reference other subobjects by pointer. Using push_back may
        // reallocate the array and invalidate those pointers.
        std::vector<D3D12_STATE_SUBOBJECT> subobjects(subobjectCount);



        UINT currentIndex = 0;

        // 1. Add all the DXIL libraries
        {
            for (const shader_lib& lib : m_libraries)
            {
                D3D12_STATE_SUBOBJECT libSubobject = {};
                libSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
                libSubobject.pDesc = &lib.m_libDesc;

                subobjects[currentIndex++] = libSubobject;
            }
        }

        // 2. Add all the hit group declarations
        {
            for (const hit_group& group : m_hitGroups)
            {
                D3D12_STATE_SUBOBJECT hitGroup = {};
                hitGroup.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
                hitGroup.pDesc = &group.m_desc;

                subobjects[currentIndex++] = hitGroup;
            }
        }

        // 3. Add a subobject for the shader payload configuration
        {
            D3D12_RAYTRACING_SHADER_CONFIG shaderDesc = {};
            shaderDesc.MaxPayloadSizeInBytes = m_maxPayLoadSizeInBytes;
            shaderDesc.MaxAttributeSizeInBytes = m_maxAttributeSizeInBytes;

            D3D12_STATE_SUBOBJECT shaderConfigObject = {};
            shaderConfigObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
            shaderConfigObject.pDesc = &shaderDesc;

            subobjects[currentIndex++] = shaderConfigObject; 
        }

        // 4. Build a list of all the symbols for ray generation, miss and hit groups
        // Those shaders have to be associated with the payload definition
        {
            std::vector<std::wstring> exportedSymbols = BuildShaderExportList();

            // Build an array of the string pointers
            std::vector<LPCWSTR> exportedSymbolPointers = {};
            exportedSymbolPointers.reserve(exportedSymbols.size());
            for (const auto& name : exportedSymbols)
            {
                exportedSymbolPointers.push_back(name.c_str());
            }
            const WCHAR** shaderExports = exportedSymbolPointers.data();

            // Add a subobject for the association between shaders and the payload
            D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION shaderPayloadAssociation = {};
            shaderPayloadAssociation.NumExports = static_cast<UINT>(exportedSymbols.size());
            shaderPayloadAssociation.pExports = shaderExports;
            // Associate the set of shaders with the payload defined in the previous subobject
            shaderPayloadAssociation.pSubobjectToAssociate = &subobjects[(currentIndex - 1)];

            // Create and store the payload association object
            D3D12_STATE_SUBOBJECT shaderPayloadAssociationObject = {};
            shaderPayloadAssociationObject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
            shaderPayloadAssociationObject.pDesc = &shaderPayloadAssociation;
            subobjects[currentIndex++] = shaderPayloadAssociationObject; 
        }

        // 5. The root signature association requires two objects for each: one to declare the root
        // signature, and another to associate that root signature to a set of symbols
        for (RootSignatureAssociation& assoc : m_rootSignatureAssociations)
        {
            // Add a subobject to declare the root signature
            D3D12_STATE_SUBOBJECT rootSigObject = {};
            rootSigObject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
            rootSigObject.pDesc = &assoc.m_rootSignature;
            subobjects[currentIndex++] = rootSigObject;


            // Add a subobject for the association between the exported shader symbols and the root signature
            assoc.m_association.pExports = assoc.m_symbolPointers.data();
            assoc.m_association.NumExports = static_cast<UINT>(assoc.m_symbolPointers.size());
            assoc.m_association.pSubobjectToAssociate = &subobjects[(currentIndex - 1)];

            D3D12_STATE_SUBOBJECT rootSigAssociationObject = {};
            rootSigAssociationObject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
            rootSigAssociationObject.pDesc = &assoc.m_association;

            subobjects[currentIndex++] = rootSigAssociationObject;
        }

        // 6. The pipeline construction always requires an empty global root signature
        //    The pipeline construction always requires an empty local root signature
        {
            D3D12_STATE_SUBOBJECT globalRootSig;
            globalRootSig.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
            ID3D12RootSignature* dgSig = m_dummyGlobalRootSignature;
            globalRootSig.pDesc = &dgSig;
            subobjects[currentIndex++] = globalRootSig; 

            D3D12_STATE_SUBOBJECT dummyLocalRootSig;
            dummyLocalRootSig.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
            ID3D12RootSignature* dlSig = m_dummyLocalRootSignature;
            dummyLocalRootSig.pDesc = &dlSig;
            subobjects[currentIndex++] = dummyLocalRootSig; 
        }

        // 7. Add a subobject for the ray tracing pipeline configuration
        {
            D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
            pipelineConfig.MaxTraceRecursionDepth = m_maxRecursionDepth;

            D3D12_STATE_SUBOBJECT pipelineConfigObject = {};
            pipelineConfigObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
            pipelineConfigObject.pDesc = &pipelineConfig;

            subobjects[currentIndex++] = pipelineConfigObject; 
        }



        // Describe the ray tracing pipeline state object
        D3D12_STATE_OBJECT_DESC pipelineDesc = {};
        pipelineDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
        pipelineDesc.NumSubobjects = currentIndex; // static_cast<UINT>(subobjects.size());
        pipelineDesc.pSubobjects = subobjects.data();

        ID3D12StateObjectPtr rtStateObject = nullptr;
        HRESULT hr = device_->CreateStateObject(&pipelineDesc, IID_PPV_ARGS(&rtStateObject));
        if (FAILED(hr))
        {
            throw std::logic_error("Could not create the raytracing state object");
        }

        // TODO
        ID3D12StateObjectPropertiesPtr m_rtStateObjectProps;
        // Cast the state object into a properties object, allowing to later access
        // the shader pointers by name
        ThrowIfFailed(rtStateObject->QueryInterface(IID_PPV_ARGS(&m_rtStateObjectProps)));

        return rtStateObject;
    }






    ID3D12ResourcePtr m_outputResource;
    ID3D12DescriptorHeapPtr m_srvUavHeap;

    void create_raytracing_frame()
    {
        D3D12_RESOURCE_DESC resDesc = {};
        resDesc.DepthOrArraySize = 1;
        resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        // The backbuffer is actually DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, but sRGB
        // formats cannot be used with UAVs. For accuracy we should convert to sRGB
        // ourselves in the shader
        resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

        resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        resDesc.Width = render_desc_.width;
        resDesc.Height = render_desc_.height;
        resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resDesc.MipLevels = 1;
        resDesc.SampleDesc.Count = 1;
        ThrowIfFailed(device_->CreateCommittedResource(&kDefaultHeapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
            D3D12_RESOURCE_STATE_COPY_SOURCE, nullptr,
            IID_PPV_ARGS(&m_outputResource)));
    }

    ID3D12DescriptorHeapPtr CreateDescriptorHeap(ID3D12Device* device, uint32_t count, D3D12_DESCRIPTOR_HEAP_TYPE type,
        bool shaderVisible)
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = count;
        desc.Type = type;
        desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        ID3D12DescriptorHeap* pHeap;
        ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&pHeap)));
        return pHeap;
    }

    void create_raytracing_respack(accel top_accel)
    {
        // Create a SRV/UAV/CBV descriptor heap. We need 2 entries - 1 UAV for the
        // raytracing output and 1 SRV for the TLAS
        m_srvUavHeap = CreateDescriptorHeap(device_, 2, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true);

        // Get a handle to the heap memory on the CPU side, to be able to write the
        // descriptors directly
        D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = m_srvUavHeap->GetCPUDescriptorHandleForHeapStart();

        // Create the UAV. Based on the root signature we created it is the first
        // entry. The Create*View methods write the view information directly into
        // srvHandle
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        device_->CreateUnorderedAccessView(m_outputResource, nullptr, &uavDesc, srvHandle);

        // Add the Top Level AS SRV right after the raytracing output buffer
        srvHandle.ptr += device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.RaytracingAccelerationStructure.Location = top_accel.pResult->GetGPUVirtualAddress();
        // Write the acceleration structure view in the heap
        device_->CreateShaderResourceView(nullptr, &srvDesc, srvHandle);
    }


    ID3D12ResourcePtr CreateBuffer(ID3D12Device* m_device, uint64_t size, D3D12_RESOURCE_FLAGS flags,
        D3D12_RESOURCE_STATES initState, const D3D12_HEAP_PROPERTIES& heapProps)
    {
        D3D12_RESOURCE_DESC bufDesc = {};
        bufDesc.Alignment = 0;
        bufDesc.DepthOrArraySize = 1;
        bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufDesc.Flags = flags;
        bufDesc.Format = DXGI_FORMAT_UNKNOWN;
        bufDesc.Height = 1;
        bufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        bufDesc.MipLevels = 1;
        bufDesc.SampleDesc.Count = 1;
        bufDesc.SampleDesc.Quality = 0;
        bufDesc.Width = size;

        ID3D12Resource* pBuffer;
        ThrowIfFailed(m_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufDesc, initState, nullptr,
            IID_PPV_ARGS(&pBuffer)));
        return pBuffer;
    }

    buffer m_vertexBuffer;
    ShaderBindingTable m_sbtHelper;
    ID3D12ResourcePtr m_sbtStorage;
    // shader binding table
    void create_raytracing_sbt()
    {
        // The SBT helper class collects calls to Add*Program.  If called several
        // times, the helper must be emptied before re-adding shaders.
        m_sbtHelper.Reset();

        // The pointer to the beginning of the heap is the only parameter required by
        // shaders without root parameters
        D3D12_GPU_DESCRIPTOR_HANDLE srvUavHeapHandle = m_srvUavHeap->GetGPUDescriptorHandleForHeapStart();

        // The helper treats both root parameter pointers and heap pointers as void*,
        // while DX12 uses the
        // D3D12_GPU_DESCRIPTOR_HANDLE to define heap pointers. The pointer in this
        // struct is a UINT64, which then has to be reinterpreted as a pointer.
        auto heapPointer = reinterpret_cast<UINT64*>(srvUavHeapHandle.ptr);

        // The ray generation only uses heap data
        m_sbtHelper.AddRayGenerationProgram(L"RayGen", { heapPointer });

        // The miss and hit shaders do not access any external resources: instead they
        // communicate their results through the ray payload
        m_sbtHelper.AddMissProgram(L"Miss", {});

        // Adding the triangle hit shader
        m_sbtHelper.AddHitGroup(L"HitGroup", { (void*)(m_vertexBuffer.resource->GetGPUVirtualAddress()) });

        // Compute the size of the SBT given the number of shaders and their
        // parameters
        uint32_t sbtSize = m_sbtHelper.ComputeSBTSize();

        // Create the SBT on the upload heap. This is required as the helper will use
        // mapping to write the SBT contents. After the SBT compilation it could be
        // copied to the default heap for performance.
        m_sbtStorage = CreateBuffer(device_, sbtSize, D3D12_RESOURCE_FLAG_NONE,
            D3D12_RESOURCE_STATE_GENERIC_READ, kUploadHeapProps);
        if (!m_sbtStorage)
        {
            throw std::logic_error("Could not allocate the shader binding table");
        }
        // Compile the SBT from the shader and parameters info
        m_sbtHelper.Generate(m_sbtStorage, m_rtStateObjectProps);
    }


    bool bInited = false;
    accel accel_;
    virtual void tracing_ray() override
    {
        if (bInited == false)
        {
            bInited = true;

            // create accel desc
            {
                auto& res_desc = pool_.desc(cmd_.res_id);

                m_vertexBuffer = pool_[res_desc.vertex];

                accel_desc desc;
                bottom_accel_desc bottom;
                bottom.shapes = { { res_desc.vertex, res_desc.index}, };
                desc.bottoms = { bottom, };
                desc.instances = { {0, render_matrix{} }, };

                accel_ = create_accel(desc);
            }

            // create raytracing state
            create_raytracing_state();
            create_raytracing_frame();
            create_raytracing_respack(accel_);
            create_raytracing_sbt();
        }

        // Record commands.

            // #DXR
            // Bind the descriptor heap giving access to the top-level acceleration
            // structure, as well as the raytracing output
        std::vector<ID3D12DescriptorHeap*> heaps = { m_srvUavHeap };
        ctx_.mCommandList->SetDescriptorHeaps(static_cast<UINT>(heaps.size()), heaps.data());


        transition_resource(m_outputResource, texture_usage_::transfer_src_, texture_usage_::storage_texture);
        // Setup the raytracing task
        {
            D3D12_DISPATCH_RAYS_DESC desc = {};
            // The layout of the SBT is as follows: ray generation shader, miss
            // shaders, hit groups. As described in the CreateShaderBindingTable method,
            // all SBT entries of a given type have the same size to allow a fixed
            // stride.

            // The ray generation shaders are always at the beginning of the SBT.
            uint32_t rayGenerationSectionSizeInBytes = m_sbtHelper.GetRayGenSectionSize();
            desc.RayGenerationShaderRecord.StartAddress = m_sbtStorage->GetGPUVirtualAddress();
            desc.RayGenerationShaderRecord.SizeInBytes = rayGenerationSectionSizeInBytes;

            // The miss shaders are in the second SBT section, right after the ray
            // generation shader. We have one miss shader for the camera rays and one
            // for the shadow rays, so this section has a size of 2*m_sbtEntrySize. We
            // also indicate the stride between the two miss shaders, which is the size
            // of a SBT entry
            uint32_t missSectionSizeInBytes = m_sbtHelper.GetMissSectionSize();
            desc.MissShaderTable.StartAddress = m_sbtStorage->GetGPUVirtualAddress() + rayGenerationSectionSizeInBytes;
            desc.MissShaderTable.SizeInBytes = missSectionSizeInBytes;
            desc.MissShaderTable.StrideInBytes = m_sbtHelper.GetMissEntrySize();

            // The hit groups section start after the miss shaders. In this sample we
            // have one 1 hit group for the triangle
            uint32_t hitGroupsSectionSize = m_sbtHelper.GetHitGroupSectionSize();
            desc.HitGroupTable.StartAddress =
                m_sbtStorage->GetGPUVirtualAddress() + rayGenerationSectionSizeInBytes + missSectionSizeInBytes;
            desc.HitGroupTable.SizeInBytes = hitGroupsSectionSize;
            desc.HitGroupTable.StrideInBytes = m_sbtHelper.GetHitGroupEntrySize();

            // Dimensions of the image to render, identical to a kernel launch dimension
            desc.Width = render_desc_.width;
            desc.Height = render_desc_.height;
            desc.Depth = 1;

            // Bind the raytracing pipeline
            ctx_.mCommandList->SetPipelineState1(m_rtStateObject);
            // Dispatch the rays and write to the raytracing output
            ctx_.mCommandList->DispatchRays(&desc); 
        }



        // The raytracing output needs to be copied to the actual render target used
        // for display. For this, we need to transition the raytracing output from a
        // UAV to a copy source, and the render target buffer to a copy destination.
        // We can then do the actual copy, before transitioning the render target
        // buffer into a render target, that will be then used to display the image
        transition_resource(m_outputResource, texture_usage_::storage_texture, texture_usage_::transfer_src_);

        transition_render_target(texture_usage_::color_attachment, texture_usage_::transfer_dst_);
        auto& tex = pool_[cmd_.frm_desc.render_targets[0].tex_id];
        ctx_.mCommandList->CopyResource(tex.resource, m_outputResource);
        transition_render_target(texture_usage_::transfer_dst_, texture_usage_::color_attachment);
    }

#pragma endregion raytracing

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


    void compute()
    {

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


        ctx_.mCommandList->SetGraphicsRootSignature(cmd_.res.root_signature);
        auto primitive_topology = static_cast<D3D_PRIMITIVE_TOPOLOGY>(primitive_topology_map.at(cmd_.pipe_desc.primitive_type));
        ctx_.mCommandList->IASetPrimitiveTopology(primitive_topology);


        bind_respack_internal();
    }

    void bind_respack_internal()
    {
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


    void transition_resource(ID3D12ResourcePtr resource, texture_usage_ old_usage, texture_usage_ new_usage)
    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = resource;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(texture_usage_map.at(old_usage).d3d12);
        barrier.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(texture_usage_map.at(new_usage).d3d12);

        ctx_.mCommandList->ResourceBarrier(1, &barrier);
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

    void transition_render_target(frame_id id, texture_usage_ old_usage, texture_usage_ new_usage)
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
                    /*
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
                    */
                    auto tex_id = cmd_.frm_desc.render_targets[0].tex_id;
                    transition_texture(pool_[tex_id], old_usage, new_usage);
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