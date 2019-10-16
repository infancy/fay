#include <unordered_set>
#include "fay/render/shader.h"
#include "fay/resource/file.h"

namespace fay
{

inline namespace shader_type
{

enum class shader_data_type
{
    none,

    vertex_buffer,
    texture,
    sampler,
    structure,
    uniform_buffer,
};

struct shader_context
{
    vertex_layout entry_layout{}; // std::vector<vertex_attribute> export_layout{};
    std::vector<std::string> entry_names{};

    std::vector<shader_desc::sampler> samplers{};
    std::vector<shader_desc::uniform_block> uniform_blocks{}; // rename : uniforms
    //std::vector<shader_desc::uniform_block> buffers{}; // big object
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

    { "float3 mPos", { attribute_usage::position,  attribute_format::float3, 1} },
    { "float3 mNor", { attribute_usage::normal,    attribute_format::float3, 1} },
    { "float2 mTex", { attribute_usage::texcoord0, attribute_format::float2, 1} },
    { "float3 mTan", { attribute_usage::tangent,   attribute_format::float3, 1} },
    { "float3 mBit", { attribute_usage::bitangent, attribute_format::float3, 1} },

    // instance
    { "matrix iModel", { attribute_usage::instance_model, attribute_format::float4, 4} },
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


// TODO£º container | have£¨something);
auto have = [](const std::vector<std::string_view>& vs, std::string_view v)
{
    return any_of(vs, std::equal_to{}, v);
};

class shader_uniform // uniform_block/constant_buffer
{
public:
    // WARNNING: std140 layout
    static const inline std::unordered_map<std::string_view, uniform_type> shader_uniform_map =
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

        { "matrix", uniform_type::mat4 },

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

    static const inline std::unordered_map<uniform_type, uint> uniform_type_size_map =
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
    void parse_struct(std::string_view name, std::stringstream& stream)
    {
        structs[std::string(name)] = extracting_block_types(stream);
    }

    shader_desc::uniform_block uniform_block_info(std::string_view name, std::stringstream& stream)
    {
        shader_desc::uniform_block ub;
        ub.name = name;
        ub.types = extracting_block_types(stream);
        ub.size = compute_uniform_block_size(ub.types);

        return ub;
    }

private:
    std::vector<uniform_type> extracting_block_types(std::stringstream& stream)
    {
        std::vector<uniform_type> vs;

        std::string line;
        std::getline(stream, line);
        line = erase_white_spaces(line);
        DCHECK(line == "{");

        while (!stream.eof())
        {
            std::getline(stream, line);
            line = erase_white_spaces(line);
            if (line.empty())
                continue;
            if (line.find('}') != std::string::npos) // function defines
                break;

            auto words = extracting_text(line);
            if (words.empty())
                continue;

            // 'vec3 pos' or 'Material mat'
            std::vector<uniform_type> types;
            if (shader_uniform_map.find(words[0]) != shader_uniform_map.end())
            {
                types.push_back(shader_uniform_map.at(words[0]));
            }
            else if (auto iter = structs.find(std::string(words[0])); iter != structs.end())
            {
                types = (*iter).second;
            }
            else
            {
                LOG(ERROR) << "can't parser the uniform_block: " << words[0];
            }

            uint sz = 1;
            if (words.size() > 2)
            {
                sz = std::stoi(std::string(words[2])); // mat mats[2];
            }

            for (auto i : range(sz))
                vs.insert(vs.end(), types.begin(), types.end());
        }

        return vs;
    }

    uint compute_uniform_block_size(const std::vector<uniform_type>& type_sizes) const
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

        for (auto type : type_sizes)
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
            total += uniform_type_size_map.at(type);
        }

        return total;
    }

private:
    std::unordered_map<std::string, std::vector<uniform_type>> structs;
};

}

