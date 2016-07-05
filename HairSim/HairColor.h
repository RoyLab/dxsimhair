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

    class GreyHair :
        public IHairColorGenerator
    {
    public:
        GreyHair(size_t n, char grey);
        ~GreyHair();

        const XMFLOAT3* GetColorArray() const;

    private:
        XMFLOAT3*   colorBuffer = nullptr;
        size_t      number = -1;
    };
}
