/* readme



*/

#pragma once

#include "fay/math/vec.h"
#include "fay/math/operators.h"

namespace fay
{

// NOTE: just think of mat<R, C, float> as float m[R][C] with many methods.
template <size_t R, size_t C, typename T = float>
struct mat
{
	static_assert(R > 0, "Row number must be positive.");
	static_assert(C > 0, "Column number must be positive.");

	enum { N = R * C };
	union
	{
		vec<N, T> a_{};
		vec<R, vec<C, T>> m_;
	};

    using row_type               = vec<C, T>; // sub_array_type
	using col_type               = vec<R, T>;
    using vec_type               = vec<N, T>;
	using this_type              = mat<R, C, T>;
	using value_type             = typename vec<N, T>::value_type;
    using size_type              = typename vec<N, T>::size_type;
	using pointer                = typename vec<N, T>::pointer; 
	using const_pointer          = typename vec<N, T>::const_pointer;
	using iterator               = typename vec<N, T>::iterator;
	using const_iterator         = typename vec<N, T>::const_iterator;
	using reverse_iterator       = typename vec<N, T>::reverse_iterator;
	using const_reverse_iterator = typename vec<N, T>::const_reverse_iterator;
    using reference              = typename vec<R, vec<C, T>>::reference;
    using const_reference        = typename vec<R, vec<C, T>>::const_reference;

	constexpr mat() {} /*= default;*/

    // todo: explicit cast
    
    // WARNING: not fill!
	constexpr explicit mat(const T& s);
    constexpr explicit mat(const T* p) : a_{ p } {}
    // WARNING: mat4(2) != mat4{ 2 }
	constexpr explicit mat(std::initializer_list<T> il)        : a_{ il } { DCHECK(il.size() <= N); }
	constexpr explicit mat(std::initializer_list<row_type> il) : m_{ il } { DCHECK(il.size() <= C); }

	void fill(const T& s) { for (auto& e : a_) e = s; }

	iterator               begin()         noexcept { return a_.begin();   }
	const_iterator         begin()   const noexcept { return a_.begin();   }
	iterator               end()           noexcept { return a_.end();     }
	const_iterator         end()     const noexcept { return a_.end();     }

	reverse_iterator       rbegin()        noexcept { return a_.rbegin();  }
	const_reverse_iterator rbegin()  const noexcept { return a_.rbegin();  }
	reverse_iterator       rend()          noexcept { return a_.rend();    }
	const_reverse_iterator rend()    const noexcept { return a_.rend();    }

	const_iterator         cbegin()  const noexcept { return a_.cbegin();  }
	const_iterator         cend()    const noexcept { return a_.cend();    }
	const_reverse_iterator crbegin() const noexcept { return a_.crbegin(); }
	const_reverse_iterator crend()   const noexcept { return a_.crend();   }

    constexpr size_type    size()    const noexcept { return a_.size(); }

