#include "test.h"

#include "fay/core/algorithm.h"
#include "fay/core/fay.h"
#include "fay/container/container.h"
#include "fay/container/array.h"
#include "fay/math/tensor.h"

// -------------------------------------------------------------------------------------------------

#define ASSERT_EQUAL(left_, ...) ASSERT_TRUE(fay::is_seq_equal(left_, std::vector<int>(__VA_ARGS__)))
#define ASSERT_EQUAL2(left_, ...) ASSERT_TRUE(fay::is_seq_equal(left_, __VA_ARGS__))

#pragma region template_test

template<typename seq>
void test_init()
{
    auto zero = { 0, 0, 0, 0 };
    auto one = { 1, 1, 1, 1 };

    // seq default_; error
    // seq value_{}; error
    // seq va(); // function define

    // direct-list
    seq a0{ 0 };          ASSERT_EQUAL(a0, { 0, 0, 0, 0 });
    seq a1{ 1 };          ASSERT_EQUAL(a1, { 1, 0, 0, 0 });
    seq a2{ 1, 2 };       ASSERT_EQUAL(a2, { 1, 2, 0, 0 });
    seq a3{ 1, 2, 3 };    ASSERT_EQUAL(a3, { 1, 2, 3, 0 });
    seq a4{ 1, 2, 3, 4 }; ASSERT_EQUAL(a4, { 1, 2, 3, 4 });
    //seq a5{ 1, 2, 3, 4, 5 }; error

    // fill
    seq f0(0); ASSERT_EQUAL(f0, zero);
    seq f1(1); ASSERT_EQUAL(f1, one);

    // sequence
    //seq s0{ a3 }; not recommended
    //seq t0{ a4, 3 }; not recommended

    seq s1(a3); ASSERT_EQUAL(s1, { 1, 2, 3, 0 });
    seq s2(a4, 3); ASSERT_EQUAL(s2, { 1, 2, 3, 0 });
    //seq s3(a4.data(), 3); ASSERT_EQUAL(s3, { 1, 2, 3, 0 });
    seq s4(a4.cbegin(), a4.cbegin() + 3); ASSERT_EQUAL(s4, { 1, 2, 3, 0 });

    // pointer
    //seq p0{ a4.data(), 3 };
    //seq p1(a4.data(), 3);
}

template<typename seq>
void test_data()
{
    // todo
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

    array4 default_; ASSERT_EQUAL(default_, zero); // warning: init all elem by 0, not use
    array4 value_{}; ASSERT_EQUAL(value_,   zero); // warning: init all elem by 0, recommend
    // array4 va(); // function define

 // direct-list
 // array4 a0{ 0 };          ASSERT_EQUAL(a0, { 0, 0, 0, 0 }); // use 'array4 a0{ 0 };' will have runtime warning
 // array4 a1{ 1 };          ASSERT_EQUAL(a1, { 1, 0, 0, 0 }); // use 'array4 a0{ 1 };' will have runtime warning
    array4 a2{ 1, 2 };       ASSERT_EQUAL(a2, { 1, 2, 0, 0 });
    array4 a3{ 1, 2, 3 };    ASSERT_EQUAL(a3, { 1, 2, 3, 0 });

    // recommend:
    array4 a0{ 0, 0, 0, 0 }; ASSERT_EQUAL(a0, { 0, 0, 0, 0 }); // use 'array4 a0{ 0 };' will have runtime warning
    array4 a1{ 1, 0, 0, 0 }; ASSERT_EQUAL(a1, { 1, 0, 0, 0 }); // use 'array4 a0{ 1 };' will have runtime warning
    array4 a2{ 1, 2, 0, 0 }; ASSERT_EQUAL(a2, { 1, 2, 0, 0 });
    array4 a3{ 1, 2, 3, 0 }; ASSERT_EQUAL(a3, { 1, 2, 3, 0 });
    array4 a4{ 1, 2, 3, 4 }; ASSERT_EQUAL(a4, { 1, 2, 3, 4 });
    //array4 a5{ 1, 2, 3, 4, 5 }; error

    // fill
    // fay::array have a std::initializer_list ctor, so have to use '()' style ctor to call
    array4 f0(0); ASSERT_EQUAL(f0, zero);
    array4 f1(1); ASSERT_EQUAL(f1, one);

    // recommend:
    array4 f2{}; f2.fill(0); ASSERT_EQUAL(f2, zero);
    array4 f3{}; f3.fill(1); ASSERT_EQUAL(f3, one);

    // sequence
    array4 s0{ a3 };    ASSERT_EQUAL(s0, { 1, 2, 3, 0 });
    array4 s1{ a4, 3 }; ASSERT_EQUAL(s1, { 1, 2, 3, 0 });

    //array4 s3(a4.data(), 3); ASSERT_EQUAL(s3, { 1, 2, 3, 0 });
    array4 s4{ a4.cbegin(), a4.cbegin() + 3 }; ASSERT_EQUAL(s4, { 1, 2, 3, 0 });

    // pointer
    //array4 p0{ a4.data(), 3 };
    //array4 p1(a4.data(), 3);

    // copy
    array4 c0{ a4 }; ASSERT_EQUAL2(c0, a4);
    array4 c1 = a4; ASSERT_EQUAL2(c1, a4);
    // move
    array4 m0{ array4(1) }; ASSERT_EQUAL2(m0, one);
    array4 m1 = array4(1);  ASSERT_EQUAL2(m1, one);
    
    test_init<array4>();
}

