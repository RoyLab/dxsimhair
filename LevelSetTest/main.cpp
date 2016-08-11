#include "LevelSet.h"

using namespace WR;

void GenerateGridLevelSet(const char* fileName, float resolution, const char* outputFile);
int main()
{
	//GenerateGridLevelSet("D:/hair project/models/head.off", 0.02f, "D:/hair project/models/head.grid");
	runLevelSetBenchMark(L"../../models/head.off");
	system("pause");
	return 0;
}