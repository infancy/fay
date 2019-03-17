#include "fay/render/shader.h"

namespace fay
{

struct shader_context
{
    vertex_layout entry_layout{}; // use in vs, gs, fs
    std::vector<std::string> entry_names{};

    //std::vector<vertex_attribute> export_layout{};
    std::vector<shader_desc::sampler> samplers{};
    //std::vector<std::string> uniforms{};
    std::vector<shader_desc::uniform_block> uniform_blocks{};
};

// TODO:skip '//'

const std::unordered_map<std::string_view, vertex_attribute> shader_vertex_map =
{
    // GLSL

    { "vec3 mPos", { attribute_usage::position,  attribute_format::float3, 1} },
    { "vec3 mNor", { attribute_usage::normal,    attribute_format::float3, 1} },
    { "vec2 mTex", { attribute_usage::texcoord0, attribute_format::float2, 1} },
    { "vec3 mTan", { attribute_usage::tangent,   attribute_format::float3, 1} },
    { "vec3 mBit", { attribute_usage::bitangent, attribute_format::float3, 1} },

    // { "m2dPos", { attribute_usage::position,  attribute_format::float2, 1} },
    // { "mColor", { attribute_usage::color,  attribute_format::float3, 1} },

    // instance
    { "mat4 iModel", { attribute_usage::instance_model, attribute_format::float4, 4} },

    // HLSL


};

const std::unordered_map<std::string_view, texture_type> shader_sampler_map =
{
    // GLSL

    // TODO: sampler1D, sampler3DArray, samplerShadow...
    { "sampler2D",     texture_type::two },
    { "sampler3D",     texture_type::two },
    { "samplerCube",   texture_type::cube },
    { "sampler2DArry", texture_type::array },

    // HLSL

    { "Texture2D",     texture_type::two },
    { "Texture3D",     texture_type::two },
    { "TextureCube",   texture_type::cube },
    { "Texture2DArry", texture_type::array },
};


auto have = [](const std::vector<std::string_view>& vs, std::string_view v)
{
    return any_of(vs, std::equal_to{}, v);
};

class shader_uniform // uniform_block/constant_buffer
{
public:
    // WARNNING: std140 layout
    const std::unordered_map<std::string_view, uniform_type> shader_uniform_map =
    {
        // GLSL

        { "bool",  uniform_type::bvec1 },
        { "bvec2", uniform_type::bvec2 },
        { "bvec3", uniform_type::bvec3 },
        { "bvec4", uniform_type::bvec4 },

        { "int",   uniform_type::ivec1 },
        { "ivec2", uniform_type::ivec2 },
        { "ivec3", uniform_type::ivec3 },
        { "ivec4", uniform_type::ivec4 },

        { "uint",  uniform_type::uvec1 },
        { "uvec2", uniform_type::uvec2 },
        { "uvec3", uniform_type::uvec3 },
        { "uvec4", uniform_type::uvec4 },

        { "float", uniform_type::vec1 },
        { "vec2",  uniform_type::vec2 },
        { "vec3",  uniform_type::vec3 },
        { "vec4",  uniform_type::vec4 },

        { "mat2",  uniform_type::mat2 },
        { "mat3",  uniform_type::mat3 }, // !!!: save as mat3x4
        { "mat4",  uniform_type::mat4 },

        { "mat2x2", uniform_type::mat2x2 },
        { "mat2x3", uniform_type::mat2x3 }, // !!!: save as mat2x4
        { "mat2x4", uniform_type::mat2x4 },

        { "mat3x2", uniform_type::mat3x2 },
        { "mat3x3", uniform_type::mat3x3 }, // !!!: save as mat3x4
        { "mat3x4", uniform_type::mat3x4 },

        { "mat4x2", uniform_type::mat4x2 },
        { "mat4x3", uniform_type::mat4x3 }, // !!!: save as mat4x4
        { "mat4x4", uniform_type::mat4x4 },

        // HLSL

        //{ "bool", uniform_type::bvec1 },
        { "bool2",  uniform_type::bvec2 },
        { "bool3",  uniform_type::bvec3 },
        { "bool4",  uniform_type::bvec4 },

        //{ "int",  uniform_type::ivec1 },
        { "int2",   uniform_type::ivec2 },
        { "int3",   uniform_type::ivec3 },
        { "int4",   uniform_type::ivec4 },

        //{ "uint", uniform_type::uvec1 },
        { "uint2",  uniform_type::uvec2 },
        { "uint3",  uniform_type::uvec3 },
        { "uint4",  uniform_type::uvec4 },

        //{ "float", uniform_type::vec1 },
        { "float2", uniform_type::vec2 },
        { "float3", uniform_type::vec3 },
        { "float4", uniform_type::vec4 },

        { "float2x2", uniform_type::mat2x2 },
        { "float2x3", uniform_type::mat2x3 }, // !!!: save as mat2x4
        { "float2x4", uniform_type::mat2x4 },

        { "float3x2", uniform_type::mat3x2 },
        { "float3x3", uniform_type::mat3x3 }, // !!!: save as mat3x4
        { "float3x4", uniform_type::mat3x4 },

        { "float4x2", uniform_type::mat4x2 },
        { "float4x3", uniform_type::mat4x3 }, // !!!: save as mat4x4
        { "float4x4", uniform_type::mat4x4 },

    };

