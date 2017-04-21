// SimHairDllTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DllExports.h"
#include <string>
#include <fstream>

using namespace XRwy;
using namespace std;

void apply_param_config(HairParameter &param, const string &conf) {
	for (int i = 0; i < conf.size(); ++i)
		param.root[i] = conf[i];
	param.root[conf.size()] = 0;
}

int main()
{
	HairParameter param;

	ifstream fin;
	fin.open("../newconfig.ini", ios::in);

	fin.read(param.root, 65536);

	InitializeHairEngine(&param, nullptr, nullptr, nullptr);

	float rigid[16];
	float *positions = new float[3 * GetHairParticleCount()];
	float *directions = new float[3 * GetHairParticleCount()];

	UpdateHairEngine(rigid, positions, directions);
	
	ReleaseHairEngine();
}

