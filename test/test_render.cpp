#include <gtest/gtest.h>

#include "fay/core/fay.h"
#include "fay/core/range.h"
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

TEST(render, shader_glsl_and_hlsl)
{
    shader_desc desc;
    desc.layout =
    {
        { attribute_usage::position,  attribute_format::float3, 1},
        { attribute_usage::normal,    attribute_format::float3, 1},
        { attribute_usage::texcoord0, attribute_format::float2, 1},
        { attribute_usage::tangent,   attribute_format::float3, 1},
        { attribute_usage::bitangent, attribute_format::float3, 1},
    };

    desc.vs_uniform_block_sz = 0;
    desc.fs_uniform_block_sz = 1;
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

    shader_desc tests[2];
    tests[0] = scan_shader_program("test_render_shader", "test/test_render_glsl.vs", "test/test_render_glsl.fs");
    tests[1] = scan_shader_program("test_render_shader", "test/test_render_vs.hlsl", "test/test_render_fs.hlsl", true);

    for (uint i : range(2))
    {
        auto test = tests[i];

        // ASSERT_TRUE(desc.vertex_names == test.vertex_names);
        ASSERT_TRUE(desc.layout == test.layout);

        ASSERT_TRUE(desc.vs_uniform_block_sz == test.vs_uniform_block_sz);
        ASSERT_TRUE(desc.fs_uniform_block_sz == test.fs_uniform_block_sz);
        ASSERT_TRUE(desc.uniform_blocks == test.uniform_blocks);

        ASSERT_TRUE(desc.vs_samplers_sz == test.vs_samplers_sz);
        ASSERT_TRUE(desc.fs_samplers_sz == test.fs_samplers_sz);
        ASSERT_TRUE(desc.samplers == test.samplers);
    }
}