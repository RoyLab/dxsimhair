#pragma once
#include "linmath.h"
class wrColorGenerator
{
private:
    wrColorGenerator();
    ~wrColorGenerator();

public:
    static void genRandLightColor(float* output);
    static void genRandSaturatedColor(float* output);
};

