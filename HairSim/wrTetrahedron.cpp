#include "precompiled.h"
#include "wrTetrahedron.h"

namespace
{
    const unsigned edges[6][2] = {
        { 0, 1 }, { 0, 2 }, { 0, 3 },
        { 1, 2 }, { 1, 3 }, { 2, 3 }
    };

    const unsigned edge_edge[3][2] = {
        { 0, 5 }, { 1, 4 }, { 2, 3 }
    };

    const unsigned face_point[4][2] = {
        { 0, 1 }, { 0, 2 }, { 1, 2 }, { 3, 4 }
    };

    const unsigned fp_in[4][3] = {
        { 0, 1, 2 },
        { 0, 1, 3 }, 
        { 0, 2, 3 },
        { 1, 2, 3 },
    };

    const unsigned fp_out[4] = {
        3, 2, 1, 0
    };

    const float K_ALTITUDE_SPRING = 0.e-6f;

}

static inline void computeWeightForPointFaceSpring(vec3& r, const vec3& p, const vec3& a, const vec3& b, const vec3&c)
{
    vec4 face;
    computeSurfaceWithoutCheck(face, a, b, c);

    vec3 center;
    computeProjection(center, p, face);

    vec3 lever[3];
    vec3_sub(lever[0], a, center);
    vec3_sub(lever[1], b, center);
    vec3_sub(lever[2], c, center);

    computeWeightForTripleLever(r, lever[0], lever[1], lever[2]);

    float sum = 0.f;
    for (size_t i = 0; i < 3; i++)
        sum += r[i];
    vec3_scale(r, r, 1.0f / sum);
}

// ---a-----b---
// ---c-----d--- 
static inline void computeWeightForEdgeEdgeSpring(vec4& r, const vec3& a, const vec3& b, const vec3& c, const vec3& d)
{
    vec3 n1, n2;
    vec3_sub(n1, b, a);
    vec3_sub(n2, d, c);

    vec3 C;
    vec3_sub(C, c, a);

    float n1n2 = vec3_mul_inner(n1, n2);
    float n1_2 = vec3_mul_inner(n1, n1);
    float n2_2 = vec3_mul_inner(n2, n2);
    float n1c = vec3_mul_inner(C, n1);
    float n2c = vec3_mul_inner(C, n2);

    float D = n1n2 * n1n2 - n1_2 * n2_2;
    float t1 = (n2_2 * n1c - n1n2 * n2c) / D;
    float t2 = (n1n2 * n1c - n1_2 * n2c) / D;

    assert(t1 < 1.0f && t1 > 0.0f);
    assert(t2 < 1.0f && t2 > 0.0f);

    r[0] = 1.f - t1;
    r[1] = t1;
    r[2] = 1.f - t2;
    r[3] = t2;
}


void wrTetrahedron::init(wrStrand* pStrand, unsigned start)
{
    idx = start;

    vec3 pos[4];
    for (size_t i = 0; i < 4; i++)
        vec3_copy(pos[i], pStrand->particles[idx + i].position);

    for (size_t i = 0; i < 4; i++)
        particles[i] = pStrand->particles + idx + i;

    if (!isCoplanarV(pos))
    {
        float V = volumnV(pos);
        if (V < 0.f) bIsNegativeVolumn = true;
        vec3 area, edge1, edge2;
        for (size_t i = 0; i < 4; i++)
        {
            unsigned ie1 = face_point[i][0];
            vec3_sub(edge1, pos[edges[ie1][0]], pos[edges[ie1][1]]);

            unsigned ie2 = face_point[i][1];
            vec3_sub(edge2, pos[edges[ie2][0]], pos[edges[ie2][1]]);

            vec3_mul_cross(area, edge1, edge2);
            l_fp[i] = V / vec3_len(area);
        }

        for (size_t i = 0; i < 3; i++)
        {
            unsigned ie1 = edge_edge[i][0];
            vec3_sub(edge1, pos[edges[ie1][0]], pos[edges[ie1][1]]);

            unsigned ie2 = edge_edge[i][1];
            vec3_sub(edge2, pos[edges[ie2][0]], pos[edges[ie2][1]]);

            vec3_mul_cross(area, edge1, edge2);
            l_ee[i] = V / vec3_len(area);
        }
    }
    else bIsCoplanar = true;
}

