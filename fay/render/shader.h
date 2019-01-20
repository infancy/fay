#pragma once


#include "fay/core/fay.h"
#include "fay/core/string.h"
#include "fay/render/define.h"
#include "fay/resource/text.h"

namespace fay
{

// TODO:skip '//'

const inline std::unordered_map<std::string_view, vertex_attribute> shader_vertex_map =
{
    { "vec3 mPos", { attribute_usage::position,  attribute_format::float3, 1} },
    { "vec3 mNor", { attribute_usage::normal,    attribute_format::float3, 1} },
    { "vec2 mTex", { attribute_usage::texcoord0, attribute_format::float2, 1} },
    { "vec3 mTan", { attribute_usage::tangent,   attribute_format::float3, 1} },
    { "vec3 mBit", { attribute_usage::bitangent, attribute_format::float3, 1} },

    // { "m2dPos", { attribute_usage::position,  attribute_format::float2, 1} },
    // { "mColor", { attribute_usage::color,  attribute_format::float3, 1} },

    // instance
    { "mInstanceModel", { attribute_usage::model_matrix, attribute_format::floatx, 16} },
};

const inline std::unordered_map<std::string_view, texture_type> shader_sampler_map =
{
    // TODO: sampler1D, sampler3DArray, samplerShadow...
    { "sampler2D",     texture_type::two },
    { "sampler3D",     texture_type::two },
    { "amplerCube",    texture_type::cube },
    { "sampler2DArry", texture_type::array },
};

struct uniform_type_size
{
    uniform_type type;
    uint32_t     size;
};

// WARNNING: std140 layout
const inline std::unordered_map<std::string_view, uniform_type_size> shader_uniform_map =
{
    { "bool",   { uniform_type::bvec1,  4 } },
    { "bvec2",  { uniform_type::bvec2,  8 } },
    { "bvec3",  { uniform_type::bvec3, 16 } },
    { "bvec4",  { uniform_type::bvec4, 16 } },

    { "int",    { uniform_type::ivec1,  4 } },
    { "ivec2",  { uniform_type::ivec2,  8 } },
    { "ivec3",  { uniform_type::ivec3, 16 } },
    { "ivec4",  { uniform_type::ivec4, 16 } },

    { "uint",   { uniform_type::uvec1,  4 } },
    { "uvec2",  { uniform_type::uvec2,  8 } },
    { "uvec3",  { uniform_type::uvec3, 16 } },
    { "uvec4",  { uniform_type::uvec4, 16 } },

    { "float",  { uniform_type::vec1,  4 } },
    { "vec2",   { uniform_type::vec2,  8 } },
    { "vec3",   { uniform_type::vec3, 16 } },
    { "vec4",   { uniform_type::vec4, 16 } },

    { "mat2",   { uniform_type::mat2, 16 } },
    { "mat3",   { uniform_type::mat3, 48 } }, // !!!
    { "mat4",   { uniform_type::mat4, 64 } },

    { "mat2x2", { uniform_type::mat2x2, 16 } },
    { "mat2x3", { uniform_type::mat2x3, 32 } }, // !!!
    { "mat2x4", { uniform_type::mat2x4, 32 } },

    { "mat3x2", { uniform_type::mat3x2, 24 } },
    { "mat3x3", { uniform_type::mat3x3, 48 } }, // !!!
    { "mat3x4", { uniform_type::mat3x4, 48 } },

    { "mat4x2", { uniform_type::mat4x2, 32 } },
    { "mat4x3", { uniform_type::mat4x3, 64 } }, // !!!
    { "mat4x4", { uniform_type::mat4x4, 64 } },
};

struct shader_context
{
    vertex_layout entry_layout{}; // use in vs, gs, fs
    std::vector<std::string> entry_names{};