inline namespace shader_func
{

std::string load_file_to_string(const std::string& filename)
{
    auto file = load_text(filename);

    std::stringstream str_stream{};
    str_stream << file.rdbuf();

    return str_stream.str();
}

// WARNING: ignore same files
std::string add_include_files(const std::string& shader_filepath)
{
    std::unordered_set<std::string> included_files;
    std::string directory = get_directory(shader_filepath);
    std::string code{ load_file_to_string(shader_filepath) };

    while (true)
    {
        auto pound_pos = code.find("#include");

        if (pound_pos == std::string::npos)
            break;
        else
        {
            auto first_semicolon  = code.find('"', pound_pos);
            auto second_semicolon = code.find('"', first_semicolon + 1);

            auto inc_filename = code.substr(first_semicolon + 1, second_semicolon - first_semicolon - 1);
            
            std::string filecode;
            if (included_files.find(inc_filename) == included_files.end())
            {
                included_files.emplace(inc_filename);

                filecode = load_file_to_string(directory + inc_filename);
            }
            code.replace(pound_pos, second_semicolon - pound_pos + 1, std::move(filecode));
        }
    }

    std::cout << "\n-------------------source_code with include file begin: " << shader_filepath << "\n\n" << code << "\n\n-------------------------source_code end\n\n";
    return code;
}

#pragma region extracting_contex

shader_desc::sampler generate_sampler_desc(std::string_view native_type, std::string_view name)
{
    DCHECK(shader_sampler_map.find(native_type) != shader_sampler_map.end()) << "can't parser the sampler: " << native_type;

    shader_desc::sampler sampler{};
    sampler.name = name;
    sampler.type = shader_sampler_map.at(native_type);
    
    return sampler;
}

std::pair<shader_data_type, std::string_view> 
    extracting_line_glsl(shader_context& ctx, const std::vector<std::string_view>& words)
{
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
            return { shader_data_type::uniform_buffer, words.back() };
        }
        else if (words[2] == "in") // layout(origin_upper_left) in vec4 gl_FragCoord;
        {
        }
        else
        {
            LOG(ERROR) << "shouldn't be here";
        }
    }
    else if (words[0] == "struct")
    {
        return { shader_data_type::structure, words[1] };
    }
    else if (words[0] == "uniform")
    {
        if (shader_sampler_map.find(words[1]) != shader_sampler_map.end()) // texture sampler, e.g. 'uniform sampler2D Ambient;'
        {
            ctx.samplers.emplace_back(generate_sampler_desc(words[1], words[2]));
        }
        else if (shader_uniform::shader_uniform_map.find(words[1]) != shader_uniform::shader_uniform_map.end()) // unifrom variable e.g. 'uniform vec3 LightPos;'
        {
            LOG(ERROR) << "shouldn't use uniform variable: " << words[0] << ' ' << words[1]; // TODO
            DCHECK(shader_uniform::shader_uniform_map.find(words[1]) != shader_uniform::shader_uniform_map.end()) << "can't parser the uniform variable: " << words[1];
        }
        else
        {
            LOG(ERROR) << "shouldn't be here: " << words[0] << ' ' << words[1]; // TODO
        }
    }
    else
    {
        LOG(ERROR) << "ignore: " << words[0]; // TODO
    }
    
    // most times, directly add to ctx
    return { shader_data_type::none, {} };
}

std::pair<shader_data_type, std::string_view> 
    extracting_line_hlsl(shader_context& ctx, const std::vector<std::string_view>& words)
{
    if (words[0] == "struct")
    {
        if(words[1] == "VertexIn")
            return { shader_data_type::vertex_buffer, words[1] };
        else if (words[1] == "VertexOut")
            return { shader_data_type::none, {} };
        else
            return { shader_data_type::structure, words[1] };
    }
    else if (words[0] == "sampler")
    {

    }
    else if (have(words, "register"))
    {
        if (words[0] == "cbuffer")
        {
            return { shader_data_type::uniform_buffer, words[1] };
        }
        else if(words[0].find("Texture") != std::string_view::npos)
        {
            ctx.samplers.emplace_back(generate_sampler_desc(words[0], words[1]));
        }
    }
    else // parse input block
    {
        /*
            struct VertexPosNormalTex
            {
                float3 mPos : POSITION;
                float3 mNor : NORMAL;
                float2 mTex : TEXCOORD0;
            };
        */
        // DCHECK(ctx.entry_layout.size() == to_int(words[2]));

        std::string name(words[0]);
        name += ' ';
        name += words[1];

        if (shader_vertex_map.find(name) != shader_vertex_map.end())
        {
            ctx.entry_names.push_back(name);
            ctx.entry_layout.push_back(shader_vertex_map.at(name));
        }
        else
        {
            LOG(ERROR) << "shouldn't be here: " << words[0] << ' ' << words[1];
        }
        
    }

    return { shader_data_type::none, {} };
}

