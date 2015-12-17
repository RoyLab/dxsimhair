#include "precompiled.h"
#include "wrColorGenerator.h"
#include "wrMath.h"


wrColorGenerator::wrColorGenerator()
{
}


wrColorGenerator::~wrColorGenerator()
{
}


void wrColorGenerator::genRandLightColor(float* output)
{
    for (int i = 0; i < 3; i++)
        output[i] = 0.5 + 0.5 * randf();
}


void wrColorGenerator::genRandSaturatedColor(float* output)
{
    for (int i = 0; i < 2; i++)
        output[i] = randf();
    output[2] = 1.5f - output[0] - output[1];
}
