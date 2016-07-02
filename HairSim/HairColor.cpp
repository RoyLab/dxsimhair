#include <DXUT.h>
#include "HairColor.h"

namespace XRwy
{
    BlackHair::BlackHair(size_t n)
    {
        colorBuffer = new XMFLOAT3[n];
        ZeroMemory(colorBuffer, sizeof(XMFLOAT3)* n);
    }

    BlackHair::~BlackHair()
    {
        SAFE_DELETE_ARRAY(colorBuffer);
    }

    const XMFLOAT3* BlackHair::GetColorArray() const
    {
        return colorBuffer;
    }
}