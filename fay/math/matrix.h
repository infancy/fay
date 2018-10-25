#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_MATH_MATRIX_H
#define FAY_MATH_MATRIX_H

#include "fay/math/vector.h"
#include "fay/math/operators.h"

namespace fay
{

// mat<C, R, T>

// if you use like m[i][j], it is similar to a two-dimensional array[C][R]
// or when use m.begin(), m.end(), it is like a one-dimensional array[C * R]

// C: how many columns (how many elements per rows)
// R: how many rows    (how many elements per columns)
template <size_t C, size_t R, typename T = float>
struct mat
{
	enum { N = C * R };
	union
	{
		vec<N, T> a{};
		vec<C, vec<R, T>> m;
	};

	using col_type = vec<R, T>;
	using row_type = vec<C, T>;
	using value_type             = typename vec<N, T>::value_type;
	using pointer                = typename vec<N, T>::pointer;
	using const_pointer          = typename vec<N, T>::const_pointer;
	using iterator               = typename vec<N, T>::iterator;
	using const_iterator         = typename vec<N, T>::const_iterator;
	using reverse_iterator       = typename vec<N, T>::reverse_iterator;
	using const_reverse_iterator = typename vec<N, T>::const_reverse_iterator;

	using reference = typename vec<C, vec<R, T>>::reference;
	using const_reference = typename vec<C, vec<R, T>>::const_reference;

	constexpr mat() {} /*= default;*/
	constexpr explicit mat(const std::initializer_list<T>& il)
	{
		DCHECK(il.size() <= N);	/* TODO: DCHECK */
		auto p = a.begin(); auto q = il.begin();
		for (; q != il.end(); ++p, ++q)
			*p = *q;
	}
	constexpr explicit mat(const std::initializer_list<col_type>& il)
	{
		DCHECK(il.size() <= N);	/* TODO: DCHECK */
		auto p = m.begin(); auto q = il.begin();
		for (; q != il.end(); ++p, ++q)
			*p = *q;
	}
	constexpr explicit mat(const T& s)  
	{ 
		// WARNING: not fill
		for (size_t i = 0; i < C && i < R; ++i)
			m[i][i] = s;
	}
	constexpr explicit mat(const T* p)  { for (auto& e : a) e = *p++; }

	void fill(const T& s) { for (auto& e : a) e = s; }

	iterator               begin()         noexcept { return a.begin(); }
	const_iterator         begin()   const noexcept { return a.begin(); }
	iterator               end()           noexcept { return a.end(); }
	const_iterator         end()     const noexcept { return a.end(); }

	reverse_iterator       rbegin()        noexcept { return a.rbegin(); }
	const_reverse_iterator rbegin()  const noexcept { return a.rbegin(); }
	reverse_iterator       rend()          noexcept { return a.rend(); }
	const_reverse_iterator rend()    const noexcept { return a.rend(); }

	const_iterator         cbegin()  const noexcept { return a.cbegin(); }
	const_iterator         cend()    const noexcept { return a.cend(); }
	const_reverse_iterator crbegin() const noexcept { return a.crbegin(); }
	const_reverse_iterator crend()   const noexcept { return a.crend(); }

