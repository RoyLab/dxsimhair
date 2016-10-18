#include "precompiled.h"
#include "xmath.h"
#include "HairColor.h"

namespace XRwy
{
    void genRandLightColor(float* output)
    {
        for (int i = 0; i < 3; i++)
            output[i] = 0.5f + 0.5f * randf();
    }

    void genRandSaturatedColor(float* output)
    {
        for (int i = 0; i < 3; i++)
            output[i] = randf();

        float minval = 1.0f, maxval = 0.0f;
        int minIdx = 0, maxIdx = 0;

        for (int i = 0; i < 3; i++)
        {
            if (output[i] < minval)
            {
                minval = output[i];
                minIdx = i;
            }

            if (output[i] > maxval)
            {
                maxval = output[i];
                maxIdx = i;
            }
        }
        output[minIdx] = 0.0f;
        output[maxIdx] = 1.0f;
    }

    /***********************************************/
    /************** Black Hair Start ***************/
    /***********************************************/
    BlackHair::BlackHair(size_t n)
    {
        number = n;
        colorBuffer = new XMFLOAT3[n];
        ZeroMemory(colorBuffer, sizeof(XMFLOAT3)* n);
    }

    BlackHair::~BlackHair()
    {
        SAFE_DELETE_ARRAY(colorBuffer);
    }

    /***********************************************/
    /************** Grey Hair Start ***************/
    /***********************************************/

    GreyHair::GreyHair(size_t n, char grey)
    {
        number = n;
        colorBuffer = new XMFLOAT3[n];
        for (size_t i = 0; i < n; i++)
            colorBuffer[i] = XMFLOAT3(grey / 255.0f, grey / 255.0f, grey / 255.0f);
    }

    GreyHair::~GreyHair()
    {
        SAFE_DELETE_ARRAY(colorBuffer);
    }


    /***********************************************/
    /************* Random Hair Start ***************/
    /***********************************************/

    RandomColorHair::RandomColorHair(size_t n, int factor, ColorGenerator func)
    {
        number = n;
        this->factor = factor;
        int nStrand = n / factor;

        colorBuffer = new XMFLOAT3[n];
        float color[3];
        float* ptr = reinterpret_cast<float*>(colorBuffer);
        for (size_t i = 0; i < nStrand; i++)
        {
            func(color);
            for (size_t j = 0; j < factor; j++)
            {
                memcpy(ptr, color, sizeof(float)* 3);
                ptr += 3;
            }
        }
    }

    RandomColorHair::~RandomColorHair()
    {
        SAFE_DELETE_ARRAY(colorBuffer);
    }
}