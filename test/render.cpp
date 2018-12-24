#include <gtest/gtest.h>

#include "fay/core/fay.h"
#include "fay/render/define.h"
#include "fay/render/pool.h"

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