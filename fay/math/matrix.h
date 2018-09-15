#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_MATH_MATRIX_H
#define FAY_MATH_MATRIX_H

#include "fay/math/vector.h"
#include "fay/math/operators.h"

namespace fay
{

// if you use like m[i][j], it is similar to a two-dimensional array[C][R]
// or when use m.begin(), m.end(), it is like a one-dimensional array[C * R]

// C: how many columns (how many elements per rows)
// R: how many rows    (how many elements per columns)
template <int C, int R, typename T = float>
struct mat final : arithmetic<mat<C, R, T>, T>
{
	enum { N = C * R };
	union
	{
		std::array<T, N> array_{};
		std::array<vec<R, T>, C> col_;
	};

	using col_type = vec<R, T>;
	using row_type = vec<C, T>;
	using value_type             = typename std::array<T, N>::value_type;
	using reference              = typename std::array<vec<R, T>, C>::reference;
	using const_reference        = typename std::array<vec<R, T>, C>::const_reference;
	using iterator               = typename std::array<T, N>::iterator;
	using const_iterator         = typename std::array<T, N>::const_iterator;
	using reverse_iterator       = typename std::array<T, N>::reverse_iterator;
	using const_reverse_iterator = typename std::array<T, N>::const_reverse_iterator;

	mat() = default;
	constexpr explicit mat(std::initializer_list<T> il) /*: array_{}*/
	{
		DCHECK(il.size() <= N);	/* TODO: DCHECK */
		auto p = array_.begin(); auto q = il.begin();
		for (; q != il.end(); ++p, ++q)
			*p = *q;
	}
	constexpr explicit mat(const T s) { for (auto& e : array_) e = s; }
	constexpr explicit mat(const T p[]) { for (auto& e : array_) e = *p++; }

	iterator               begin()         noexcept { return array_.begin(); }
	const_iterator         begin()   const noexcept { return array_.begin(); }
	iterator               end()           noexcept { return array_.end(); }
	const_iterator         end()     const noexcept { return array_.end(); }

	reverse_iterator       rbegin()        noexcept { return array_.rbegin(); }
	const_reverse_iterator rbegin()  const noexcept { return array_.rbegin(); }
	reverse_iterator       rend()          noexcept { return array_.rend(); }
	const_reverse_iterator rend()    const noexcept { return array_.rend(); }

	const_iterator         cbegin()  const noexcept { return array_.cbegin(); }
	const_iterator         cend()    const noexcept { return array_.cend(); }
	const_reverse_iterator crbegin() const noexcept { return array_.crbegin(); }
	const_reverse_iterator crend()   const noexcept { return array_.crend(); }

	// WARNING: return col_[i]
	reference       operator[](size_t i) { return col_[i]; }
	const_reference operator[](size_t i) const { return col_[i]; }
};

// -----------------------------------------------------------------------------
// operator functions

/* row-major matrix * vector, or vector * row-major matrix
   left-mul or right-mul is not important, it's just a convention

   memory(M * V):        memory(V * M):        math(M * V):
            | x                 | x - - -              | x
            | y                 | y - - -              | y         
            | z                 | z - - -              | z
            V w                 V w - - -              V w
   | x - - -             - - - >               - - - >
   | y - - -             x y z w               x y z w
   | z - - -                                   - - - -
   V w - - -                                   - - - -
                                               - - - -
*/
template <int C, int R, typename T>
inline typename mat<C, R, T>::col_type 
operator*(const mat<C, R, T>& m, const typename mat<C, R, T>::row_type& v)
{
	typename mat<C, R, T>::col_type r{};

	for (int i = 0; i < R; ++i)
		for (int j = 0; j < C; ++j)
			r[i] += m[j][i] * v[j];

	return r;
}
/* vector * column-major matrix, or column-major matrix * vector

   memory(V * M):        memory(M * V):        math(V * M):
          - - - >               | x                   | x - - -
          x y z w               | y                   | y - - -
          - - - -               | z                   | z - - -
          - - - -               V w                   V w - - -
          - - - -        - - - >               - - - >
   - - - >               x y z w               x y z w 
   x y z w               - - - - 
                         - - - -
                         - - - -
*/
template <int C, int R, typename T>
inline typename mat<C, R, T>::row_type
operator*(const typename mat<C, R, T>::col_type& v, const mat<C, R, T>& m)
{
	typename mat<C, R, T>::row_type r{};

	for (int i = 0; i < C; ++i)
		r[i] = dot(v, m[i]);

	return r;
}



} // namespace fay

#endif // FAY_MATH_MATRIX_H
