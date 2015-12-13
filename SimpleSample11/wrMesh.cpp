#include "DXUT.h"
#include "wrMesh.h"
#include "wrMath.h"

using namespace DirectX;

void CreateSphere(float r, int stacks, int slices, wrSimpleVertexInput** vArr, DWORD** iArr)
{
    const float PI = WR_M_PI;

    wrSimpleVertexInput*& pV = *vArr;
    DWORD*& pI = *iArr;

    int nv = (stacks - 1)*slices + 2;
    int nf = (stacks-1)*slices * 2;

    const int offsetv1 = 1;
    const int offsetv2 = nv - slices - 1;
    const int offseti1 = 3 * slices;
    const int offseti2 = 3 * (nf - slices);

    pV = new wrSimpleVertexInput[nv];
    pV[0].position = XMFLOAT3(0.f, r, 0.f);
    pV[nv - 1].position = XMFLOAT3(0.f, -r, 0.f);

    for (int t = 1; t < stacks; t++) // stacks are ELEVATION so they count theta
    {
        float theta = ((float)(t) / stacks)*PI;
        for (int p = 0; p < slices; p++) // slices are ORANGE SLICES so the count azimuth
        {
            float phi = ((float)(p) / slices) * 2 * PI; // azimuth goes around 0 .. 2*PI
            auto &v = pV[slices * (t-1) + p + offsetv1];
            vec3 pos, norm;
            pos[0] = sinf(theta) * cosf(phi);
            pos[1] = cosf(theta);
            pos[2] = sinf(theta) * sinf(phi);

            vec3_copy(reinterpret_cast<float*>(&v.position), pos);
            vec3_norm(norm, pos);
            vec3_copy(reinterpret_cast<float*>(&v.normal), norm);
        }
    }

    pI = new DWORD[3*nf];
    for (int t = 0; t < slices; t++)
    {
        pI[3 * t] = 0;
        pI[3 * t + 1] = t + offsetv1;
        pI[3 * t + 2] = (t + 1) % slices + offsetv1;

        pI[3 * t + offseti2] = nv - 1;
        pI[3 * t + 1 + offseti2] = (t + 1) % slices + offsetv2;
        pI[3 * t + 2 + offseti2] = t + offsetv2;
    }


    for (int t = 0; t < stacks - 2; t++) // stacks are ELEVATION so they count theta
    {
        auto ptr = pI + t * slices * 6 + offseti1;
        for (int p = 0; p < slices; p++) // slices are ORANGE SLICES so the count azimuth
        {

            //phi1   phi2
            // |      |
            // 1------2 -- theta1
            // |\ _   |
            // |    \ |
            // 4------3 -- theta2
            //

            int v1 = offsetv1 + slices * t + p;
            int v2 = offsetv1 + slices * t + p + 1;
            int v3 = offsetv1 + slices * (t + 1) + p + 1;
            int v4 = offsetv1 + slices * (t + 1) + p;
            
            ptr[0] = v1;    ptr[1] = v3;    ptr[2] = v2;
            ptr[3] = v1;    ptr[4] = v4;    ptr[5] = v3;
            ptr += 6;
        }
    }
}