    const std::unordered_map<uniform_type, uint> uniform_type_size_map =
    {
        { uniform_type::bvec1,  4 },
        { uniform_type::bvec2,  8 },
        { uniform_type::bvec3, 16 },
        { uniform_type::bvec4, 16 },

        { uniform_type::ivec1,  4 },
        { uniform_type::ivec2,  8 },
        { uniform_type::ivec3, 16 },
        { uniform_type::ivec4, 16 },

        { uniform_type::uvec1,  4 },
        { uniform_type::uvec2,  8 },
        { uniform_type::uvec3, 16 },
        { uniform_type::uvec4, 16 },

        { uniform_type::vec1,  4 },
        { uniform_type::vec2,  8 },
        { uniform_type::vec3, 16 },
        { uniform_type::vec4, 16 },

        { uniform_type::mat2, 16 },
        { uniform_type::mat3, 48 }, // !!!: save as mat3x4
        { uniform_type::mat4, 64 },

        { uniform_type::mat2x2, 16 },
        { uniform_type::mat2x3, 32 }, // !!!: save as mat2x4
        { uniform_type::mat2x4, 32 },

        { uniform_type::mat3x2, 24 },
        { uniform_type::mat3x3, 48 }, // !!!: save as mat3x4
        { uniform_type::mat3x4, 48 },

        { uniform_type::mat4x2, 32 },
        { uniform_type::mat4x3, 64 }, // !!!: save as mat4x4
        { uniform_type::mat4x4, 64 },
    };

public:
    // TODO replace  'std::stringstream& stream' with 'std::string_view'
    shader_desc::uniform_block uniform_block_info(std::stringstream& stream)
    {

    }

private:
    std::vector<uniform_type_size> uniform_block_extracting_types(std::stringstream& stream)
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

    uint compute_uniform_block_size(const std::vector<uniform_type_size>& type_sizes)
    {
        uint total{};

        auto try_padding = [&total](uint align)
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
            // first padding
            switch (type)
            {

                case uniform_type::bvec1:
                case uniform_type::ivec1:
                case uniform_type::uvec1:
                case uniform_type::vec1:

                    // try_padding(4);
                    break;

                case uniform_type::bvec2:
                case uniform_type::ivec2:
                case uniform_type::uvec2:
                case uniform_type::vec2:
                case uniform_type::mat2:
                case uniform_type::mat2x2:
                case uniform_type::mat3x2:
                case uniform_type::mat4x2:

                    try_padding(8);
                    break;

                case uniform_type::bvec3:
                case uniform_type::ivec3:
                case uniform_type::uvec3:
                case uniform_type::vec3:
                case uniform_type::mat3:
                case uniform_type::mat2x3:
                case uniform_type::mat3x3:
                case uniform_type::mat4x3:

                case uniform_type::bvec4:
                case uniform_type::ivec4:
                case uniform_type::uvec4:
                case uniform_type::vec4:
                case uniform_type::mat4:
                case uniform_type::mat2x4:
                case uniform_type::mat3x4:
                case uniform_type::mat4x4:

                    try_padding(16);
                    break;

                case uniform_type::none:
                default:
                    break;
            }

            // then add size
            total += size;
        }

        return total;
    }

private:

};

