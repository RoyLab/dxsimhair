//#include "LevelSet.h"
//
//using namespace WR;
//
//void GenerateGridLevelSet(const char* fileName, float resolution, const char* outputFile);
//int main1()
//{
//	//GenerateGridLevelSet("D:/hair project/models/head.off", 0.02f, "D:/hair project/models/head.grid");
//	runLevelSetBenchMark(L"../../models/head.off");
//	system("pause");
//	return 0;
//}

#include <fstream>
#include <windows.h>
#include <iostream>

#include <CGAL\AABB_tree.h>
#include <CGAL\AABB_triangle_primitive.h>
#include <CGAL\AABB_traits.h>
#include <cgal\Sphere_3.h>
#include <GroupPBD.h>
#include <list>
#include <CholeskyUpdate.hpp>

typedef CGAL::Sphere_3<XRwy::KernelPBD> Sphere;
typedef XRwy::KDSearchPoint Point_3;

int main3()
{
	size_t nParticle;
	float* p;
	std::ifstream f("D:/Data/50k.vertex", std::ios::binary);
	f.read((char*)&nParticle, sizeof(size_t));

	p = new float[3 * nParticle];
	f.read((char*)p, sizeof(float)*3*nParticle);
	f.close();

	LARGE_INTEGER freq, t1, t2;
	QueryPerformanceFrequency(&freq);
	std::vector<Point_3> points;
	for (size_t i = 0; i < nParticle; i++)
	{
		auto pos = p+3*i;
		points.emplace_back(pos[0], pos[1], pos[2], i);
	}


	//for (size_t i = 0; i < nParticle; i++)
	//{
	//	auto pos = p+3*i;
	//	points.emplace_back(pos[0], pos[1], pos[2], i);
	//}

	for (int i = 0; i < 100; i++)
	{
		XRwy::Tree tree;
		QueryPerformanceCounter(&t1);
		tree.insert(points.begin(), points.end());
		tree.build();

		QueryPerformanceCounter(&t2);
		std::cout << "Vertex Count: " << points.size();
		std::cout << " Tree initialization: " << (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart << std::endl;
	}

	system("pause");
	return 0;
}