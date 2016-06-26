#pragma once
#include <DirectXMath.h>

#include "wrMacro.h"

namespace XRwy
{
    using DirectX::XMFLOAT3;

    struct ParticleGeometry
    {
        XMFLOAT3 position;
        XMFLOAT3 direction;
    };

    struct HairGeometry
    {
        size_t              nParticle;
        size_t              nStrand;
        size_t              particlePerStrand = -1;

        ParticleGeometry*   particles;
        uint32_t*           indices;
    };

    struct HairColorsPerStrand
    {
        size_t nStrand;
        XMFLOAT3* color;

        ~HairColorsPerStrand()
    };
}