TEST(static_array, data)
{
    /*const*/ array4 a{ 1, 2, 3, 4 };

    ASSERT_TRUE(!a.empty());
    ASSERT_TRUE(a.size() == 4);

    // get
    ASSERT_TRUE(a[0] == a.at(0));
    ASSERT_TRUE(a[3] == a.at(3));

    ASSERT_TRUE(a[0] == a.front());
    ASSERT_TRUE(a[3] == a.back());

    ASSERT_TRUE(a.data()             == a.begin());
    ASSERT_TRUE(a.data() + a.size() == a.end());

    ASSERT_TRUE(a.front() == *(a.rend() - 1));
    ASSERT_TRUE(a.back()  == *(a.rbegin()));
    // ASSERT_TRUE(*(a.data() + a.size()) == *(a.rbegin())); // interesting

    // set
    array4 b{ 5, 6, 7, 8 };
    a.front() = b[0];
    a[1]      = b[1];
    a.at(2)   = b[2];
    a.back()  = b[3];
    ASSERT_EQUAL2(a, b);

    array4 c{ 9, 10, 11, 12 };
    *(a.begin())    = c[0];
    *(a.rend() - 2) = c[1];
    *(a.end() - 2)  = c[2];
    *(a.rbegin())   = c[3];
    ASSERT_EQUAL2(a, c);
}

/*
TEST(static_array, const)
{
    const array4 a{ 1, 2, 3, 4 };
    
    static_assert(std::is_const_v<const int&>);
    static_assert(std::is_const_v<decltype(a.data())> == false);

    static_assert(std::is_const_v<decltype(a[0])>);
    static_assert(std::is_const_v<decltype(a.at(0))>);

    static_assert(std::is_const_v<decltype(a.front())>);
    static_assert(std::is_const_v<decltype(a.back())>);

    static_assert(std::is_const_v<decltype(a.begin())>);
    static_assert(std::is_const_v<decltype(a.rbegin())>);
    static_assert(std::is_const_v<decltype(a.crbegin())>);

    static_assert(std::is_const_v<decltype(a.end())>);
    static_assert(std::is_const_v<decltype(a.rend())>);
    static_assert(std::is_const_v<decltype(a.crend())>);
}
*/

#pragma endregion



#pragma region dynamic_array

/*
TEST(dynamic_array, init)
{
    fay::array<int> a0{ 0sz }; // error
    fay::array<int> a1{ 1sz };
    fay::array<int> a1{ 1sz, 1 };
    fay::array<int> a1{ 1sz, {1} };
    fay::array<int> a4{ 4sz, 1 };
    fay::array<int> a4{ 4sz, {1, 2, 3, 4} };

    // not provided
    // fay::array<int> a3{1, 2, 3};
    // fay::array<int> a4{1, 2, 3, 4};

}
*/

#pragma endregion



#pragma region tensor

using seq2 = fay::tensor<int, 1, 2>;
using seq3 = fay::tensor<int, 1, 3>;
using seq4 = fay::tensor<int, 1, 4>;

TEST(tensor, init)
{
    test_init<seq4>();
}

#pragma endregion