shader_context extracting_context(std::stringstream& stream, bool is_hlsl = false)
{
    auto extracting_line = is_hlsl ? extracting_line_hlsl : extracting_line_glsl;

    shader_uniform uniform_parser;
    shader_context ctx{};

    bool across_lines{};
    std::string line;
    while (!stream.eof())
    {
        std::getline(stream, line);
        line = erase_white_spaces(line);
        if (line.empty())
            continue;
        if (line.find('@') != std::string::npos) // function defines
            break;

        auto words = extracting_text(line);
        if (words.empty())
            continue;

        auto pair = extracting_line(ctx, words);
        switch (pair.first)
        {
            case shader_data_type::vertex_buffer: // only used for HLSL
                // do nothiing, continue parsing
                break;
            case shader_data_type::uniform_buffer:
                ctx.uniform_blocks.emplace_back(uniform_parser.uniform_block_info(pair.second, stream));
                break;
            case shader_data_type::structure:
                uniform_parser.parse_struct(pair.second, stream);
                break;
            default:
                break;
        }
    }

    // using aligna = alignas(8) bool;
    // goto: return
    return ctx;
}

#pragma endregion

shader_desc merge_context(std::vector<shader_context>&& ctxs)
{
    DCHECK(ctxs.size() >= 2);
    // DCHECK(ctxs[0].entry_layout.size() > 0);

    shader_desc desc;

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

} // namespace shader_func

// TODO: scan_shader_program(const std::string shader_name, std::vector<std::string> shader)
shader_desc scan_shader_program(const std::string shader_name, std::string vs_filepath, std::string fs_filepath, render_backend_type backend_type)
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

    bool is_hlsl{ false };
    if (backend_type == render_backend_type::d3d11)
    {
        is_hlsl = true;
        vs_filepath.replace(vs_filepath.end() - 3, vs_filepath.end(), "_vs.hlsl");
        fs_filepath.replace(fs_filepath.end() - 3, fs_filepath.end(), "_fs.hlsl");
    }

    std::cout << "==================================\n";
    std::string vs_code = add_include_files(vs_filepath);
    std::string fs_code = add_include_files(fs_filepath);

    std::stringstream vs_stream{ vs_code };
    std::stringstream fs_stream{ fs_code };

    auto vs_ctx = extracting_context(vs_stream, is_hlsl);
    auto fs_ctx = extracting_context(fs_stream, is_hlsl);

    auto&& desc = merge_context({ vs_ctx, fs_ctx });
    desc.name = shader_name;
    desc.vs = std::move(vs_code);
    desc.fs = std::move(fs_code);

    std::cout << "\n-----------------------\n\nshader " << desc.name << "'s context: \n";

    std::cout << "\nvertex names:\n";
    for (const auto& str : vs_ctx.entry_names)
        std::cout << str << '\n';

    std::cout << "\nuniform blocks:\n";
    for (const auto& ub : desc.uniform_blocks)
        std::cout << ub.name << '\n';

    std::cout << "\n samplers:\n";
    for (const auto& sampler : desc.samplers)
        std::cout << sampler.name << '\n';

    std::cout << "==================================\n\n";

    return desc;
}

} // namespace fay