	// WARNING: return per-column
	reference       operator[](size_t i) { return m[i]; }
	const_reference operator[](size_t i) const { return m[i]; }
};

using mat2 = mat<2, 2, float>;
using mat3 = mat<3, 3, float>;
using mat4 = mat<4, 4, float>;

using mat2x2 = mat<2, 2, float>;
using mat2x3 = mat<2, 3, float>;
using mat2x4 = mat<2, 4, float>;
using mat3x2 = mat<3, 2, float>;
using mat3x3 = mat<3, 3, float>;
using mat3x4 = mat<3, 4, float>;
using mat4x2 = mat<4, 2, float>;
using mat4x3 = mat<4, 3, float>;
using mat4x4 = mat<4, 4, float>;



// -------------------------------------------------------------------------------------------------
// operator functions



template <size_t C, size_t R, typename T>
bool operator==(const mat<C, R, T>& x, const mat<C, R, T>& y) { return x.a == y.a; }
template <size_t C, size_t R, typename T>
bool operator!=(const mat<C, R, T>& x, const mat<C, R, T>& y) { return x.a != y.a; }

template <size_t C, size_t R, typename T>
mat<C, R, T> operator-(const mat<C, R, T>& v) { mat<C, R, T> r; r -= v; return r; } // for NRVO

#define FAY_MAT_ADDITIVE_T( OP )                                                                                         \
template <size_t C, size_t R, typename T>                                                                                \
inline mat<C, R, T>& operator OP##=(mat<C, R, T>& x, const mat<C, R, T>& y) { x.a OP##= y.a; return x; }                 \
                                                                                                                         \
template <size_t C, size_t R, typename T>                                                                                \
inline mat<C, R, T> operator OP(const mat<C, R, T>& x, const mat<C, R, T>& y) { mat<C, R, T> r(x); r OP##= y; return r; }

FAY_MAT_ADDITIVE_T(+)
FAY_MAT_ADDITIVE_T(-)

#define FAY_MAT_ARITHMETIC_U( OP )                                                                                 \
template <size_t C, size_t R, typename T> /*, typename U, bool = std::is_arithmetic<U>::value>*/                   \
inline mat<C, R, T>& operator OP##=(mat<C, R, T>& x, const T y) { x.a OP##= y; return x; }                         \
                                                                                                                   \
template <size_t C, size_t R, typename T> /*, typename U, bool = std::is_arithmetic<U>::value>*/                   \
inline mat<C, R, T> operator OP(const mat<C, R, T>& x, const T y) { mat<C, R, T> r(x); r OP##= y; return r; }      \
                                                                                                                   \
                                                                                                                   \
                                                                                                                   \
template <size_t C, size_t R, typename T> /*, typename U, bool = std::is_arithmetic<U>::value>*/                   \
inline mat<C, R, T> operator OP(const T x, const mat<C, R, T>& y) { mat<C, R, T> r(x); r.a OP##= x.a; return r; }

FAY_MAT_ARITHMETIC_U(+)
FAY_MAT_ARITHMETIC_U(-)
FAY_MAT_ARITHMETIC_U(*)
FAY_MAT_ARITHMETIC_U(/)

#undef FAY_MAT_ADDITIVE_T
#undef FAY_MAT_ARITHMETIC_U



// -------------------------------------------------------------------------------------------------
// matrix multiply



/* 
   glm-style multiplication, I think it is not convenient to use sometimes and troublesome to implement,
   the only advantage is that it can be passed to shader directly
.

   <-----------------------------><-----------------------------><----------------------------->
   row-major matrix * vector
   left-mul or right-mul is not important, it's just a convention(OpenGL and PBRT style)
   the key is th matrix is row-maior or column ?

   math(M * V):                   memory(M * V):                 real(V * M):
           | v                              | v                          | m - - -
           | v                              | v                          | m - - -
           | v                              | v                          | m - - - 
           V v                              V v                          V m - - -
   - - - >                        | m - - -                      - - - >
   m m m m                        | m - - -                      v v v v
   - - - -                        | m - - -
   - - - -                        V m - - -
   - - - -

   the "real result" is same to "math result"

   <-----------------------------><-----------------------------><----------------------------->
   vector * column-major matrix

   math(V * M):                   memory(V * M):                 real(M * V):
		   | m - - -                      - - - >                        | v
		   | m - - -                      m m m m                        | v
		   | m - - -                      - - - -                        | v
		   V m - - -                      - - - -                        V v
   - - - >                                - - - -                - - - >
   v v v v                        - - - >                        m m m m
								  v v v v                        - - - -
																 - - - -
																 - - - -
   the "real result" is same to "math result"

   <-----------------------------><-----------------------------><----------------------------->
   row-major matrix * column-major matrix

   math:                          memory:                        real:

   $ C = A * B $                  $ C^T = A^T * B^T $            $ C^T = B^T * A^T $

		   | b - - -                        - - - >                      | a - - -
		   | b - - -                        b b b b                      | a - - -
		   | b - - -                        - - - -                      | a - - -
		   V b - - -                        - - - -                      V a - - -
   - - - >                                  - - - -              - - - >
   a a a a                        | a - - -                      b b b b
   - - - -                        | a - - -                      - - - -
   - - - -                        | a - - -                      - - - -
   - - - -                        V a - - -                      - - - -

   e.g. : in math: m24 = m23 * m34, in memory: m42 = m32 * m43, in real: m42 = m43 * m32
   anyway, the "real result" is transpose of math's

   <-----------------------------><-----------------------------><----------------------------->

   in fay, I use PBRT-sytle(or math-style) multiplication.

*/

template <size_t C, size_t R, typename T>
inline typename mat<R, C, T> transpose(const mat<C, R, T>& m);

template <size_t C, size_t R, typename T>
inline typename mat<C, R, T>::row_type
operator*(const mat<C, R, T>& m, const typename mat<C, R, T>::col_type& v)	// C* 1 = C*R * R*1
{
	typename mat<C, R, T>::row_type r{};

	for (size_t i = 0; i < R; ++i)
		r[i] = dot(m[i], v);

	return r;
}

template <size_t C, size_t R, typename T>
inline typename mat<C, R, T>::col_type
operator*(const typename mat<C, R, T>::row_type& v, const mat<C, R, T>& m)
{
	return transpose(m) * v;
}

template <size_t C, size_t I, size_t R, typename T>
inline mat<C, R, T> operator*(const mat<C, I, T>& m0, const mat<I, R, T>& m1)
{
	auto mt = transpose(m1);
	mat<C, R, T> rst{};

	for (size_t i = 0; i < C; ++i)
		for (size_t j = 0; j < R; ++j)
			rst[i][j] = m0[i] * mt[j];

	return rst;
}



// -------------------------------------------------------------------------------------------------
// named functions



template <size_t C, size_t R, typename T>
inline typename mat<R, C, T> transpose(const mat<C, R, T>& m)
{
	mat<R, C, T> rst{};
}

// inverse
// 基于高斯消元的LU分解极其各种针对特殊形状矩阵的变种


} // namespace fay

#endif // FAY_MATH_MATRIX_H