void wrTetrahedron::applySpring(Eigen::MatrixXf& C)
{
    const int offset = -3;

    vec3 area, edge1, edge2;
    vec3 maxU, maxV;
    float maxUxV = 0.0f;
    int maxIdx = -1;
    int max_t = -1;
    for (size_t i = 0; i < 4; i++)
    {
        unsigned ie1 = face_point[i][0];
        vec3_sub(edge1, particles[edges[ie1][0]]->position, 
            particles[edges[ie1][1]]->position);

        unsigned ie2 = face_point[i][1];
        vec3_sub(edge2, particles[edges[ie2][0]]->position,
            particles[edges[ie2][1]]->position);

        vec3_mul_cross(area, edge1, edge2);
        float UxV = vec3_len(area);
        if (UxV > maxUxV)
        {
            maxUxV = UxV;
            vec3_copy(maxU, edge1);
            vec3_copy(maxV, edge2);
            maxIdx = i;
            max_t = 0;
        }
    }

    for (size_t i = 0; i < 3; i++)
    {
        unsigned ie1 = edge_edge[i][0];
        vec3_sub(edge1, particles[edges[ie1][1]]->position,
            particles[edges[ie1][0]]->position);

        unsigned ie2 = edge_edge[i][1];
        vec3_sub(edge2, particles[edges[ie2][1]]->position,
            particles[edges[ie2][0]]->position);

        vec3_mul_cross(area, edge1, edge2);
        float UxV = vec3_len(area);
        if (UxV > maxUxV)
        {
            maxUxV = UxV;
            vec3_copy(maxU, edge1);
            vec3_copy(maxV, edge2);
            maxIdx = i;
            max_t = 1;
        }
    }

    if (bIsCoplanar || maxUxV < WR_M_SMALL)
    {
        if (vec3_len(maxU) > WR_M_SMALL &&
            vec3_len(maxV) > WR_M_SMALL)
        {
            // colinear
        }
        else return;
    }
    else
    {
        // valid altitude
        float V = volumn(particles[0]->position, particles[1]->position,
            particles[2]->position, particles[3]->position);

        float len = V / maxUxV;
        vec3 pos[4];
        for (size_t i = 0; i < 4; i++)
            vec3_copy(pos[i], particles[i]->position);

        if (max_t == 0)
        {
            float force = (len / l_fp[maxIdx] - 1) * K_ALTITUDE_SPRING;
            WR_LOG_TRACE << force;
            vec4 face;
            computeSurfaceWithoutCheck(face, pos[fp_in[maxIdx][0]], pos[fp_in[maxIdx][1]], pos[fp_in[maxIdx][2]]);

            vec3 center;
            computeProjection(center, pos[fp_out[maxIdx]], face);

            vec3 altitude;
            vec3_sub(altitude, pos[fp_out[maxIdx]], center);
            vec3_norm(altitude, altitude);

            vec3 lever[3];
            for (size_t i = 0; i < 3; i++)
                vec3_sub(lever[i], pos[fp_in[maxIdx][i]], center);

            vec3 r;
            computeWeightForTripleLever(r, lever[0], lever[1], lever[2]);

            for (size_t i = 0; i < 3; i++)
            {
                int index = 3 * (idx + fp_out[maxIdx] + offset) + i;
                if (index >= 0)
                    C(index) += force * altitude[i];
            }

            for (size_t i = 0; i < 3; i++)
            {
                for (size_t j = 0; j < 3; j++)
                {
                    int index = 3 * (idx + fp_in[maxIdx][j] + offset) + i;
                    if (index >= 0)
                        C(index) -= force * altitude[i] * r[j];
                }
            }
        }
        else
        {
            vec3 C1;
            vec3_sub(C1, pos[edges[edge_edge[maxIdx][0]][0]], pos[edges[edge_edge[maxIdx][1]][0]]);


            //// for debug
            //vec3 a = { 0, 3, 1 };
            //vec3 b = { 0, -1, 1 };
            //vec3 c = { 1, 0, 0 };
            //vec3 d = { -5, 0, 0 };

            //vec3_sub(maxU, b, a);
            //vec3_sub(maxV, d, c);
            //vec3_sub(C1, a, c);


            float n1n2 = vec3_mul_inner(maxU, maxV);
            float n1_2 = vec3_mul_inner(maxU, maxU);
            float n2_2 = vec3_mul_inner(maxV, maxV);
            float n1c = vec3_mul_inner(C1, maxU);
            float n2c = vec3_mul_inner(C1, maxV);

            float D = n1n2 * n1n2 - n1_2 * n2_2;
            float t1 = (n2_2 * n1c - n1n2 * n2c) / D;
            float t2 = (n1n2 * n1c - n1_2 * n2c) / D;

            assert(t1 < 1.0f && t1 > 0.0f);
            assert(t2 < 1.0f && t2 > 0.0f);

            vec4 r;
            r[0] = 1.f - t1;
            r[1] = t1;
            r[2] = 1.f - t2;
            r[3] = t2;

            // 伸长为正，len可以为负
            float force = (len / l_ee[maxIdx] - 1) * K_ALTITUDE_SPRING;
            WR_LOG_TRACE << force;

            int index[4];
            index[0] = 3 * (idx + edges[edge_edge[maxIdx][0]][0] + offset);
            index[1] = 3 * (idx + edges[edge_edge[maxIdx][0]][1] + offset);
            index[2] = 3 * (idx + edges[edge_edge[maxIdx][1]][0] + offset);
            index[3] = 3 * (idx + edges[edge_edge[maxIdx][1]][1] + offset);

            vec3 altitude;
            vec3_scale(maxU, maxU, t1);
            vec3_scale(maxV, maxV, t2);
            vec3_add(altitude, C1, maxU);
            vec3_sub(altitude, altitude, maxV);
            vec3_norm(altitude, altitude);

            float K[4][3] = { 0 };
            for (size_t i = 0; i < 4; i++)
            {
                if (index[i] >= 0)
                {
                    for (size_t j = 0; j < 3; j++)
                    {
                        C(index[i] + j) += force * r[i] * altitude[j] * ((i > 2.5) ? -1.f : 1.f);
                    }
                }
            }

        }
    }
}