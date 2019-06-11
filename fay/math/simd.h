#pragma once

#include "fay/math/math.h"

#if defined(FAY_IN_MSVC)
#include <float.h>
#include <intrin.h>
#elif
#error "fay/math/math.h error in *nix"
#endif

namespace fay
{

struct vec4
{
	union
	{
	#ifdef FAY_SIMD
		__m128 m128;
	#endif
		float m[4];
		struct { float x, y, z, w; };
		struct { float r, g, b, a; };
		//struct { vec3 xyz; };
		//struct { vec3 rgb; };
		//struct { vec2 xy; vec2 zw; };
	};

	vec4() { x = y = z = w = 0.f; /* memset(m, 0.f, sizeof(m[0])); */ }
	vec4(const float s)	// scalar
	{
	#ifdef FAY_SIMD
		m128 = _mm_set_ps1(s);
	#else
		x = s;
		y = s;
		z = s;
		w = s;
	#endif
	}
	vec4(const float sx, const float sy, const float sz, const float sw)
	{
	#ifdef FAY_SIMD
		m128 = _mm_set_ps(sw, sz, sy, sx);
	#else
		x = sx;
		y = sy;
		z = sz;
		w = sw;
	#endif
	}
	vec4(const float* p)
	{
	#ifdef FAY_SIMD
		m128 = _mm_loadr_ps(p);
	#else
		x = p[0];
		y = p[1];
		z = p[2];
		w = p[3];
	#endif
	}

	bool operator==(const vec4& v) const
	{
		if (is_equal(x, v.x) && is_equal(y, v.y) && is_equal(z, v.z) && is_equal(w, v.w))
			return true;
		return false;
	}
	bool operator!=(const vec4& v) const
	{
		return !operator==(v);
	}

	vec4& operator+=(const vec4& v)
	{
	#ifdef FAY_SIMD
		m128 = _mm_add_ps(m128, v.m128);
	#else
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;
	#endif
		return *this;
	}
	vec4& operator-=(const vec4& v)
	{
	#ifdef FAY_SIMD
		m128 = _mm_sub_ps(m128, v.m128);
	#else
		x -= v.x;
		y -= v.y;
		z -= v.z;
		w -= v.w;
	#endif
		return *this;
	}
	vec4& operator^=(const vec4& v)
	{
		vec4 tmp(
			w * v.x + x * v.w - z * v.y + y * v.z,
			w * v.y + y * v.w - x * v.z + z * v.x,
			w * v.z + z * v.w - y * v.x + x * v.y,
			w * v.w - x * v.x - y * v.y - z * v.z);
		x = tmp.x;
		y = tmp.y;
		z = tmp.z;
		w = tmp.w;
		return *this;
	}
	vec4& operator*=(const vec4& v)
	{
	#ifdef FAY_SIMD
		m128 = _mm_mul_ps(m128, v.m128);
	#else
		x *= v.x;
		y *= v.y;
		z *= v.z;
		w *= v.w;
	#endif
		return *this;
	}
	vec4& operator*=(const float s)
	{
	#ifdef FAY_SIMD
		__m128 mul = _mm_set_ps1(s);
		m128 = _mm_mul_ps(m128, mul);
	#else
		x *= s;
		y *= s;
		z *= s;
		w *= s;
	#endif
		return *this;
	}
	vec4& operator/=(const float s)
	{
	#ifdef FAY_SIMD
		__m128 div = _mm_set_ps1(s);
		m128 = _mm_div_ps(m128, div);
	#else
		x /= s;
		y /= s;
		z /= s;
		w /= s;
	#endif		
		return *this;
	}

	vec4 operator+(const vec4& v) const
	{
	#ifdef FAY_SIMD
		vec4 tmp;
		tmp.m128 = _mm_add_ps(m128, v.m128);
	#else
		vec4 tmp(x + v.x, y + v.y, z + v.z, w + v.w);
	#endif
		return tmp;
	}
	vec4 operator-(const vec4& v) const
	{
	#ifdef FAY_SIMD
		vec4 tmp;
		tmp.m128 = _mm_sub_ps(m128, v.m128);
	#else
		vec4 tmp(x - v.x, y - v.y, z - v.z, w - v.w);
	#endif		
		return tmp;
	}
	vec4 operator^(const vec4& v) const
	{
		vec4 tmp(
			w * v.x + x * v.w - z * v.y + y * v.z,
			w * v.y + y * v.w - x * v.z + z * v.x,
			w * v.z + z * v.w - y * v.x + x * v.y,
			w * v.w - x * v.x - y * v.y - z * v.z);
		return tmp;
	}
	vec4 operator*(const vec4& v) const
	{
	#ifdef FAY_SIMD
		vec4 tmp;
		tmp.m128 = _mm_mul_ps(m128, v.m128);
	#else
		vec4 tmp(x * v.x, y * v.y, z * v.z, w * v.w);
	#endif
		return tmp;
	}
	vec4 operator/(const float s) const
	{
	#ifdef FAY_SIMD
		vec4 tmp;
		tmp.m128 = _mm_set_ps1(s);
		tmp.m128 = _mm_div_ps(m128, tmp.m128);
	#else
		vec4 tmp(x / s, y / s, z / s, w / s);
	#endif		
		return tmp;
	}

	vec4 operator*(const float s) const
	{
	#ifdef FAY_SIMD
		vec4 tmp;
		tmp.m128 = _mm_set_ps1(s);
		tmp.m128 = _mm_mul_ps(m128, tmp.m128);
	#else
		vec4 tmp(x * s, y * s, z * s, w * s);
	#endif
		return tmp;
	}

	float length() const
	{
		return sqrtf(x * x + y * y + z * z + w * w);
	}
	float lengthsq() const
	{
		return x * x + y * y + z * z + w * w;
	}
	vec4& normalize()
	{
		*this *= 1.f / length();
		return *this;
	}
	void sqrt()
	{
	#ifdef FAY_SIMD
		m128 = _mm_sqrt_ps(m128);
	#else
		x = sqrtf(x);
		y = sqrtf(y);
		z = sqrtf(z);
		w = sqrtf(w);
	#endif
	}
	float dot(const vec4& v)
	{
		return x * v.x + y * v.y + z * v.z + w * v.w;
	}
};

float dot(const vec4& lhs, const vec4& rhs)
{
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}

} // namespace fay