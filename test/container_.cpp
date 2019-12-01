#include "test.h"

#include "fay/core/algorithm.h"
#include "fay/core/fay.h"
#include "fay/container/container.h"
#include "fay/container/array.h"
#include "fay/math/tensor.h"

// -------------------------------------------------------------------------------------------------

#define ASSERT_EQUAL(left_, ...) ASSERT_TRUE(fay::is_seq_equal(left_, std::vector<int>(__VA_ARGS__)))

#pragma region template_test

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
    //seq p0{ a4.data(), 3 };
    //seq p0(a4.data(), 3);
}

#pragma endregion



#pragma region static_array

using array2 = fay::array<int, 2>;
using array3 = fay::array<int, 3>;
using array4 = fay::array<int, 4>;

TEST(static_array, array4_init)
{
    auto zero = { 0, 0, 0, 0 };
    auto one  = { 1, 1, 1, 1 };

    // array4 default_; ASSERT_EQUAL(default_, zero);
    // array4 value_{}; ASSERT_EQUAL(value_, zero);
    // array4 va(); // function define

    // direct-list
    array4 a0{ 0 };          ASSERT_EQUAL(a0, { 0, 0, 0, 0 });
    array4 a1{ 1 };          ASSERT_EQUAL(a1, { 1, 0, 0, 0 });
    array4 a2{ 1, 2 };       ASSERT_EQUAL(a2, { 1, 2, 0, 0 });
    array4 a3{ 1, 2, 3 };    ASSERT_EQUAL(a3, { 1, 2, 3, 0 });
    array4 a4{ 1, 2, 3, 4 }; ASSERT_EQUAL(a4, { 1, 2, 3, 4 });
    //array4 a5{ 1, 2, 3, 4, 5 };

    // fill
    array4 f0(0); ASSERT_EQUAL(f0, zero);
    array4 f1(1); ASSERT_EQUAL(f1, one);

    // sequence
    array4 s0{ a3 }; ASSERT_EQUAL(s0, { 1, 2, 3, 0 });
    array4 s1(a3);   ASSERT_EQUAL(s1, { 1, 2, 3, 0 });

    array4 t0{ a4, 3 }; ASSERT_EQUAL(t0, { 1, 2, 3, 0 });
    array4 t1(a4, 3);   ASSERT_EQUAL(t1, { 1, 2, 3, 0 });

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

using seq2 = fay::tensor<int, 1, 2>;
using seq3 = fay::tensor<int, 1, 3>;
using seq4 = fay::tensor<int, 1, 4>;

TEST(tensor, init)
{
    //seq4 a0;
    seq4 a0{};
    seq4 a1{ 0 };
    seq4 a2{ 1 };

    seq2 a3i{ 1, 2 };
    seq3 a3j{ 1, 2, 3 };
    seq4 a3k{ 1, 2, 3, 4 };
    seq4 a4{ a3i.x, a3i.y, a3j.z, a3k.w };
    //seq4 a33{ 1, 1, 1, 1, 1 };

    seq4 b0(0);
    seq4 b1(1);

    seq4 c0{ a3i.data(), 2 };
    seq4 c1( a3j.data(), 3);
    seq2 c2{ a3k.data() };
    seq3 c3( a3k.data());

    seq4 d1{ a3k };
}

#pragma endregion