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
		vec<N, T> a_{};
		vec<C, vec<R, T>> m_;
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
		auto p = a_.begin(); auto q = il.begin();
		for (; q != il.end(); ++p, ++q)
			*p = *q;
	}
	constexpr explicit mat(const std::initializer_list<col_type>& il)
	{
		DCHECK(il.size() <= N);	/* TODO: DCHECK */
		auto p = m_.begin(); auto q = il.begin();
		for (; q != il.end(); ++p, ++q)
			*p = *q;
	}
	constexpr explicit mat(const T& s)  
	{ 
		// WARNING: not fill
		for (size_t i = 0; i < C && i < R; ++i)
			m_[i][i] = s;
	}
	constexpr explicit mat(const T* p)  { for (auto& e : a_) e = *p++; }

	void fill(const T& s) { for (auto& e : a_) e = s; }

	iterator               begin()         noexcept { return a_.begin(); }
	const_iterator         begin()   const noexcept { return a_.begin(); }
	iterator               end()           noexcept { return a_.end(); }
	const_iterator         end()     const noexcept { return a_.end(); }

	reverse_iterator       rbegin()        noexcept { return a_.rbegin(); }
	const_reverse_iterator rbegin()  const noexcept { return a_.rbegin(); }
	reverse_iterator       rend()          noexcept { return a_.rend(); }
	const_reverse_iterator rend()    const noexcept { return a_.rend(); }

	const_iterator         cbegin()  const noexcept { return a_.cbegin(); }
	const_iterator         cend()    const noexcept { return a_.cend(); }
	const_reverse_iterator crbegin() const noexcept { return a_.crbegin(); }
	const_reverse_iterator crend()   const noexcept { return a_.crend(); }

	// WARNING: return m_[i]
	reference       operator[](size_t i) { return m_[i]; }
	const_reference operator[](size_t i) const { return m_[i]; }
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
bool operator==(const mat<C, R, T>& x, const mat<C, R, T>& y) { return x.a_ == y.a_; }
template <size_t C, size_t R, typename T>
bool operator!=(const mat<C, R, T>& x, const mat<C, R, T>& y) { return x.a_ != y.a_; }

template <size_t C, size_t R, typename T>
mat<C, R, T> operator-(const mat<C, R, T>& v) { mat<C, R, T> r; r -= v; return r; } // for NRVO

#define FAY_MAT_ADDITIVE_T( OP )                                                                                         \
template <size_t C, size_t R, typename T>                                                                                \
inline mat<C, R, T>& operator OP##=(mat<C, R, T>& x, const mat<C, R, T>& y) { x.a_ OP##= y.a_; return x; }               \
                                                                                                                         \
template <size_t C, size_t R, typename T>                                                                                \
inline mat<C, R, T> operator OP(const mat<C, R, T>& x, const mat<C, R, T>& y) { mat<C, R, T> r(x); r OP##= y; return r; }

FAY_MAT_ADDITIVE_T(+)
FAY_MAT_ADDITIVE_T(-)

#define FAY_MAT_ARITHMETIC_U( OP )                                                                                 \
template <size_t C, size_t R, typename T> /*, typename U, bool = std::is_arithmetic<U>::value>*/                   \
inline mat<C, R, T>& operator OP##=(mat<C, R, T>& x, const T y) { x.a_ OP##= y; return x; }                        \
                                                                                                                   \
template <size_t C, size_t R, typename T> /*, typename U, bool = std::is_arithmetic<U>::value>*/                   \
inline mat<C, R, T> operator OP(const mat<C, R, T>& x, const T y) { mat<C, R, T> r(x); r OP##= y; return r; }      \
                                                                                                                   \
                                                                                                                   \
template <size_t C, size_t R, typename T> /*, typename U, bool = std::is_arithmetic<U>::value>*/                   \
inline mat<C, R, T> operator OP(const T x, const mat<C, R, T>& y) { mat<C, R, T> r(x); r.a_ OP##= x.a_; return r; }

FAY_MAT_ARITHMETIC_U(+)
FAY_MAT_ARITHMETIC_U(-)
FAY_MAT_ARITHMETIC_U(*)
FAY_MAT_ARITHMETIC_U(/)

#undef FAY_MAT_ADDITIVE_T
#undef FAY_MAT_ARITHMETIC_U



// -------------------------------------------------------------------------------------------------
// matrix multiply



/* 
   row-major matrix * vector, or vector * row-major matrix
   left-mul or right-mul is not important, it's just a convention

   memory(M * V):                memory(V * M):                math(M * V):
            | x                         | x - - -                      | x
            | y                         | y - - -                      | y         
            | z                         | z - - -                      | z
            V w                         V w - - -                      V w
   | x - - -                     - - - >                       - - - >
   | y - - -                     x y z w                       x y z w
   | z - - -                                                   - - - -
   V w - - -                                                   - - - -
                                                               - - - -
*/
template <size_t C, size_t R, typename T>
inline typename mat<C, R, T>::col_type 
operator*(const mat<C, R, T>& m, const typename mat<C, R, T>::row_type& v)
{
	typename mat<C, R, T>::col_type r{};

	for (size_t i = 0; i < R; ++i)
		for (size_t j = 0; j < C; ++j)
			r[i] += m[j][i] * v[j];

	return r;
}
/* 
   vector * column-major matrix, or column-major matrix * vector

   memory(V * M):                memory(M * V):                math(V * M):
          - - - >                       | x                           | x - - -
          x y z w                       | y                           | y - - -
          - - - -                       | z                           | z - - -
          - - - -                       V w                           V w - - -
          - - - -                - - - >                       - - - >
   - - - >                       x y z w                       x y z w 
   x y z w                       - - - - 
                                 - - - -
                                 - - - -
*/
template <size_t C, size_t R, typename T>
inline typename mat<C, R, T>::row_type
operator*(const typename mat<C, R, T>::col_type& v, const mat<C, R, T>& m)
{
	typename mat<C, R, T>::row_type r{};

	for (size_t i = 0; i < C; ++i)
		r[i] = dot(v, m[i]);

	return r;
}

/* 
   row-major matrix * column-major matrix

   memory(m * M):                                math(m * M):
		       - - - >                                   | X - - -
		       X Y X W                                   | Y - - -
		       - - - -                                   | Z - - -
		       - - - -                                   V W - - -
		       - - - -                           - - - >
   | x - - -                                     x y z w
   | y - - -                                     - - - - 
   | z - - -                                     - - - -
   V w - - -                                     - - - -
*/
template <size_t C, size_t R1, size_t R2, typename T>
inline mat<R2, R1, T> operator*(const mat<C, R1, T>& x, const mat<R2, C, T>& y)
{
	// e.g. : in memory: m42 = m32 * m43, in math: m24 = m23 * m34
	mat<R2, R1, T> r{};

	for (size_t i = 0; i < R2; ++i)
		r[i] = x * y[i];

	return r;
}



// -------------------------------------------------------------------------------------------------
// named functions



// transpose

// inverse


} // namespace fay

#endif // FAY_MATH_MATRIX_H
