#pragma once
#include "HairManager.h"


namespace XRwy
{
    class BlackHair:
        public IHairColorGenerator
    {
    public:
        BlackHair(size_t n);
        ~BlackHair();

        const XMFLOAT3* GetColorArray() const;

    private:
        XMFLOAT3*   colorBuffer = nullptr;
        size_t      number = -1;
    };
}
