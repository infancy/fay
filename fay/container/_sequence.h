#pragma once

#include <array>
#include <initializer_list>

#include "fay/container/array.h"
#include "fay/math/math.h"

namespace fay
{

// seq<Num, Type, Tag>

// TODO: change handmade code with std::algorithm
// std::copy(il.begin(), il.end(), a_.begin());
// std::fill(v.begin(), v.end(), -1);

enum class seq_tag
{
    sequence,
    point,
    vector,
    normal,
    matrix,
    spectrum,
};

template <typename T, size_t N, seq_tag Tag = seq_tag::sequence>
struct seq : array<T, N>
{

};

using seq2  = seq<float, 2>;
using seq3  = seq<float, 3>;
using seq4  = seq<float, 4>;
using seq2i = seq<int, 2>;
using seq3i = seq<int, 3>;
using seq4i = seq<int, 4>;

// -------------------------------------------------------------------------------------------------
// compare



// TODO: don't support '==', '!=' operators

template <typename T, size_t N>
bool operator==(const seq<T, N>& x, const seq<T, N>& y)
{
	auto p = x.cbegin(), q = y.cbegin();
	for (; p != x.cend(); ++p, ++q)
		if (!is_equal(*p, *q)) 
			return false;
	return true;
}
// TODO
// template <size_t N>
// bool operator==(const seq<N, float>& x, const seq<N, float>& y)
template <typename T, size_t N>
bool operator!=(const seq<T, N>& x, const seq<T, N>& y) { return !(x == y); }



// -------------------------------------------------------------------------------------------------
// arithmetic functions



template <typename T, size_t N>
seq<T, N> operator-(const seq<T, N>& v) { seq<T, N> t; t -= v; return t; } // for NRVO

/*
template <typename T, size_t N, typename U>
inline seq<T, N>& operator +=(seq<T, N>& x, const seq<N, U>& y)
{
	auto p = x.begin(); auto q = y.cbegin();
	for (; q != y.cend(); ++p, ++q)
		*p += *q;
	return x;
}
template <typename T, size_t N, typename U>
inline seq<T, N> operator +(const seq<T, N>& x, const seq<N, U>& y) { seq<T, N> t(x); t += y; return t; }

template <typename T, size_t N, typename U>
inline seq<T, N>& operator +=(seq<T, N>& x, const U y) { for (auto& e : x) e += y; return x; }
template <typename T, size_t N, typename U>
inline seq<T, N> operator +(const seq<T, N>& x, const U& y) { seq<T, N> t(x); t += y; return t; }
template <typename T, size_t N, typename U>
inline seq<T, N> operator +(const U& x, const seq<T, N>& y) { seq<T, N> t(y); t += x; return t; }
*/

/*
// move to test code
e.g.
seq4 u, v;
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

#define FAY_SEQ_ARITHMETIC( OP )                                                                              \
template <typename T, size_t N>                                                                               \
inline seq<T, N>& operator OP##=(seq<T, N>& x, const seq<T, N>& y)                                            \
{                                                                                                             \
	for (size_t i = 0; i < N; ++i)                                                                            \
		x[i] OP##= y[i];                                                                                      \
	return x;                                                                                                 \
}                                                                                                             \
template <typename T, size_t N>                                                                               \
inline seq<T, N> operator OP(const seq<T, N>& x, const seq<T, N>& y) { seq<T, N> r(x); r OP##= y; return r; } \
                                                                                                              \
                                                                                                              \
                                                                                                              \
template <typename T, size_t N> /*, typename U, bool = std::is_arithmetic<U>::value>*/                        \
inline seq<T, N>& operator OP##=(  seq<T, N>& x, const T y)        { for (auto& e : x) e OP##= y; return x; } \
                                                                                                              \
template <typename T, size_t N> /*, typename U, bool = std::is_arithmetic<U>::value>*/                        \
inline seq<T, N> operator OP(const seq<T, N>& x, const T y)          { seq<T, N> r(x); r OP##= y; return r; } \
                                                                                                              \
template <typename T, size_t N> /*, typename U, bool = std::is_arithmetic<U>::value>*/                        \
inline seq<T, N> operator OP(const T x, const seq<T, N>& y)          { seq<T, N> r(x); r OP##= y; return r; }


FAY_SEQ_ARITHMETIC( + )
FAY_SEQ_ARITHMETIC( - )
FAY_SEQ_ARITHMETIC( * )
FAY_SEQ_ARITHMETIC( / )

#undef FAY_SEQ_FUNCTIONS
#undef FAY_SEQ_ARITHMETIC



// -------------------------------------------------------------------------------------------------
// named functions



/*

template <typename T, size_t N>
T length(const seq<T, N>& v)
{
	//return sqrtf(x * x + y * y + z * z + w * w);
}

template <typename T, size_t N>
T square(const seq<T, N>& v)
{
	//return x * x + y * y + z * z + w * w;
}

template <typename T, size_t N>
seq<T, N>& normalize(const seq<T, N>& v)
{
	v *= T(1) / length(v);
	return v;
}

template <typename T, size_t N>
void sqrt(const seq<T, N>& v)
{
	//
}
*/

// x dot y != x * y
template <typename T, size_t N>
T dot(const seq<T, N>& v0, const seq<T, N>& v1)
{
	T r{};

	for (size_t i = 0; i < N; ++i)
		r += v0[i] * v1[i];

	return r;
}

/*
// only for N=3
template <typename T>
seq<3, T>& cross(const seq<3, T>& v0, const seq<3, T>& v1)
{
	return seq<3, T>();
}

template <typename T>
seq<3, T>& cross(const seq<4, T>& v0, const seq<4, T>& v1)
{
	return seq<4, T>(
		v0.w * v1.x + v0.x * v1.w - v0.z * v1.y + v0.y * v1.z,
		v0.w * v1.y + v0.y * v1.w - v0.x * v1.z + v0.z * v1.x,
		v0.w * v1.z + v0.z * v1.w - v0.y * v1.x + v0.x * v1.y,
		v0.w * v1.w - v0.x * v1.x - v0.y * v1.y - v0.z * v1.z);
}
*/



// -------------------------------------------------------------------------------------------------
// trigonometric

// template<Ragne range, Function func>
// constexpr range vectorize(const range& rg, func f)

// template <
// 	   typename T, size_t N, 
//     template<R, Args...> typename F>
template <typename T, size_t N>
inline seq<T, N> vectorize(const seq<T, N>& s, std::function<T(T)> f)
{
    seq<T, N> r(s);

    for (auto& v : r)
        v = f(v);

    return r;
}

// TODO: macro

template <typename T, size_t N> inline seq<T, N> radians(const seq<T, N>& v) { return vectorize(v, fay::radians); }
template <typename T, size_t N> inline seq<T, N> degrees(const seq<T, N>& v) { return vectorize(v, fay::degrees); }

template <typename T, size_t N> inline seq<T, N> sin(const seq<T, N>& v) { return vectorize(v, std::sin); }
template <typename T, size_t N> inline seq<T, N> cos(const seq<T, N>& v) { return vectorize(v, std::cos); }
template <typename T, size_t N> inline seq<T, N> tan(const seq<T, N>& v) { return vectorize(v, std::tan); }



// -------------------------------------------------------------------------------------------------
// typecast



// to_glm

// to_fay



// -------------------------------------------------------------------------------------------------
// output

// TODO: std::format

template <typename T, size_t N> // typename = std::enable_if_t<!std::is_floating_point_v<T>, void>>
inline std::ostream& operator<<(std::ostream& os, const seq<T, N>& v)
{
	os << "[ ";
	for (auto e : v) os << e << ", ";
	os << " ]";
	return os;
}


template <size_t N> // typename T, typename = std::enable_if_t<std::is_floating_point_v<T>, void>>
inline std::ostream& operator<<(std::ostream& os, const seq<float, N>& v)	// TODO: float_point
{
	os << "[ ";
	for (auto e : v) os << e << ", ";	// TODO
	// TODO: putback(", ")
	os << " ]";
	return os;
}



// -------------------------------------------------------------------------------------------------
// chain call

// d(c(b(a(...))))   vs   ... | a | b | c | d

} // namespace fay
