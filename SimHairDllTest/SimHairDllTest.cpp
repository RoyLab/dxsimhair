// SimHairDllTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DllExports.h"
#include <string>

using namespace XRwy;
using namespace std;

int main()
{
	HairParameter param;
	string test_string =  "this = that\n#this is a comment\nyu=po";
	for (int i = 0; i < test_string.size(); ++i)
		param.root[i] = test_string[i];
	param.root[test_string.size()] = 0;

	int a = InitializeHairEngine(&param, nullptr, nullptr, nullptr);
}

