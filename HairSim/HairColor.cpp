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

    GreyHair::GreyHair(size_t n, char grey)
    {
        colorBuffer = new XMFLOAT3[n];
        for (size_t i = 0; i < n; i++)
            colorBuffer[i] = XMFLOAT3(grey / 255.0f, grey / 255.0f, grey / 255.0f);
    }

    GreyHair::~GreyHair()
    {
        SAFE_DELETE_ARRAY(colorBuffer);
    }

    const XMFLOAT3* GreyHair::GetColorArray() const
    {
        return colorBuffer;
    }
}