#pragma once
#include "HairManager.h"

namespace XRwy
{
    typedef void(*ColorGenerator)(float*);

    void genRandLightColor(float* output);
    void genRandSaturatedColor(float* output);

    class BlackHair:
        public IHairColorGenerator
    {
    public:
        BlackHair(size_t n);
        ~BlackHair();

        const XMFLOAT3* GetColorArray() const { return colorBuffer; }

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

        const XMFLOAT3* GetColorArray() const { return colorBuffer; }

    private:
        XMFLOAT3*   colorBuffer = nullptr;
        size_t      number = -1;
    };

    class RandomColorHair:
        public IHairColorGenerator
    {
    public:
        RandomColorHair(size_t n, int factor, ColorGenerator func);
        ~RandomColorHair();

        const XMFLOAT3* GetColorArray() const { return colorBuffer; }

    private:
        XMFLOAT3*   colorBuffer = nullptr;
        size_t      number = -1;
        size_t      factor = -1;
    };
}
