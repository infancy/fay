#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_MATH_MATRIX_H
#define FAY_MATH_MATRIX_H

namespace fay
{

/*
union
{
	vec<N, vec<M, T>> vec_;
	std::array<T, N*M> vec_;
}
*/

template <int N_, typename T>
struct vec final : arithmetic<vec<N_, T>, T>
{
	enum { N = N_ };
	std::array<T, N> vec_{};

	vec() = default;
	constexpr explicit vec(std::initializer_list<T> il) /*: vec_{}*/
	{
		assert(il.size() <= N);	// TODO: DCHECK
		auto p0 = vec_.begin(); auto p1 = il.begin();
		for (; p1 != il.end(); ++p0, ++p1)
			*p0 = *p1;
	}
	constexpr explicit vec(const T s) { for (auto& e : vec_) e = s; }
	constexpr explicit vec(const T p[]) { for (auto& e : vec_) e = *p++; }

	bool operator==(const vec<N, T>& v) const
	{
		auto p0 = vec_.cbegin(), p1 = v.vec_.cbegin();
		for (; p0 != vec_.cend(); ++p0, ++p1)
			if (!is_equal(*p0, *p1)) return false;
		return true;
	}
	bool operator!=(const vec<N, T>& v) const { return !operator==(v); }

	vec<N, T> operator-() { return vec<N, T>(static_cast<T>(0)) -= *this; }

	vec& operator+=(vec<N, T>& v, const T s) { for (auto& e : v.vec_) e += s; return v; }

	vec& operator+=(vec<N, T>& v0, const vec<N, T>& v1)
	{
		auto p0 = v0.vec_.begin(); auto p1 = v.vec_.cbegin();
		for (; p1 != v.vec_.cend(); ++p0, ++p1)
			*p0 += *p1;
		return v0;
	}


	T& operator[](size_t i) { return vec_[i]; }
	const T& operator[](size_t i) const { return vec_[i]; }
};


} // namespace fay

#endif // FAY_MATH_MATRIX_H
