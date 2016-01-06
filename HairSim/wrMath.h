#pragma once
#include <random>
#include "linmath.h"
#include <cmath>

const float WR_M_PI = 3.141592654f;
const float WR_M_SMALL = 1.0e-6f;

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

static inline float randf()
{
    return (float)rand() / RAND_MAX;
}

static inline float randSignedFloat()
{
    return 2.0f * randf() - 1.0f;
}

static inline float computeDistance(const float* a, const float* b)
{
    vec3 diff;
    vec3_sub(diff, a, b);
    return vec3_len(diff);
}

static inline bool isCollinear(const float* a, const float *b)
{
    if (fabs(a[1] * b[2] - a[2] * b[1]) > WR_M_SMALL ||
        fabs(a[2] * b[0] - a[0] * b[2]) > WR_M_SMALL ||
        fabs(a[0] * b[1] - a[1] * b[0]) > WR_M_SMALL) return false;
    return true;
}

static inline float volumn(const float* a, const float *b, const float *c, const float *d)
{
    vec3 e1, e2, e3;
    vec3_sub(e1, a, b);
    vec3_sub(e2, a, c);
    vec3_sub(e3, a, d);

    vec3 n;
    vec3_mul_cross(n, e1, e2);
    return vec3_mul_inner(n, e3);
}

static inline bool isCoplanar(const float* a, const float *b, const float *c, const float *d)
{
    vec3 e1, e2, e3;
    vec3_sub(e1, a, b);
    vec3_sub(e2, a, c);
    vec3_sub(e3, a, d);

    vec3 n;
    vec3_mul_cross(n, e1, e2);
    if (fabs(vec3_mul_inner(n, e3)) < WR_M_SMALL)
        return true;

    return false;
}

static inline bool isCoplanarV(const vec3 (&v)[4])
{
    return isCoplanar(v[0], v[1], v[2], v[3]);
}

static inline float volumnV(const vec3(&v)[4])
{
    return volumn(v[0], v[1], v[2], v[3]);
}


static inline bool isZero(const float* a)
{
    if (fabs(a[0]) < WR_M_SMALL &&
        fabs(a[1]) < WR_M_SMALL &&
        fabs(a[2]) < WR_M_SMALL) return true;
    return false;
}

static inline void computeSurfaceWithoutCheck(vec4& r, const vec3& a, const vec3& b, const vec3&c)
{
    vec3 pp1, pp2;
    vec3_sub(pp1, b, a);
    vec3_sub(pp2, c, a);

    vec3_mul_cross(r, pp1, pp2);
    r[3] = -vec3_mul_inner(r, a);
}

static inline void computeProjection(vec3& p, const vec3& s, const vec4& face)
{
    float t = s[0] * face[0] + s[1] * face[1] + s[2] * face[2] + face[3];
    float square = 0;
    for (size_t i = 0; i < 3; i++)
        square += s[i] * s[i];

    t /= square;
    for (size_t i = 0; i < 3; i++)
        p[i] = s[i] + face[i] * t;
}

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

