#include <gtest/gtest.h>

#include "fay/core/fay.h"
#include "fay/core/utility.h"

using namespace std;
using namespace fay;

// -------------------------------------------------------------------------------------------------

TEST(utility, all_any_none)
{
    ASSERT_TRUE( all_of(vector{ 1, -1, 0 }, std::less_equal{},     1));
    ASSERT_TRUE( all_of(vector{ 1, -1, 0 }, std::greater_equal{}, -1));
    ASSERT_TRUE( any_of(vector{ 1, -1, 0 }, std::equal_to{},       0));
    ASSERT_TRUE(none_of(vector{ 1, -1, 0 }, std::greater{},        1));
    ASSERT_TRUE(none_of(vector{ 1, -1, 0 }, std::less{},          -1));

    ASSERT_TRUE( all_of(vector{ 1, -1, 0 }, [](auto&& a) { return a <=  1; }));
    ASSERT_TRUE( all_of(vector{ 1, -1, 0 }, [](auto&& a) { return a >= -1; }));
    ASSERT_TRUE( any_of(vector{ 1, -1, 0 }, [](auto&& a) { return a ==  0; }));
    ASSERT_TRUE(none_of(vector{ 1, -1, 0 }, [](auto&& a) { return a >   1; }));
    ASSERT_TRUE(none_of(vector{ 1, -1, 0 }, [](auto&& a) { return a <  -1; }));
}

TEST(utility, index)
{
    ASSERT_EQ(index(vector{ 1, -1, 0 },  1).value(), 0);
    ASSERT_EQ(index(vector{ 1, -1, 0 }, -1).value(), 1);
    ASSERT_EQ(index(vector{ 1, -1, 0 },  0).value(), 2);

    ASSERT_TRUE(index(vector{ 1, -1, 0 }, [](auto&&     a) { return a == -2; }).value_or(-1) == -1);
    ASSERT_TRUE(index(vector{ 1, -1, 0 }, [](auto&&     a) { return a ==  1; }).value() == 0);
    ASSERT_TRUE(index(vector{ 1, -1, 0 }, [](int        a) { return a == -1; }).value() == 1);
    ASSERT_TRUE(index(vector{ 1, -1, 0 }, [](const int& a) { return a ==  0; }).value() == 2);
}