inline namespace shader_func
{

// WARNING: report errors when include the same files
std::string add_include_files(std::string_view shader_code)
{

}

shader_context extracting_context(std::stringstream& stream)
{
    shader_context ctx{};

    std::string line;
    while (!stream.eof())
    {
        std::getline(stream, line); // TODO: skip blank line
        line = erase_white_spaces(line);
        auto words = extracting_text(line);
        if (words.empty())
            continue;

        if (words[0] == "layout")
        {
            if (words[3] == "in") // vertex layout, e.g. layout (location = 0) in vec3 mPos;
            {
                DCHECK(ctx.entry_layout.size() == to_int(words[2]));
                std::string name(words[4]);
                name += ' ';
                name += words[5];
                DCHECK(shader_vertex_map.find(name) != shader_vertex_map.end()) << "can't parser the vertex layout: " << words[5];

                ctx.entry_names.push_back(name);
                ctx.entry_layout.push_back(shader_vertex_map.at(name));
            }
            else if (*(words.end() - 2) == "uniform") // uniform block, e.g. 'layout (std140) uniform light_paras{ ... }'
            {
                auto type_size_pairs = uniform_block_extracting_types(stream);

                shader_desc::uniform_block ub{};
                ub.name = words.back();
                ub.size = compute_uniform_block_size(type_size_pairs);

                auto size = type_size_pairs.size();
                ub.types.resize(size);
                for (auto i : range(size))
                    ub.types[i] = type_size_pairs[i].type;

                ctx.uniform_blocks.emplace_back(std::move(ub));
            }
            else if (words[2] == "in") // layout(origin_upper_left) in vec4 gl_FragCoord;
            {
            }
            else
            {
                LOG(ERROR) << "shouldn't be here";
            }
        }
        else if (words[0] == "uniform")
        {
            if (shader_sampler_map.find(words[1]) != shader_sampler_map.end())
            {
                // texture sampler
                // uniform sampler2D Ambient;
                DCHECK(shader_sampler_map.find(words[1]) != shader_sampler_map.end()) << "can't parser the sampler: " << words[1];

                shader_desc::sampler sampler{};
                sampler.name = std::string(words[2]);
                sampler.type = shader_sampler_map.at(words[1]);
                ctx.samplers.emplace_back(sampler);
            }
            else if (shader_uniform_map.find(words[1]) != shader_uniform_map.end())
            {
                // unifrom variable
                // uniform vec3 LightPos;
                DCHECK(shader_uniform_map.find(words[1]) != shader_uniform_map.end()) << "can't parser the uniform variable: " << words[1];
                // TODO
            }
            else
            {
                LOG(ERROR) << "shouldn't be here: " << line;
            }
        }
        else if (have(words, "(")) // goto: return
        {
            break;
        }
        // else if ...
    }
    // using aligna = alignas(8) bool;
    // goto: return
    return ctx;
}

shader_desc merge_context(std::vector<shader_context>&& ctxs)
{
    DCHECK(ctxs.size() >= 2);
    DCHECK(ctxs[0].entry_layout.size() > 0);

    shader_desc desc;

    desc.vertex_names = ctxs.front().entry_names;
    desc.layout = ctxs.front().entry_layout;

    desc.vs_uniform_block_sz = ctxs.front().uniform_blocks.size();
    desc.fs_uniform_block_sz = ctxs.back().uniform_blocks.size();

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

}

// TODO: scan_shader_program(const std::string shader_name, std::vector<std::string> shader)
shader_desc scan_shader_program(const std::string shader_name, std::string vs_filepath, std::string fs_filepath, bool is_hlsl)
{
    /*
    if (buildin)
    {
        //std::stringbuf vs_buf(vs_filepath);
        //std::stringstream vs_tmp{ vs_buf };

        //vs_stream.read(vs_filepath.data(), vs_filepath.size());
        //fs_stream.read(fs_filepath.data(), fs_filepath.size());
        vs_code = vs_filepath;
        fs_code = fs_filepath;
        vs_stream.write(vs_filepath.data(), vs_filepath.size());
        fs_stream.write(fs_filepath.data(), fs_filepath.size());
    }
    */

    const auto vs_text = load_text(vs_filepath);
    const auto fs_text = load_text(fs_filepath);

    std::stringstream vs_stream{};
    std::stringstream fs_stream{};
    vs_stream << vs_text.rdbuf();
    fs_stream << fs_text.rdbuf();

    std::string vs_code = vs_stream.str();
    std::string fs_code = fs_stream.str();

    shader_context vs_ctx{};
    shader_context fs_ctx{};

    if (is_hlsl)
    {

    }
    else
    {
        vs_ctx = extracting_context(vs_stream);
        fs_ctx = extracting_context(fs_stream);
    }

    auto&& desc = merge_context({ vs_ctx, fs_ctx });
    desc.name = shader_name;
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
