#pragma once

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
	// static_assert...==, <

	// equality_comparable
	friend bool operator!=(const T& x, const T& y) { return !static_cast<bool>(x == y); }
	
	// less_than_comparable
	friend bool operator> (const T& x, const T& y) { return y < x; }
	friend bool operator<=(const T& x, const T& y) { return !static_cast<bool>(y < x); }
	friend bool operator>=(const T& x, const T& y) { return !static_cast<bool>(x < y); }
};



// -------------------------------------------------------------------------------------------------
// arithmetic



/*
#define FAY_BINARY_OPERATOR_T( OP )                                             \
friend T operator OP(const T& x, const T& y) { T t(x); t OP##= y; return t; }   \

#define FAY_BINARY_OPERATOR_U( OP )                                             \
friend T operator OP(const T& x, const U& y) { T t(x); t OP##= y; return t; }   \
friend T operator OP(const U& x, const T& y) { T t(y); t OP##= x; return t; }
*/

#define FAY_BINARY_OPERATOR( OP )                                             \
friend T operator OP(const T& x, const T& y) { T t(x); t OP##= y; return t; } \
friend T operator OP(const U& x, const T& y) { T t(x); t OP##= y; return t; } \
                                                                              \
friend T operator OP(const T& x, const U& y) { T t(x); t OP##= y; return t; }

template <typename T, typename U, typename Base = operators>
struct additive : Base
{
	// static_assert...+=T, -=T, +=U, -=U
	FAY_BINARY_OPERATOR(+);
	FAY_BINARY_OPERATOR(-);
};

template <typename T, typename U, typename Base = operators>
struct multiplicative : Base
{
	// static_assert...*=T, /=T, *=U, /=U
	FAY_BINARY_OPERATOR(*);
	FAY_BINARY_OPERATOR(/);
};

template <typename T, typename U, typename Base = operators>
struct arithmetic : additive<T, U, multiplicative<T, U>>
{
	// static_assert...+=T, -=T, *=T, /=T, +=U, -=U, *=U, /=U
	// operator%
};

#undef FAY_BINARY_OPERATOR

} // namespace fay

