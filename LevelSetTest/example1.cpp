#include <iostream>
#include <cstdlib>
#include <time.h>

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

  int64_t begin, end;

  XRwy::GridRaster<Point3f> octree(points, 0.03f);
  for (int i0 = 0; i0 < 20; i0++)
  {
	  // initializing the Octree with points from point cloud.
	  octree.reset();
	  begin = clock();
	  octree.createGrid();

	  //unibn::Octree<Point3f> octree;
	  //unibn::OctreeParams params(powf(2, i0+3));
	  //octree.initialize(points, params);
	  end = clock();
	  double search_time = ((double)(end - begin) / CLOCKS_PER_SEC);
	  std::cout << "Construction in " << search_time << " seconds. " << std::endl;
	  //octree.stat();

	  //// radiusNeighbors returns indexes to neighboring points.
	  //std::vector<uint32_t> results;
	  ////const Point3f& q = points[0];
	  ////octree.radiusNeighbors<unibn::L2Distance<Point3f> >(q, 0.2f, results);
	  ////std::cout << results.size() << " radius neighbors (r = 0.2m) found for (" << q.x << ", " << q.y << "," << q.z << ")"
		 //// << std::endl;
	  ////for (uint32_t i = 0; i < results.size(); ++i)
	  ////{
		 //// const Point3f& p = points[results[i]];
		 //// std::cout << "  " << results[i] << ": (" << p.x << ", " << p.y << ", " << p.z << ") => "
			////  << std::sqrt(unibn::L2Distance<Point3f>::compute(p, q)) << std::endl;
	  ////}

	  //// performing queries for each point in point cloud
	  //for (int k = 0; k < 3; k++)
	  //{
		 // begin = clock();
		 // float r = 0.01f*(k + 1);
		 // int count = 0;
		 // for (uint32_t i = 0; i < points.size(); ++i)
		 // {
			//  octree.radiusNeighbors<unibn::L2Distance<Point3f> >(points[i], r, results);
			//  count += results.size();
		 // }
		 // end = clock();
		 // search_time = ((double)(end - begin) / CLOCKS_PER_SEC);
		 // std::cout << "Searching for all radius neighbors (r =" << r << "m) took " << search_time << " seconds." << std::endl;
		 // std::cout << "Size: " << count/(float)nParticle << std::endl;
		 // results.clear();
	  //}
	  //octree.clear();
  }
  system("pause");
  return 0;
}
