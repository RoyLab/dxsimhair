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

class Point3f
{
  public:
    Point3f(float x, float y, float z) :
        x(x), y(y), z(z)
    {
    }

    float x, y, z;
};

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

  float r = 0.03f;

  LARGE_INTEGER freq, t1, t2;
  QueryPerformanceFrequency(&freq);

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
  auto tmp = (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart;
  std::cout << "Octree query in " << tmp << std::endl;
  std::cout << "Result size: " << count << std::endl;
  octree.clear();
#endif

  XRwy::GridRaster<Point3f> grid(points, r);
  grid.reset();
  grid.createGrid();
  grid.stat();
   
  // 382548 ≤ª»•÷ÿ
  //std::cout << "???? "<< 1u - 3 << std::endl;
  double accum = 0.0;
  std::vector<uint32_t> id0, id1;
  for (int i0 = 0; i0 < 10; i0++)
  {
	  QueryPerformanceCounter(&t1);

	  id0.clear(); id1.clear();
	  grid.query(id0, id1);
	  
	  QueryPerformanceCounter(&t2);
	  auto tmp = (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart;
	  accum += tmp;
	  std::cout << "Grid query in " << tmp;
	  std::cout << ". Total Pair: " << id0.size() << std::endl;

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

	  //std::cout << "Problem: " << grid.checkPairs(id0, id1) << std::endl;

	  //// initializing the Octree with points from point cloud.


	  // radiusNeighbors returns indexes to neighboring points.
	  //const Point3f& q = points[0];
	  //octree.radiusNeighbors<unibn::L2Distance<Point3f> >(q, 0.2f, results);
	  //std::cout << results.size() << " radius neighbors (r = 0.2m) found for (" << q.x << ", " << q.y << "," << q.z << ")"
		 // << std::endl;
	  //for (uint32_t i = 0; i < results.size(); ++i)
	  //{
		 // const Point3f& p = points[results[i]];
		 // std::cout << "  " << results[i] << ": (" << p.x << ", " << p.y << ", " << p.z << ") => "
			//  << std::sqrt(unibn::L2Distance<Point3f>::compute(p, q)) << std::endl;
	  //}

	  // performing queries for each point in point cloud
  }
  std::cout << "All time " << accum << std::endl;
  //std::cout << XRwy::stat1 << " ??? " << XRwy::stat2 << std::endl;;

  system("pause");
  return 0;
}
