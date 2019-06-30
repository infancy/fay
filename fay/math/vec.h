#pragma once

#include <array>
#include <initializer_list>

#include "fay/math/math.h"

namespace fay
{

// vec<N, T>

#define FAY_VEC_FUNCTIONS                                                         \
using value_type             = typename std::array<T, N>::value_type;             \
using size_type              = typename std::array<T, N>::size_type;              \
using pointer                = typename std::array<T, N>::pointer;                \
using const_pointer          = typename std::array<T, N>::const_pointer;          \
using reference              = typename std::array<T, N>::reference;              \
using const_reference        = typename std::array<T, N>::const_reference;        \
using iterator               = typename std::array<T, N>::iterator;               \
using const_iterator         = typename std::array<T, N>::const_iterator;         \
using reverse_iterator       = typename std::array<T, N>::reverse_iterator;       \
using const_reverse_iterator = typename std::array<T, N>::const_reverse_iterator; \
                                                                                  \
constexpr vec() = default;                                                        \
constexpr explicit vec(const std::initializer_list<T>&/*&&*/ il) /*: a_{}*/       \
{                                                                                 \
	DCHECK(il.size() <= N);	/* TODO: DCHECK */                                    \
	auto p = a_.begin(); auto q = il.begin();                                     \
	for (; q != il.end(); ++p, ++q)                                               \
		*p = *q;                                                                  \
}                                                                                 \
constexpr explicit vec(const T& s)  { for (auto& e : a_) e = s; }                 \
constexpr explicit vec(const T* p)  { for (auto& e : a_) e = *p++; }              \
                                                                                  \
iterator               begin()         noexcept { return a_.begin(); }            \
const_iterator         begin()   const noexcept { return a_.begin(); }            \
iterator               end()           noexcept { return a_.end(); }              \
const_iterator         end()     const noexcept { return a_.end(); }              \
                                                                                  \
reverse_iterator       rbegin()        noexcept { return a_.rbegin(); }           \
const_reverse_iterator rbegin()  const noexcept { return a_.rbegin(); }           \
reverse_iterator       rend()          noexcept { return a_.rend(); }             \
const_reverse_iterator rend()    const noexcept { return a_.rend(); }             \
                                                                                  \
const_iterator         cbegin()  const noexcept { return a_.cbegin(); }           \
const_iterator         cend()    const noexcept { return a_.cend(); }             \
const_reverse_iterator crbegin() const noexcept { return a_.crbegin(); }          \
const_reverse_iterator crend()   const noexcept { return a_.crend(); }            \
                                                                                  \
constexpr size_type    size()    const noexcept { return a_.size(); }             \
                                                                                  \
reference       operator[](size_t i)       { return a_[i]; }                      \
const_reference operator[](size_t i) const { return a_[i]; }


template <size_t N_, typename T = float>
struct vec
{
	enum { N = N_ };
	std::array<T, N> a_{};

	FAY_VEC_FUNCTIONS
};

template <typename T>
struct vec<2, T>
{
	enum { N = 2 };
	union
	{
		std::array<T, N> a_{};
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
		std::array<T, N> a_{};
		struct { T x, y, z; };
        struct { T r, g, b; };
	};

	FAY_VEC_FUNCTIONS
};

template <typename T>
struct vec<4, T>
{
	enum { N = 4 };
	union
	{
		std::array<T, N> a_{};
		struct { T x, y, z, w; };
		struct { T r, g, b, a; };
		//struct { vec3 xyz; };
		//struct { vec3 rgb; };
		//struct { vec2 xy; vec2 zw; };
	};

