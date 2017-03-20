#pragma once
#include "xmath.h"

namespace XR
{
	static void genRandLightColor(float* output)
	{
		for (int i = 0; i < 3; i++)
			output[i] = 0.5 + 0.5 * randf();
	}

	static void genRandSaturatedColor(float* output)
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
}



