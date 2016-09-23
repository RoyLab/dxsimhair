#include <iostream>
#include <cstdlib>
#include <time.h>
#include <deque>

#include <windows.h>
#include "Octree.hpp"
#include "utils.h"
#include "gridraster.h"

/** Example 1: Searching radius neighbors with default access by public x,y,z variables.
*
* \author behley
*/

using std::cout;
using std::endl;

class Point3f
{
public:
	Point3f(float x, float y, float z) :
		x(x), y(y), z(z)
	{
	}

	float x, y, z;
};

int mai2n()
{
	std::vector<int> a;
	a.reserve(5);
	for (int i = 0; i < 200; i++)
		a.push_back(2);
	return 0;
}

int main(int argc, char** argv)
{
	//if (argc < 2)
	//{
	//  std::cerr << "filename of point cloud missing." << std::endl;
	//  return -1;
	//}
	//std::string filename = argv[1];
	std::string filename = "C:\\Users\\v-war\\Desktop\\wachtberg_folds\\scan_001_points.dat";

	size_t nParticle;
	float* p;
	std::ifstream f("D:/Data/50k.vertex", std::ios::binary);
	f.read((char*)&nParticle, sizeof(size_t));
	p = new float[3 * nParticle];
	f.read((char*)p, sizeof(float) * 3 * nParticle);
	f.close();

	std::vector<Point3f> points;

	for (size_t i = 0; i < nParticle; i++)
	{
		auto pos = p + 3 * i;
		points.emplace_back(pos[0], pos[1], pos[2]);
	}

	//readPoints<Point3f>(filename, points);
	std::cout << "Read " << points.size() << " points." << std::endl;
	if (points.size() == 0)
	{
		std::cerr << "Empty point cloud." << std::endl;
		return -1;
	}

	float r = 0.015f;

	LARGE_INTEGER freq, t1, t2;
	QueryPerformanceFrequency(&freq);
	double tmp;
#ifndef _DEBUG
	unibn::Octree<Point3f> octree;
	unibn::OctreeParams params(powf(2, 6));
	octree.initialize(points, params);
	QueryPerformanceCounter(&t1);
	std::vector<uint32_t> results;
	int count = 0;
	for (uint32_t i = 0; i < points.size(); ++i)
	{
		octree.radiusNeighbors<unibn::L2Distance<Point3f> >(points[i], r, results);
		count += results.size();
	}
	QueryPerformanceCounter(&t2);
	tmp = (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart;
	std::cout << "Octree query in " << tmp << std::endl;
	std::cout << "Result size: " << count << std::endl;
	octree.clear();
#endif


	// 382548 ²»È¥ÖØ
	//std::cout << "???? "<< 1u - 3 << std::endl;
	std::vector<uint32_t> id0, id1;
	XRwy::GridRaster<Point3f> *pgrid;
	pgrid = new XRwy::GridRaster<Point3f>(points, r, 1.01);
	for (int i = 0; i < 3; i++)
	{
		cout << "Sequence: " << i << endl;
		double accum = 0.0;
		auto &grid = *pgrid;
		QueryPerformanceCounter(&t1);
		grid.reset();
		grid.createGrid();
		QueryPerformanceCounter(&t2);
		tmp = (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart;
		std::cout << "Grid construction in " << tmp << std::endl;
		grid.stat();

		std::cout << "Grid construct in ";
		for (int i0 = 0; i0 < 20; i0++)
		{
			QueryPerformanceCounter(&t1);

			grid.reset();
			grid.createGrid();

			QueryPerformanceCounter(&t2);
			auto tmp = (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart;
			accum += tmp;
			if (i0 < 3)
				std::cout << '\t' << tmp;
		}
		std::cout << std::endl;

		std::cout << "Grid query in ";
		for (int i0 = 0; i0 < 20; i0++)
		{
			QueryPerformanceCounter(&t1);

			id0.clear(); id1.clear();
			grid.query(id0, id1);
			//grid.reset();
			//grid.createGrid();

			QueryPerformanceCounter(&t2);
			auto tmp = (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart;
			accum += tmp;
			if (i0 < 3)
			{
				cout << "\t(" << tmp << ", " << id0.size() << ")";
			}
			//uint32_t e = grid.checkPairs(id0, id1);
			//if (e) cout << "!!!!!" << e << endl;

			// dump
			//std::ofstream f("D:/gridpair.dump");
			//for (int i = 0; i < id0.size(); i++)
			//{
			// //f.write((char*)&id0[i], sizeof(uint32_t));
			// //f.write((char*)&id1[i], sizeof(uint32_t));
			// f << id0[i] << '\t' << id1[i] << std::endl;
			//}
			//f.close();
			//exit(0);

			// load
			//std::ifstream f2("D:/gridpair.dump", std::ios::binary);
			//id0.clear(); id1.clear();
			//uint32_t i0, i1;
			//for (int i = 0; i < id0.size(); i++)
			//{
			// f2.read((char*)&i0, sizeof(uint32_t));
			// f2.read((char*)&i1, sizeof(uint32_t));
			// id0.push_back(i0);
			// id1.push_back(i1);
			//}
			//f2.close();

		}
		std::cout << std::endl;
		std::cout << "All time " << accum << std::endl << endl;
		//std::cout << XRwy::stat1 << " ??? " << XRwy::stat2 << std::endl;;
	}
	delete pgrid;
	cout << XRwy::stat1 << " ||| " << XRwy::stat2 << endl;
	system("pause");
	return 0;
}