	FAY_VEC_FUNCTIONS
};

using vec2  = vec<2, float>;
using vec3  = vec<3, float>;
using vec4  = vec<4, float>;
using vec2i = vec<2, int>;
using vec3i = vec<3, int>;
using vec4i = vec<4, int>;



// -------------------------------------------------------------------------------------------------
// compare



// TODO: don't support '==', '!=' operators

template <size_t N, typename T>
bool operator==(const vec<N, T>& x, const vec<N, T>& y)
{
	auto p = x.cbegin(), q = y.cbegin();
	for (; p != x.cend(); ++p, ++q)
		if (!is_equal(*p, *q)) 
			return false;
	return true;
}
// TODO
// template <size_t N>
// bool operator==(const vec<N, float>& x, const vec<N, float>& y)
template <size_t N, typename T>
bool operator!=(const vec<N, T>& x, const vec<N, T>& y) { return !(x == y); }



// -------------------------------------------------------------------------------------------------
// arithmetic functions



template <size_t N, typename T>
vec<N, T> operator-(const vec<N, T>& v) { vec<N, T> t; t -= v; return t; } // for NRVO

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
inline vec<N, T> operator +(const vec<N, T>& x, const vec<N, U>& y) { vec<N, T> t(x); t += y; return t; }

template <size_t N, typename T, typename U>
inline vec<N, T>& operator +=(vec<N, T>& x, const U y) { for (auto& e : x) e += y; return x; }
template <size_t N, typename T, typename U>
inline vec<N, T> operator +(const vec<N, T>& x, const U& y) { vec<N, T> t(x); t += y; return t; }
template <size_t N, typename T, typename U>
inline vec<N, T> operator +(const U& x, const vec<N, T>& y) { vec<N, T> t(y); t += x; return t; }
*/

/*
// move to test code
e.g.
vec4 u, v;
u += v;
w = u + v;

float f{};
v += f;
x  = v + f;
y  = f + v;
*/

// std::transform(x.begin(), x.end(), 
//                y.begin(),/* y.end(), */
//                x.begin(), [](const T x, const T y) { return x OP y; });

#define FAY_VEC_ARITHMETIC( OP )                                                                              \
template <size_t N, typename T>                                                                               \
inline vec<N, T>& operator OP##=(vec<N, T>& x, const vec<N, T>& y)                                            \
{                                                                                                             \
	for (size_t i = 0; i < N; ++i)                                                                            \
		x[i] OP##= y[i];                                                                                      \
	return x;                                                                                                 \
}                                                                                                             \
template <size_t N, typename T>                                                                               \
inline vec<N, T> operator OP(const vec<N, T>& x, const vec<N, T>& y) { vec<N, T> r(x); r OP##= y; return r; } \
                                                                                                              \
                                                                                                              \
                                                                                                              \
template <size_t N, typename T> /*, typename U, bool = std::is_arithmetic<U>::value>*/                        \
inline vec<N, T>& operator OP##=(  vec<N, T>& x, const T y)        { for (auto& e : x) e OP##= y; return x; } \
                                                                                                              \
template <size_t N, typename T> /*, typename U, bool = std::is_arithmetic<U>::value>*/                        \
inline vec<N, T> operator OP(const vec<N, T>& x, const T y)          { vec<N, T> r(x); r OP##= y; return r; } \
                                                                                                              \
template <size_t N, typename T> /*, typename U, bool = std::is_arithmetic<U>::value>*/                        \
inline vec<N, T> operator OP(const T x, const vec<N, T>& y)          { vec<N, T> r(x); r OP##= y; return r; }


FAY_VEC_ARITHMETIC( + )
FAY_VEC_ARITHMETIC( - )
FAY_VEC_ARITHMETIC( * )
FAY_VEC_ARITHMETIC( / )

#undef FAY_VEC_FUNCTIONS
#undef FAY_VEC_ARITHMETIC



// -------------------------------------------------------------------------------------------------
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



// -------------------------------------------------------------------------------------------------
// trigonometric



template <size_t N, typename T>
inline vec<N, T> vectorize(const vec<N, T>& v, std::function<T(T)> func)
{
    vec<N, T> r(v);

    for (auto& i : r)
        i = func(i);

    return r;
}

template <size_t N, typename T> inline vec<N, T> radians(const vec<N, T>& v) { return vectorize(v, fay::radians); }
template <size_t N, typename T> inline vec<N, T> degrees(const vec<N, T>& v) { return vectorize(v, fay::degrees); }

template <size_t N, typename T> inline vec<N, T> sin(const vec<N, T>& v) { return vectorize(v, std::sin); }
template <size_t N, typename T> inline vec<N, T> cos(const vec<N, T>& v) { return vectorize(v, std::cos); }
template <size_t N, typename T> inline vec<N, T> tan(const vec<N, T>& v) { return vectorize(v, std::tan); }



// -------------------------------------------------------------------------------------------------
// type



// to_glm

// to_fay



// -------------------------------------------------------------------------------------------------
// output

// TODO: std::format

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
