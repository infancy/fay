#include <iostream>

#include <gtest/gtest.h>

#include "fay/core/profiler.h"
#include "fay/math/vector.h"
#include "fay/math/matrix.h"

using namespace std;
using namespace fay;

// -----------------------------------------------------------------------------

using vec2 = vec<2, float>;
using vec3 = vec<3, float>;
using vec4 = vec<4, float>;
using mat2x4 = mat<2, 4, float>;

TEST(math, vector)
{
	vec4 v0{0.f, 0.f, 0.f, 0.f}, v1(1.f);

	auto add1 = v0 + v1;
	auto add2 = v0 + 1.f;
	auto add3 = 1.f + v1;

	cout << add1 << '\n' << add2 << '\n' << add3 << '\n';

	add1 += add2;
	add2 += 1.f;

	cout << add1 << '\n' << add2 << '\n';

	auto v3 = v0 + 3.f;
	//auto v4 = 4 * v1;
	//auto v5 = v3 / 0.5f;
	//auto v6 = v4 - 1;

	auto v7 = v0 + v1;
	//auto v8 = v3 - v2;
	//auto v9 = v1 * v2;
	//auto va = v9 / v1;
	//auto vb(va);
}

TEST(math, matrix_multiply_vector)
{
	mat2x4 m(1);
	vec2 v1(1);
	vec4 v2(1);

	auto col = m * v1;
	auto row = v2 * m;
}