    //std::vector<vertex_attribute> export_layout{};
    std::vector<shader_desc::sampler> samplers{};
    //std::vector<std::string> uniforms{};
    std::vector<shader_desc::uniform_block> uniform_blocks{};
};

inline auto have = [](const std::vector<std::string_view>& vs, std::string_view v)
{
    return any_of(vs, std::equal_to{}, v);
};

inline std::vector<uniform_type_size> uniform_block_extracting_types(std::stringstream& stream)
{
    std::vector<uniform_type_size> vs;

    std::string line;
    std::getline(stream, line);
    line = erase_white_spaces(line);
    DCHECK(line == "{");

    while (!stream.eof())
    {
        std::getline(stream, line); // TODO: skip blank line
        line = erase_white_spaces(line);
        auto texts = split(line); // extracting_text(line);

        if (texts.empty())
            continue;

        if (texts[0] == "}")
            break;

        // TODO: have(shader_uniform_map, texts[0]);
        DCHECK(shader_uniform_map.find(texts[0]) != shader_uniform_map.end()) << "can't parser the uniform_block: " << texts[0];

        vs.push_back(shader_uniform_map.at(texts[0]));
    }

    return vs;
}

inline uint32_t uniform_block_compute_size(const std::vector<uniform_type_size>& type_sizes)
{
    uint32_t total{};

    auto try_padding = [&total](uint32_t align)
    {
        auto tail = total % align;
        if (tail != 0)
        {
            total -= tail;
            total += align;
        }
    };

    for (auto[type, size] : type_sizes)
    {
        switch (type)
        {
                
            case fay::uniform_type::bvec1:
            case fay::uniform_type::ivec1:
            case fay::uniform_type::uvec1:
            case fay::uniform_type::vec1:

                try_padding(4);
                break;

            case fay::uniform_type::bvec2:
            case fay::uniform_type::ivec2:
            case fay::uniform_type::uvec2:
            case fay::uniform_type::vec2:
            case fay::uniform_type::mat2:
            case fay::uniform_type::mat2x2:
            case fay::uniform_type::mat3x2:
            case fay::uniform_type::mat4x2:

                try_padding(8);
                break;

            case fay::uniform_type::bvec3:
            case fay::uniform_type::ivec3:
            case fay::uniform_type::uvec3:
            case fay::uniform_type::vec3:
            case fay::uniform_type::mat3:
            case fay::uniform_type::mat2x3:
            case fay::uniform_type::mat3x3:
            case fay::uniform_type::mat4x3:

            case fay::uniform_type::bvec4:
            case fay::uniform_type::ivec4:
            case fay::uniform_type::uvec4:
            case fay::uniform_type::vec4:
            case fay::uniform_type::mat4:
            case fay::uniform_type::mat2x4:
            case fay::uniform_type::mat3x4:
            case fay::uniform_type::mat4x4:

                try_padding(16);
                break;

            case fay::uniform_type::invalid:
            default:
                break;
        }

        total += size;
    }

    return total;
}

inline shader_context shader_extracting_context(std::stringstream& stream)
{
    shader_context ctx{};

    std::string line;
    while (!stream.eof())
    {
        std::getline(stream, line); // TODO: skip blank line
        line = erase_white_spaces(line);
        auto vs = extracting_text(line);
        if (vs.empty())
            continue;

        if (vs[0] == "layout")
        { 
            if (vs[3] == "in")
            {
                // vertex layout
                // layout (location = 0) in vec3 mPos;

                DCHECK(ctx.entry_layout.size() == to_int(vs[2]));
                std::string name(vs[4]);
                name += ' ';
                name += vs[5];
                DCHECK(shader_vertex_map.find(name) != shader_vertex_map.end()) << "can't parser the vertex layout: " << vs[5];

                ctx.entry_names.push_back(name);
                ctx.entry_layout.push_back(shader_vertex_map.at(name));
            }
            else if (vs[2] == "uniform")
            {
                // uniform block
                //layout(std140) uniform color

                auto type_sizes = uniform_block_extracting_types(stream);
                uint32_t total  = uniform_block_compute_size(type_sizes);

                shader_desc::uniform_block ub{};
                ub.name = vs[3];
                ub.size = total;

                auto size = type_sizes.size();
                ub.types.resize(size);
                for (auto i : range(size))
                    ub.types[i] = type_sizes[i].type;

                ctx.uniform_blocks.emplace_back(std::move(ub));
            }
            else
            {
                LOG(ERROR) << "shouldn't be here";
            }
        }
        else if (vs[0] == "uniform")
        {
            if (shader_sampler_map.find(vs[1]) != shader_sampler_map.end())
            {
                // texture sampler
                // uniform sampler2D Ambient;
                DCHECK(shader_sampler_map.find(vs[1]) != shader_sampler_map.end()) << "can't parser the sampler: " << vs[1];

                shader_desc::sampler sampler{};
                sampler.name = std::string(vs[2]);
                sampler.type = shader_sampler_map.at(vs[1]);
                ctx.samplers.emplace_back(sampler);
            }
            else if (shader_uniform_map.find(vs[1]) != shader_uniform_map.end())
            {
                // unifrom variable
                // uniform vec3 LightPos;
                DCHECK(shader_uniform_map.find(vs[1]) != shader_uniform_map.end()) << "can't parser the uniform variable: " << vs[1];
                // TODO
            }
            else
            {
                LOG(ERROR) << "shouldn't be here";
            }
        }
        else if (have(vs, "main")) // goto: return
        {
            break;
        }
        // else if ...
    }
    // using aligna = alignas(8) bool;
    // goto: return
    return ctx;
}

inline shader_desc shader_merge_context(std::vector<shader_context>&& ctxs)
{
    DCHECK(ctxs.size() >= 2);
    DCHECK(ctxs[0].entry_layout.size() > 0);

    shader_desc desc;

    desc.vertex_names = ctxs.front().entry_names;
    desc.layout = ctxs.front().entry_layout;
    desc.vs_samplers_sz = ctxs.front().samplers.size();
    desc.fs_samplers_sz = ctxs.back().samplers.size();

    for (auto& ctx : ctxs)
    {
        // WARNNING: Need to erase the same samplers?
        // vs_samplers + gs_samplers + fs_samplers
        desc.samplers.insert(desc.samplers.end(), ctx.samplers.begin(), ctx.samplers.end());

        desc.uniform_blocks.insert(desc.uniform_blocks.end(), ctx.uniform_blocks.begin(), ctx.uniform_blocks.end());
    }

    // uniforms

    return desc;
}

// TODO: scan_shader_program(std::vector<std::string> shader, bool buildin = false)
inline shader_desc scan_shader_program(std::string vs, std::string fs, bool buildin = false)
{
    std::stringstream vs_stream{};
    std::stringstream fs_stream{};
    std::string vs_code;
    std::string fs_code;

    if (buildin)
    {
        //std::stringbuf vs_buf(vs);
        //std::stringstream vs_tmp{ vs_buf };

        //vs_stream.read(vs.data(), vs.size());
        //fs_stream.read(fs.data(), fs.size());
        vs_code = vs;
        fs_code = fs;
        vs_stream.write(vs.data(), vs.size());
        fs_stream.write(fs.data(), fs.size());
    }
    else
    {
        const auto vs_text = load_text(vs);
        const auto fs_text = load_text(fs);

        vs_stream << vs_text.rdbuf();
        fs_stream << fs_text.rdbuf();
        vs_code = vs_stream.str();
        fs_code = fs_stream.str();
    }

    auto&& vs_ctx = shader_extracting_context(vs_stream);
    auto&& fs_ctx = shader_extracting_context(fs_stream);

    auto&& desc = shader_merge_context({ vs_ctx, fs_ctx });
    desc.vs = vs_code;
    desc.fs = fs_code;

    std::cout << "\nshader " << desc.name << "'s context: \n";

    std::cout << "\nvertex names:\n";
    for (const auto& str : desc.vertex_names)
        std::cout << str << '\n';

    std::cout << "\nuniform blocks:\n";
    for (const auto& ub : desc.uniform_blocks)
        std::cout << ub.name << '\n';

    std::cout << "\n samplers:\n";
    for (const auto& sampler : desc.samplers)
        std::cout << sampler.name << '\n';

    return desc;
}

} // namespace fay