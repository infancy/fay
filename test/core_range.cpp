#include <gtest/gtest.h>

#include "fay/core/range.h"

using namespace std;
using namespace fay;

// -------------------------------------------------------------------------------------------------

TEST(range, range)
{
    auto equal = [](auto src, auto target)
    {
        return (src.size() == target.size()) &&
            std::equal(src.begin(), src.end(), target.begin());
    };

    auto positive = { 0, 1, 2 };
    auto negative = { 0, -1, -2 };
    auto one = { 0 };
    decltype(one) zero = {};

    ASSERT_TRUE(equal(range(3), positive));
    //ASSERT_TRUE(equal(range(0, -3), negative));
    ASSERT_TRUE(equal(range(1), one));
    ASSERT_TRUE(equal(range(0), zero));

    ASSERT_TRUE(equal(range(0, 3), positive));
    //ASSERT_TRUE(equal(range(0, -3), negative));
    ASSERT_TRUE(equal(range(0, 1), one));
    ASSERT_TRUE(equal(range(0, 0), zero));

    ASSERT_TRUE(equal(range(0, 3, 1), positive));
    ASSERT_TRUE(equal(range(0, -3, -1), negative));
    ASSERT_TRUE(equal(range(0, 1, 1), one));
    ASSERT_TRUE(equal(range(0, 0, 1), zero));
    ASSERT_TRUE(equal(range(0, 0, -1), zero));
}