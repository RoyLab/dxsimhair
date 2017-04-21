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
	fin.open("C:\\Users\\vivid\\Desktop\\newconfig.ini", ios::in);

	fin.read(param.root, 65536);

	InitializeHairEngine(&param, nullptr, nullptr, nullptr);

	float rigid[16];
	int size = GetHairParticleCount();
	float *positions = new float[3 * size];
	float *directions = new float[3 * size];

	UpdateHairEngine(rigid, positions, directions);
	
	ReleaseHairEngine();
}

