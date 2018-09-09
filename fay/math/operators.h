#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_MATH_OPERATORS_H
#define FAY_MATH_OPERATORS_H

namespace fay
{

//template <typename T>
struct operators
{
	// empty
};

/*
TODO:
template <typename T, typename U, typename Base = operators>
struct totally_ordered
{
};
*/

template <typename T, typename Base = operators>
struct totally_ordered : Base
{
	// static_cast...

	// equality_comparable
	friend bool operator!=(const T& x, const T& y) { return !static_cast<bool>(x == y); }
	
	// less_than_comparable
	friend bool operator> (const T& x, const T& y) { return y < x; }
	friend bool operator<=(const T& x, const T& y) { return !static_cast<bool>(y < x); }
	friend bool operator>=(const T& x, const T& y) { return !static_cast<bool>(x < y); }
};

#define FAY_BINARY_OPERATOR( OP )                                             \
friend T operator OP(const T& x, const T& y) { T t(x); t OP##= y; return t; } \
friend T operator OP(const T& x, const U& y) { T t(x); t OP##= y; return t; } \
friend T operator OP(const U& x, const T& y) { T t(y); t OP##= x; return t; }

template <typename T, typename U, typename Base = operators>
struct arithmetic : Base
{
	// static_cast...
	FAY_BINARY_OPERATOR(+);
	FAY_BINARY_OPERATOR(-);
	FAY_BINARY_OPERATOR(*);
	FAY_BINARY_OPERATOR(/);
	// operator%
};

#undef FAY_BINARY_OPERATOR

} // namespace fay

#endif // FAY_MATH_OPERATORS_H

