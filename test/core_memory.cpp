#include <gtest/gtest.h>

#include "fay/core/fay.h"
#include "fay/core/memory.h"

using namespace std;

// -------------------------------------------------------------------------------------------------

TEST(memory, memory)
{
    // WARNNING: you forget compare mem.size()
    char src[] = "abcd";
    fay::memory mem((uint8_t*)src, 5);
    ASSERT_TRUE((memcmp(src, mem.data(), /*mem.size()*/5) == 0) && (mem.size() == 5));

    src[0] = 'b';
    ASSERT_FALSE((memcmp(src, mem.data(), 5) == 0) && (mem.size() == 5));

    auto mem2 = std::move(mem);
    ASSERT_FALSE((memcmp(src, mem2.data(), 5) == 0) && (mem2.size() == 5));

    ((char*)mem2.data())[0] = 'b';
    ASSERT_TRUE((memcmp(src, mem2.data(), 5) == 0) && (mem2.size() == 5));

    auto mem3(mem2);
    ASSERT_TRUE((memcmp(src, mem3.data(), 5) == 0) && (mem3.size() == 5));

    fay::memory mem4;
    mem4 = mem2;
    ASSERT_TRUE((memcmp(src, mem4.data(), 5) == 0) && (mem4.size() == 5));
}
