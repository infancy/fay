#include <gtest/gtest.h>

#include "fay/core/fay.h"
#include "fay/core/type.h"

using namespace std;
using namespace fay;

// -------------------------------------------------------------------------------------------------

#define FAY_TEST_TYPE_ENUM(T) \
ASSERT_EQ(type_v<T>, type_enum::T##_); \
ASSERT_TRUE((std::is_same_v<type_t<type_enum::T##_>, T>)); \
ASSERT_EQ(typeof(static_cast<T>(-1.5)), type_enum::T##_); \
cout << static_cast<T>(-1.5) << ' ' << (int)typeof(static_cast<T>(-1.5)) << ' ' << (int)type_v<T> << '\n';

#define FAY_TEST_TYPE_ENUM2(T1, T2) \
ASSERT_EQ(type_v<T1 T2>, type_enum::T1##_##T2##_); \
ASSERT_TRUE((std::is_same_v<type_t<type_enum::T1##_##T2##_>, T1 T2>)); \
ASSERT_EQ(typeof(static_cast<T1 T2>(-1.5)), type_enum::T1##_##T2##_); \
cout << static_cast<T1 T2>(-1.5) << ' ' << (int)typeof(static_cast<T1 T2>(-1.5)) << ' ' << (int)type_v<T1 T2> << '\n';

TEST(type, type)
{
    FAY_TEST_TYPE_ENUM(bool)
    FAY_TEST_TYPE_ENUM(char)
    FAY_TEST_TYPE_ENUM(short)
    FAY_TEST_TYPE_ENUM(int)
    FAY_TEST_TYPE_ENUM(long)
    FAY_TEST_TYPE_ENUM2(long, long)
    FAY_TEST_TYPE_ENUM2(unsigned, char)
    FAY_TEST_TYPE_ENUM2(unsigned, short)
    FAY_TEST_TYPE_ENUM2(unsigned, int)
    FAY_TEST_TYPE_ENUM2(unsigned, long)
    FAY_TEST_TYPE_ENUM(float)
    FAY_TEST_TYPE_ENUM(double)
    FAY_TEST_TYPE_ENUM2(long, double)
}

#undef FAY_TEST_TYPE_ENUM
#undef FAY_TEST_TYPE_ENUM2

// -------------------------------------------------------------------------------------------------