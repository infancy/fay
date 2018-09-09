#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_MATH_VECTOR_H
#define FAY_MATH_VECTOR_H

#include <array>
#include <initializer_list>

#include "fay/math/math.h"

namespace fay
{

#define FAY_VEC_FUNCTIONS                                                         \
using value_type             = typename std::array<T, N>::value_type;             \
using reference              = typename std::array<T, N>::reference;              \
using const_reference        = typename std::array<T, N>::const_reference;        \
using iterator               = typename std::array<T, N>::iterator;               \
using const_iterator         = typename std::array<T, N>::const_iterator;         \
using reverse_iterator       = typename std::array<T, N>::reverse_iterator;       \
using const_reverse_iterator = typename std::array<T, N>::const_reverse_iterator; \
                                                                                  \
vec() = default;                                                                  \
constexpr explicit vec(std::initializer_list<T>/*&&*/ il) /*: array_{}*/          \
{                                                                                 \
	DCHECK(il.size() <= N);	/* TODO: DCHECK */                                    \
	auto p = array_.begin(); auto q = il.begin();                                 \
	for (; q != il.end(); ++p, ++q)                                               \
		*p = *q;                                                                  \
}                                                                                 \
constexpr explicit vec(const T s)   { for (auto& e : array_) e = s; }             \
constexpr explicit vec(const T p[]) { for (auto& e : array_) e = *p++; }          \
                                                                                  \
iterator               begin()         noexcept { return array_.begin(); }        \
const_iterator         begin()   const noexcept { return array_.begin(); }        \
iterator               end()           noexcept { return array_.end(); }          \
const_iterator         end()     const noexcept { return array_.end(); }          \
                                                                                  \
reverse_iterator       rbegin()        noexcept { return array_.rbegin(); }       \
const_reverse_iterator rbegin()  const noexcept { return array_.rbegin(); }       \
reverse_iterator       rend()          noexcept { return array_.rend(); }         \
const_reverse_iterator rend()    const noexcept { return array_.rend(); }         \
                                                                                  \
const_iterator         cbegin()  const noexcept { return array_.cbegin(); }       \
const_iterator         cend()    const noexcept { return array_.cend(); }         \
const_reverse_iterator crbegin() const noexcept { return array_.crbegin(); }      \
const_reverse_iterator crend()   const noexcept { return array_.crend(); }        \
                                                                                  \
reference       operator[](size_t i)       { return array_[i]; }                  \
const_reference operator[](size_t i) const { return array_[i]; }

template <size_t N_, typename T = float>
struct vec
{
	enum { N = N_ };
	std::array<T, N> array_{};

	FAY_VEC_FUNCTIONS
};

template <typename T>
struct vec<2, T>
{
	enum { N = 2 };
	union
	{
		std::array<T, N> array_{};
		struct { T x, y; };
	};

	FAY_VEC_FUNCTIONS
};

template <typename T>
struct vec<3, T>
{
	enum { N = 3 };
	union
	{
		std::array<T, N> array_{};
		struct { T x, y, z; };
	};

	FAY_VEC_FUNCTIONS
};

template <typename T>
struct vec<4, T>
{
	enum { N = 4 };
	union
	{
		std::array<T, N> array_{};
		struct { T x, y, z, w; };
		//struct { T r, g, b, a; };
		//struct { vec3 xyz; };
		//struct { vec3 rgb; };
		//struct { vec2 xy; vec2 zw; };
	};

