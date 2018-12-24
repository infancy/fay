#include <iostream>
#include <gtest/gtest.h>

#include "fay/core/profiler.h"
#include "fay/math/vector.h"
#include "fay/math/matrix.h"

using namespace std;
using namespace fay;

// -------------------------------------------------------------------------------------------------
// TOOD: sub, div
// TODO: mat.full, mat(1), mat{{}, {}, {}, {}}

// TODO: compare glm with fay.math

TEST(math, vector_construct)
{
	vec4 v0;
	// vec4 vz();
	vec4 v1{};

	vec4 v2{ 0 };
	vec4 v3{ 1 };
	vec4 vy{ 1, 0, 0, 0 };
	vec4 vx{ 1, 1, 1, 1 };
	vec4 v4{ 0, 1, 2, 3 };

	vec4 v5(0.f);
	vec4 v6(1);
	vec4 v7((float*)&v4);
	vec4 v8(v4);

	ASSERT_EQ(v0 == v0, true);
	// ASSERT_EQ(v0 == vz, true);
	ASSERT_EQ(v0 == v1, true);
	ASSERT_EQ(v0 == v2, true);
	ASSERT_EQ(v0 == v5, true);

	ASSERT_EQ(v3 == vy, true);
	ASSERT_EQ(vx == v6, true);
	ASSERT_EQ(v4 == v7, true);
	ASSERT_EQ(v4 == v8, true);

	vec4i i0;
	vec4i i1{};

	vec4i i2{ 0 };
	vec4i i3{ 1 };
	vec4i iy{ 1, 0, 0, 0 };
	vec4i ix{ 1, 1, 1, 1 };
	vec4i i4{ 0, 1, 2, 3 };

	vec4i i5(0);
	vec4i i6(1);
	vec4i i7((int*)&i4);
	vec4i i8(i4);

	ASSERT_EQ(i0 == i0, true);
	ASSERT_EQ(i0 == i1, true);
	ASSERT_EQ(i0 == i2, true);
	ASSERT_EQ(i0 == i5, true);

	ASSERT_EQ(i3 == iy, true);
	ASSERT_EQ(ix == i6, true);
	ASSERT_EQ(i4 == i7, true);
	ASSERT_EQ(i4 == i8, true);
}

TEST(math, vector_operator)
{
	vec4 v0(0.f), v1(1), v2(2);

	auto v3 = v0 + v1;
	auto v4 = v0 + 1.f;
	auto v5 = 1.f + v1;

	ASSERT_EQ(v3 == v1, true);
	ASSERT_EQ(v4 == v1, true);
	ASSERT_EQ(v5 == v2, true);

	v0 += 1.f;
	ASSERT_EQ(v0 == v1, true);

	v1 += v1;
	ASSERT_EQ(v1 == v2, true);
}

TEST(math, matrix_construct)
{
	mat2 m0;
	// mat2 mz();
	mat2 m1{};

	mat2 m2{ 0 };
	mat2 m3{ 1 };
	mat2 my{ 1, 0, 0, 0 };
	mat2 mx{ 1, 0, 0, 1 };
	mat2 m4{ 1, 2, 3, 4 };

	mat2 m5(0.f);
	mat2 m6(1);
	mat2 m7((float*)&m4);
	mat2 m8(m4);

	ASSERT_EQ(m0 == m0, true);
	// ASSERT_EQ(m0 == mz, true);
	ASSERT_EQ(m0 == m1, true);
	ASSERT_EQ(m0 == m2, true);
	ASSERT_EQ(m0 == m5, true);

	ASSERT_EQ(m3 == my, true);
	ASSERT_EQ(mx == m6, true);
	ASSERT_EQ(m4 == m7, true);
	ASSERT_EQ(m4 == m8, true);
}

TEST(math, matrix_additive)
{
    mat4 m0(0.f), m1(1), m2({1, 1, 1, 1});

	auto m3 = m0 + m1;
	m0 += m1;

	ASSERT_EQ(m3 == m1, true);
	ASSERT_EQ(m0 == m1, true);

	auto m4 = m0 + 1.f;
	auto m5 = 1.f + m1;
	m1 += 1.f;

	//ASSERT_EQ(m4 == m2, true);
	//ASSERT_EQ(m5 == m2, true);
	//ASSERT_EQ(m1 == m2, true);
}

TEST(math, matrix_multiply)
{
	vec3 v3(2);
	vec4 v4(2);

	mat3x3 m33(2);
	mat3x4 m34(2);
	mat4x3 m43(2);
	mat4x4 m44(2);

	vec3 r1 = v3  * m33;
	vec3 r2 = m33 * v3;
	vec3 r3 = m34 * v4;  // m34 * v41
	vec4 r4 = v3  * m34; // v13 * m34
	vec4 r5 = v4  * m44;
	vec4 r6 = m44 * v4;

	ASSERT_EQ(r1 == vec3(4), true);
	ASSERT_EQ(r2 == vec3(4), true);
	ASSERT_EQ(r3 == vec3(4), true);
	ASSERT_EQ(r4 == vec4(4), true);
	ASSERT_EQ(r5 == vec4(4), true);
	ASSERT_EQ(r6 == vec4(4), true);

	mat4x3 r7 = m43 * m33; // m33 * m43
	mat3x4 r8 = m33 * m34;
	mat3x3 r9 = m34 * m43;
	mat4x4 ra = m43 * m34;

	mat4x3 rb = m44 * m43;
	mat3x4 rc = m34 * m44;
	mat4x4 rd = m44 * m44;

	ASSERT_EQ(r7 == mat4x3(12), true);
	ASSERT_EQ(r8 == mat3x4(12), true);
	ASSERT_EQ(r9 == mat3x3(16), true);
	ASSERT_EQ(ra == mat4x4(12), true);

	ASSERT_EQ(rb == mat4x3(16), true);
	ASSERT_EQ(rc == mat3x4(16), true);
	ASSERT_EQ(rd == mat4x4(16), true);
}