	// WARNING: return per-row
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

template<typename T>
using matrix4 = mat<4, 4, T>;

// WARNING: only fill diagonal element
template <size_t R, size_t C, typename T>
constexpr mat<R, C, T>::mat(const T& s)
{
	for (size_t i = 0; i < R && i < C; ++i)
		m_[i][i] = s;
}



// -------------------------------------------------------------------------------------------------
// operator functions



template <size_t R, size_t C, typename T>
bool operator==(const mat<R, C, T>& x, const mat<R, C, T>& y) { return x.a_ == y.a_; }
template <size_t R, size_t C, typename T>
bool operator!=(const mat<R, C, T>& x, const mat<R, C, T>& y) { return x.a_ != y.a_; }

template <size_t R, size_t C, typename T>
mat<R, C, T> operator-(const mat<R, C, T>& v) { mat<R, C, T> r; r -= v; return r; } // for NRVO

#define FAY_MAT_ADDITIVE_T( OP )                                                                                          \
template <size_t R, size_t C, typename T>                                                                                 \
inline mat<R, C, T>& operator OP##=(mat<R, C, T>& x, const mat<R, C, T>& y) { x.a_ OP##= y.a_; return x; }                \
                                                                                                                          \
template <size_t R, size_t C, typename T>                                                                                 \
inline mat<R, C, T> operator OP(const mat<R, C, T>& x, const mat<R, C, T>& y) { mat<R, C, T> r(x); r OP##= y; return r; }

FAY_MAT_ADDITIVE_T(+)
FAY_MAT_ADDITIVE_T(-)
// WARNNING: can't use FAY_MAT_ADDITIVE_T in '*' and '/'

#define FAY_MAT_ARITHMETIC_U( OP )                                                                                  \
template <size_t R, size_t C, typename T> /*, typename U, bool = std::is_arithmetic<U>::value>*/                    \
inline mat<R, C, T>& operator OP##=(mat<R, C, T>& x, const T y) { x.a_ OP##= y; return x; }                         \
                                                                                                                    \
template <size_t R, size_t C, typename T> /*, typename U, bool = std::is_arithmetic<U>::value>*/                    \
inline mat<R, C, T> operator OP(const mat<R, C, T>& x, const T y) { mat<R, C, T> r(x); r OP##= y; return r; }       \
                                                                                                                    \
template <size_t R, size_t C, typename T> /*, typename U, bool = std::is_arithmetic<U>::value>*/                    \
inline mat<R, C, T> operator OP(const T x, const mat<R, C, T>& y) { mat<R, C, T> r(y); r OP##= x; return r; }

FAY_MAT_ARITHMETIC_U(+)
FAY_MAT_ARITHMETIC_U(-)
FAY_MAT_ARITHMETIC_U(*)
FAY_MAT_ARITHMETIC_U(/)

#undef FAY_MAT_ADDITIVE_T
#undef FAY_MAT_ARITHMETIC_U



// -------------------------------------------------------------------------------------------------
// matrix multiply



template <size_t R, size_t C, typename T>
inline mat<C, R, T> transpose(const mat<R, C, T>& m);

template <size_t R, size_t C, typename T>
inline typename mat<R, C, T>::col_type
operator*(const mat<R, C, T>& m, const typename mat<R, C, T>::row_type& v)
{
    typename mat<R, C, T>::col_type rst{};

	for (size_t i = 0; i < R; ++i)
		rst[i] = dot(m[i], v);

	return rst;
}

template <size_t R, size_t C, typename T>
inline typename mat<R, C, T>::row_type
operator*(const typename mat<R, C, T>::col_type& v, const mat<R, C, T>& m)
{
	return transpose(m) * v;
}

template <size_t R, size_t M, size_t C, typename T>
inline mat<R, C, T> operator*(const mat<R, M, T>& m0, const mat<M, C, T>& m1)
{
    mat<R, C, T> rst{};

	auto mt = transpose(m1);

	for (size_t i = 0; i < R; ++i)
		for (size_t j = 0; j < C; ++j)
			rst[i][j] = dot(m0[i], mt[j]);

	return rst;
}



// -------------------------------------------------------------------------------------------------
// named functions



template <size_t R, size_t C, typename T>
inline mat<C, R, T> transpose(const mat<R, C, T>& m)
{
	mat<C, R, T> rst{};

    for (int i = 0; i < C; ++i)
        for (int j = 0; j < R; ++j)
            rst[i][j] = m[j][i];

    return rst;
}

// inverse



// -------------------------------------------------------------------------------------------------
// chain call



	/*
	this_type& translate(T x, T y, T z);
	this_type& rotate();
	this_type& scale(T x, T y, T z);
	this_type& scale(T u);
	this_type& rotatex();
	this_type& rotatey();
	this_type& rotatez();
	*/

} // namespace fay



/*
   glm-style multiplication, I think it is not convenient to use sometimes and troublesome to implement,
   the only advantage is that it can be passed to shader directly.

   <-----------------------------><-----------------------------><----------------------------->
   row-major matrix * vector
   left-mul or right-mul is not important, it's just a convention(OpenGL and PBRT style)
   the key is th matrix is row-maior or column ?

   math(M * V):                   memory(M * V):                 real(V * M):
           | v                              | v                          | m - - -
           | v                              | v                          | m - - -
           | v                              | v                          | m - - -
           V v                              V v                          V m - - -
                                  | m - - -                      - - - >
   m m m m                        | m - - -                      v v v v
   - - - -                        | m - - -
   - - - -                        V m - - -
   - - - -

   the "real result" is same to "math result"
   <-----------------------------><-----------------------------><----------------------------->
   row-major matrix * row-major matrix

   math:                          memory:                        real:

   $ R = A * B $                  $ R = A * B $                  $ R = A * B $

           | b b b b                      | b b b b                      | b b b b
           | - - - -                      | - - - -                      | - - - -
           | - - - -                      | - - - -                      | - - - -
           V - - - -                      V - - - -                      V - - - -
   - - - >                        - - - >                        - - - >
   a a a a                        a - - -                        a a a a
   - - - -                        a - - -                        - - - -
   - - - -                        a - - -                        - - - -
   - - - -                        a - - -                        - - - -

   e.g. :

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
   column-major matrix * column-major matrix

   math:                          memory:                        real:

   $ R = A * B $                  $ R^T = A^T * B^T $            $ R^T = B^T * A^T $

                                            - - - >
           | b b b b                        b - - -                      | a - - -
           | - - - -                        b - - -                      | a - - -
           | - - - -                        b - - -                      | a - - -
           V - - - -                        b - - -                      V a - - -
   - - - >                                                       - - - >
   a a a a                        | a - - -                      b - - -
   - - - -                        | a - - -                      b - - -
   - - - -                        | a - - -                      b - - -
   - - - -                        V a - - -                      b - - -

   e.g. : in math: m24 = m23 * m34, in memory: m42 = m32 * m43, in real: m42 = m43 * m32,
   anyway, the "real result" is transpose of math's

   <-----------------------------><-----------------------------><----------------------------->

   fay use PBRT-sytle(or math-style) multiplication.

*/
