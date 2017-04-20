// SimHairDllTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DllExports.h"

using namespace XRwy;

int main()
{
	int a = InitializeHairEngine(nullptr, nullptr, nullptr, nullptr);
	int b = UpdateParameter(123, "123", 'a');
	int c = UpdateHairEngine(nullptr, nullptr, nullptr);
	ReleaseHairEngine( );
	int d = GetHairParticleCount();
	int e = GetParticlePerStrandCount();
}

