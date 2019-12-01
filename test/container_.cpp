#include "test.h"

#include "fay/core/algorithm.h"
#include "fay/core/fay.h"
#include "fay/container/container.h"
#include "fay/container/array.h"
#include "fay/math/tensor.h"

// -------------------------------------------------------------------------------------------------

// TODO: template_test
#pragma region static_array

template<typename seq>
void test_init_4()
{
    seq default_;
    seq value_{};

    // direct-list
    seq a0{ 0 };
    seq a1{ 1 };
    seq a2{ 1, 2 };
    seq a3{ 1, 2, 3 };
    seq a4{ 1, 2, 3, 4 };
    //seq a5{ 1, 2, 3, 4, 5 };

    // fill
    seq f0(0);
    seq f1(1);

    // sequence
    seq s0{ a3 };
    seq s1(a3);

    seq t0{ a4, 3 };
    seq t1(a4, 3);

    // pointer
    seq p0{ a4.data(), 3 };
    seq p0(a4.data(), 3);
}

#pragma endregion

#define ASSERT_EQUAL(left_, right_) ASSERT_TRUE(fay::is_seq_equal(left_, right_))

#pragma region static_array

using array2 = fay::array<int, 2>;
using array3 = fay::array<int, 3>;
using array4 = fay::array<int, 4>;

TEST(static_array, array4_init)
{
    auto zero = { 0, 0, 0, 0 };
    auto one  = { 1, 1, 1, 1 };

    array4 default_;  ASSERT_EQUAL(default_, zero);
    array4 value_{};  ASSERT_EQUAL(value_, one);

    // direct-list
    array4 a0{ 0 };  ASSERT_EQUAL(a0, zero);
    array4 a1{ 1 };  ASSERT_EQUAL(a1, one);
    array4 a2{ 1, 2 };
    array4 a3{ 1, 2, 3 };
    array4 a4{ 1, 2, 3, 4 };
    //array4 a5{ 1, 2, 3, 4, 5 };

    // fill
    array4 f0(0);
    array4 f1(1);

    // sequence
    array4 s0{ a3 };
    array4 s1(a3);

    array4 t0{ a4, 3 };
    array4 t1(a4, 3);

    // pointer
    //array4 p0{ a4.data(), 3 };
    //array4 p1(a4.data(), 3);

    //test_init_4<array4>();
}

/*
TEST(static_array, copy)
{
    array2 a2{ 1, 2 };
    array3 a3{ 1, 2, 3 };
    array4 a4{ 1, 2, 3, 4 };

    array4 c0{ a2.data(), 2 };
    array4 c1( a3.data(), 3);
}
*/

#pragma endregion

#pragma region dynamic_array

TEST(dynamic_array, init)
{
    //fay::array<int> an(1);
}

#pragma endregion

#pragma region tensor

using vec2 = fay::tensor<int, 1, 2>;
using vec3 = fay::tensor<int, 1, 3>;
using vec4 = fay::tensor<int, 1, 4>;

TEST(tensor, init)
{
    //vec4 a0;
    vec4 a0{};
    vec4 a1{ 0 };
    vec4 a2{ 1 };

    vec2 a3i{ 1, 2 };
    vec3 a3j{ 1, 2, 3 };
    vec4 a3k{ 1, 2, 3, 4 };
    vec4 a4{ a3i.x, a3i.y, a3j.z, a3k.w };
    //vec4 a33{ 1, 1, 1, 1, 1 };

    vec4 b0(0);
    vec4 b1(1);

    vec4 c0{ a3i.data(), 2 };
    vec4 c1( a3j.data(), 3);
    vec2 c2{ a3k.data() };
    vec3 c3( a3k.data());

    vec4 d1{ a3k };
}

#pragma endregion