	FAY_VEC_FUNCTIONS
};

// -----------------------------------------------------------------------------
// operator functions

template <size_t N, typename T>
bool operator==(const vec<N, T>& x, const vec<N, T>& y)
{
	auto p = x.cbegin(), q = y.cbegin();
	for (; p != x.cend(); ++p, ++q)
		if (!is_equal(*p, *q)) 
			return false;
	return true;
}
template <size_t N, typename T>
bool operator!=(const vec<N, T>& x, const vec<N, T>& y) { return !(x == y); }

template <size_t N, typename T>
vec<N, T> operator-(const vec<N, T>& v) { vec<N, T> t; t -= v; return t; } // for NRVO

#define FAY_VEC_ARITHMETIC( OP )                                                                              \
template <size_t N, typename T, typename U>                                                                   \
inline vec<N, T>& operator OP##=(vec<N, T>& x, const vec<N, U>& y)                                            \
{                                                                                                             \
	for (size_t i = 0; i < N; ++i)                                                                            \
		x[i] OP##= y[i];                                                                                      \
	return x;                                                                                                 \
}                                                                                                             \
template <size_t N, typename T, typename U>                                                                   \
inline vec<N, T> operator OP(const vec<N, T>& x, const vec<N, U>& y) { vec<N, T> t(x); t OP##= y; return t; } \
                                                                                                              \
template <size_t N, typename T> /*, typename U, bool = std::is_arithmetic<U>::value>*/                        \
inline vec<N, T>& operator OP##=(vec<N, T>& x, const T y)          { for (auto& e : x) e OP##= y; return x; } \
                                                                                                              \
template <size_t N, typename T> /*, typename U, bool = std::is_arithmetic<U>::value>*/                        \
inline vec<N, T> operator OP(const vec<N, T>& x, const T y)          { vec<N, T> t(x); t OP##= y; return t; } \
                                                                                                              \
template <size_t N, typename T> /*, typename U, bool = std::is_arithmetic<U>::value>*/                        \
inline vec<N, T> operator OP(const T x, const vec<N, T>& y)          { vec<N, T> t(y); t OP##= x; return t; }

FAY_VEC_ARITHMETIC( + )
FAY_VEC_ARITHMETIC( - )
FAY_VEC_ARITHMETIC( * )
FAY_VEC_ARITHMETIC( / )

#undef FAY_VEC_FUNCTIONS
#undef FAY_VEC_ARITHMETIC

/*
template <size_t N, typename T, typename U>
inline vec<N, T>& operator +=(vec<N, T>& x, const vec<N, U>& y)
{
	auto p = x.begin(); auto q = y.cbegin();
	for (; q != y.cend(); ++p, ++q)
		*p += *q;
	return x;
}
template <size_t N, typename T, typename U>
inline vec<N, T>& operator +=(vec<N, T>& x, const U y) { for (auto& e : x) e += y; return x; }
template <size_t N, typename T, typename U>
inline vec<N, T> operator +(const vec<N, T>& x, const vec<N, U>& y) { vec<N, T> t(x); t += y; return t; }
template <size_t N, typename T, typename U>
inline vec<N, T> operator +(const vec<N, T>& x, const U& y) { vec<N, T> t(x); t += y; return t; }
template <size_t N, typename T, typename U>
inline vec<N, T> operator +(const U& x, const vec<N, T>& y) { vec<N, T> t(y); t += x; return t; }
*/

// -----------------------------------------------------------------------------
// named functions

/*

template <size_t N, typename T>
T length(const vec<N, T>& v)
{
	//return sqrtf(x * x + y * y + z * z + w * w);
}

template <size_t N, typename T>
T square(const vec<N, T>& v)
{
	//return x * x + y * y + z * z + w * w;
}

template <size_t N, typename T>
vec<N, T>& normalize(const vec<N, T>& v)
{
	v *= T(1) / length(v);
	return v;
}

template <size_t N, typename T>
void sqrt(const vec<N, T>& v)
{
	//
}
*/
// x dot y != x * y
template <size_t N, typename T>
T dot(const vec<N, T>& v0, const vec<N, T>& v1)
{
	T r{};

	for (size_t i = 0; i < N; ++i)
		r += v0[i] * v1[i];

	return r;
}
/*
// only for N=3
template <typename T>
vec<3, T>& cross(const vec<3, T>& v0, const vec<3, T>& v1)
{
	return vec<3, T>();
}

template <typename T>
vec<3, T>& cross(const vec<4, T>& v0, const vec<4, T>& v1)
{
	return vec<4, T>(
		v0.w * v1.x + v0.x * v1.w - v0.z * v1.y + v0.y * v1.z,
		v0.w * v1.y + v0.y * v1.w - v0.x * v1.z + v0.z * v1.x,
		v0.w * v1.z + v0.z * v1.w - v0.y * v1.x + v0.x * v1.y,
		v0.w * v1.w - v0.x * v1.x - v0.y * v1.y - v0.z * v1.z);
}
*/

// -----------------------------------------------------------------------------
// output

template <size_t N, typename T> // typename = std::enable_if_t<!std::is_floating_point_v<T>, void>>
inline std::ostream& operator<<(std::ostream& os, const vec<N, T>& v)
{
	os << "[ ";
	for (auto e : v) os << e << ", ";
	os << " ]";
	return os;
}


template <size_t N> // typename T, typename = std::enable_if_t<std::is_floating_point_v<T>, void>>
inline std::ostream& operator<<(std::ostream& os, const vec<N, float>& v)	// TODO: float_point
{
	os << "[ ";
	for (auto e : v) os << e << ", ";	// TODO
	// TODO: putback(", ")
	os << " ]";
	return os;
}

} // namespace fay

#endif // FAY_MATH_VECTOR_H
