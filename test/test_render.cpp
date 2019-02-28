#include <gtest/gtest.h>

#include "fay/core/fay.h"
#include "fay/render/define.h"
#include "fay/render/pool.h"
#include "fay/render/shader.h"

using namespace std;
using namespace fay;

// -------------------------------------------------------------------------------------------------

TEST(render, pool)
{
    resource_pool<buffer_desc, texture_desc, shader_desc, pipeline_desc, frame_desc> pool{};
    
    ASSERT_FALSE(pool.have(buffer_id(0)));
    ASSERT_FALSE(pool.have(buffer_id(1)));

    //auto m = pool.map(buffer_id());
    auto buf_id = pool.insert(buffer_desc());
    ASSERT_TRUE(pool.have(buffer_id(1)));

    pool.erase(buf_id);
    ASSERT_FALSE(pool.have(buffer_id(1)));
}

// TODO: headless, render_init
TEST(render, resource_id)
{
    
}

static std::string test_vs = R"(
#version 330 core
layout (location = 0) in vec3 mPos;
layout (location = 1) in vec3 mNor;
layout (location = 2) in vec2 mTex;
layout (location = 3) in vec3 mTan;
layout (location = 4) in vec3 mBit;

out VS_OUT
{
    vec3 wPos;
    vec3 wNor;
    vec2 wTex;
    vec3 tLightPos;
    vec3 tViewPos;
    vec3 tPos;
} vs_out;

uniform mat4 Model;
uniform mat4 MVP;

uniform vec3 LightPos;
uniform vec3 ViewPos;

void main()
{
    // ...
}
)";

static std::string test_fs = R"(
#version 330 core

in VS_OUT 
{
    vec3 wPos;
    vec3 wNor;
    vec2 wTex;
    vec3 tLightPos;
    vec3 tViewPos;
    vec3 tPos;
} fs_in;

out vec4 FragColor;

uniform sampler2D Ambient;
uniform sampler2D Diffuse;
uniform sampler2D Specular;
uniform sampler2D Parallax;

layout (std140) uniform light_paras
{
    vec3 LightPos;
    vec3 ViewPos;

    bool use_normal_map;
    float sa;
    float sd;
    float ss;
    float shininess;
};

void main()
{
    // ...
}

)";

TEST(render, shader)
{
    shader_desc desc;
    desc.vertex_names = { "vec3 mPos", "vec3 mNor", "vec2 mTex", "vec3 mTan", "vec3 mBit" };
    desc.layout = 
    {
        { attribute_usage::position,  attribute_format::float3, 1},
        { attribute_usage::normal,    attribute_format::float3, 1},
        { attribute_usage::texcoord0, attribute_format::float2, 1},
        { attribute_usage::tangent,   attribute_format::float3, 1},
        { attribute_usage::bitangent, attribute_format::float3, 1},
    };
    desc.uniform_blocks.resize(1);
    desc.uniform_blocks[0] = 
    {
        "light_paras", 
        52,
        {
            uniform_type::vec3,
            uniform_type::vec3,

            uniform_type::bvec1,
            uniform_type::vec1,
            uniform_type::vec1,
            uniform_type::vec1,
            uniform_type::vec1,
        }
    };

    desc.vs_samplers_sz = 0;
    desc.fs_samplers_sz = 4;
    desc.samplers =
    {
        { "Ambient",  texture_type::two },
        { "Diffuse",  texture_type::two },
        { "Specular", texture_type::two },
        { "Parallax", texture_type::two },
    };

    shader_desc test = scan_shader_program("test_render_shader", test_vs, test_fs, true);

    ASSERT_TRUE(desc.vertex_names == test.vertex_names);
    ASSERT_TRUE(desc.layout == test.layout);
    ASSERT_TRUE(desc.uniform_blocks == test.uniform_blocks);
    ASSERT_TRUE(desc.vs_samplers_sz == test.vs_samplers_sz);
    ASSERT_TRUE(desc.fs_samplers_sz == test.fs_samplers_sz);
    ASSERT_TRUE(desc.samplers == test.samplers);
}


