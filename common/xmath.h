#pragma once
#include <random>
#include <cmath>
#include "linmath.h"

const float XRM_PI = 3.141592654f;
const float XRM_SMALL = 1.0e-6f;

typedef vec3 mat3x3[3];

static inline void mat3x3_mul_vec3(vec3 r, mat3x3 M, vec3 v)
{
    int i, j;
    for (j = 0; j<3; ++j) {
        r[j] = 0.f;
        for (i = 0; i<3; ++i)
            r[j] += M[i][j] * v[i];
    }
}

static inline void vec3_to_vec4(vec4 r, vec3 v)
{
    for (int i = 0; i < 3; i++)
        r[i] = v[i];
    r[3] = 1.0f;
}

static inline void vec4_to_vec3(vec3 r, vec4 v)
{
    for (int i = 0; i < 3; i++)
        r[i] = v[i];
}

static inline void mat4x4_mul_vec3(vec3 r, mat4x4 M, vec3 v)
{
    vec4 tmp, tmp2;
    vec3_to_vec4(tmp, v);
    mat4x4_mul_vec4(tmp2, M, tmp);
    vec4_to_vec3(r, tmp2);
}

static inline LinType vec3_distance(const LinType* a, const LinType* b)
{
    vec3 diff;
    vec3_sub(diff, a, b);
    return vec3_len(diff);
}

static inline bool vec3_collinear(const LinType* a, const LinType *b)
{
    if (fabs(a[1] * b[2] - a[2] * b[1]) > XRM_SMALL ||
        fabs(a[2] * b[0] - a[0] * b[2]) > XRM_SMALL ||
        fabs(a[0] * b[1] - a[1] * b[0]) > XRM_SMALL) return false;
    return true;
}

static inline LinType vec3_volume(const LinType* a, const LinType *b, const LinType *c, const LinType *d)
{
    vec3 e1, e2, e3;
    vec3_sub(e1, a, b);
    vec3_sub(e2, a, c);
    vec3_sub(e3, a, d);

    vec3 n;
    vec3_mul_cross(n, e1, e2);
    return vec3_mul_inner(n, e3);
}

static inline bool vec3_coplanar(const LinType* a, const LinType *b, const LinType *c, const LinType *d)
{
    vec3 e1, e2, e3;
    vec3_sub(e1, a, b);
    vec3_sub(e2, a, c);
    vec3_sub(e3, a, d);

    vec3 n;
    vec3_mul_cross(n, e1, e2);
    if (fabs(vec3_mul_inner(n, e3)) < XRM_SMALL)
        return true;

    return false;
}

static inline bool vec3_coplanarv(const vec3 (&v)[4])
{
    return vec3_coplanar(v[0], v[1], v[2], v[3]);
}

static inline LinType vec3_volumev(const vec3(&v)[4])
{
    return vec3_volume(v[0], v[1], v[2], v[3]);
}

template <int N>
static inline bool vec3_iszero(const LinType* a)
{
	for (int i = 0; i < N; i++)
    if (fabs(a[0]) < XRM_SMALL &&
        fabs(a[1]) < XRM_SMALL &&
        fabs(a[2]) < XRM_SMALL) return true;
    return false;
}

static inline void vec3_inexact_surface(vec4& r, const vec3& a, const vec3& b, const vec3&c)
{
	vec3 pp1, pp2;
	vec3_sub(pp1, b, a);
	vec3_sub(pp2, c, a);

	vec3_mul_cross(r, pp1, pp2);
	r[3] = -vec3_mul_inner(r, a);
}

static inline void vec3_point_plane_projection(vec3& p, const vec3& s, const vec4& face)
{
	LinType t = s[0] * face[0] + s[1] * face[1] + s[2] * face[2] + face[3];
	LinType square = 0;
	for (size_t i = 0; i < 3; i++)
		square += s[i] * s[i];

	t /= square;
	for (size_t i = 0; i < 3; i++)
		p[i] = s[i] + face[i] * t;
}

static inline float randf()
{
	return (float)rand() / RAND_MAX;
}

static inline double randd()
{
	return (double)rand() / RAND_MAX;
}

static inline float randsf()
{
	return 2.0f * randf() - 1.0f;
}

static inline double randsd()
{
	return 2.0 * randd() - 1.0;
}

namespace XR
{
	// do not check the degenerate case, for triple balance, a, b, c are vector from the center
	static inline void computeWeightForTripleLever(vec3& r, const vec3& a, const vec3& b, const vec3&c)
	{
		r[0] = (b[0] * c[1] - b[1] * c[0]) + (b[1] * c[2] - b[2] * c[1]) + (b[2] * c[0] - b[0] * c[2]);
		r[1] = (c[0] * a[1] - c[1] * a[0]) + (c[1] * a[2] - c[2] * a[1]) + (c[2] * a[0] - c[0] * a[2]);
		r[2] = (a[0] * b[1] - a[1] * b[0]) + (a[1] * b[2] - a[2] * b[1]) + (a[2] * b[0] - a[0] * b[2]);

		float sum = 0.f;
		for (size_t i = 0; i < 3; i++)
			sum += r[i];
		vec3_scale(r, r, 1.0f / sum);
	}

	template <typename T>
	inline int sgn(T val)
	{
		return (T(0) < val) - (val < T(0));
	}

	template <class _matA, class _matB>
	void convert3x3(_matA& a, const _matB& b)
	{
		for (size_t i = 0; i < 3; i++)
			for (size_t j = 0; j < 3; j++)
				a(i, j) = b(i, j);
	}

	template <class _vecA, class _vecB>
	void convert3(_vecA& a, const _vecB& b)
	{
		for (size_t i = 0; i < 3; i++)
			a[i] = b[i];
	}

	template<class _MAT>
	void write(char* fileName, _MAT& mat)
	{
		std::ofstream f(fileName);
		f << mat;
		f.close();
	}
}



