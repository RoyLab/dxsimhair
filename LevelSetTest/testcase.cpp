#include <boost/log/trivial.hpp>
#include <iostream>
#include <cstdlib>
#include <time.h>
#include <deque>

#include <windows.h>
#include "Octree.hpp"
#include "utils.h"
#include "gridraster.h"
#include <MatrixFactory.hpp>

// macros
#define TRUE_OR_ERROR_LOG(func, ...) {if (!(res = func(__VA_ARGS__))) {BOOST_LOG_TRIVIAL(error) << "Test case not pass: " << #func;}\
	else {BOOST_LOG_TRIVIAL(info) << "Test case passed: " << #func;} res0 &= res;}


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

typedef  Eigen::SimplicialLLT<WR::SparseMat, Eigen::Upper, Eigen::NaturalOrdering<WR::SparseMat::Index>> solver_t;
typedef  Eigen::SimplicialLLT<WR::SparseMat, Eigen::Upper> solver_t2;

int trivial()
{
	solver_t *solver = new solver_t;
	Eigen::VectorXf m(4), x(4);
	WR::SparseMat *m2 = new std::remove_reference<decltype(*m2)>::type(4, 4);
	m2->setIdentity();
	m2->coeffRef(0, 2) = 0.5;

	decltype(m.data()) a(0);

	auto k = m2->triangularView<Eigen::Upper>();
	m2->coeffRef(0, 0) = 9;

	cout.precision(2);
	cout.width(8);

	solver->compute(*m2);
	assert(solver->info() == Eigen::Success);
	WR::SparseMat k2x = solver->matrixL();
	auto k2 = k2x.triangularView<Eigen::Lower>();

	cout << *m2 << endl;
	cout << "L: \n" << k2 << endl;

	m << 1, 1, 1, 1;

	x = solver->solve(m);
	cout << "1: \n" <<  x << endl;

	k2.solveInPlace(m);
	auto kv = k2x.transpose().triangularView<Eigen::Upper>();
	kv.solveInPlace(m);
	cout << "2: \n" << m << endl;

	k2x.resize(2, 2);
	k2x.setIdentity();
	k2x.coeffRef(1, 0) = 20;
	cout << k2 << kv << endl;

	return 0;
}

bool testall();
bool check_matrix_update();
int profiler_find_all_pair();
bool check_find_all_pairs(double *r, unsigned n);

int main(int argc, char** argv)
{
	//trivial();
	
	if (check_matrix_update()) cout << "All cases are passed! :-)" << endl;
	//if (testall()) cout << "All cases are passed! :-)" << endl;

	system("pause");
	return 0;
}

bool testall()
{
	bool res, res0 = true;;
	double r[] = { 0.01, 0.02, 0.03 };
	TRUE_OR_ERROR_LOG(check_find_all_pairs, r, 3);
	TRUE_OR_ERROR_LOG(check_matrix_update);
	return res0;
}

bool check_matrix_update()
{
	const char fver[][128] = { "D:/Data/vpos/50k.vertex" , "D:/Data/vpos/50kf10.vertex" , "D:/Data/vpos/50kf20.vertex"};
	const char fgroup[] = "E:/c0514/indexcomp1/cg-100.group";

	typedef std::vector<uint32_t> IdContainer;
	XRwy::Hair::MatrixFactory<IdContainer> mf(fgroup, 1, 25);

	std::ifstream f;
	size_t nParticle;
	float* p;
	float r = 0.02f;
	unsigned ncase = sizeof(fver) / sizeof(fver[0]);
	for (int j = 0; j < 1; j++)
	{
		f.open(fver[j], std::ios::binary);
		f.read((char*)&nParticle, sizeof(size_t));
		p = new float[3 * nParticle];
		f.read((char*)p, sizeof(float) * 3 * nParticle);
		f.close();

		std::vector<Point3f> points;
		for (size_t ii = 0; ii < nParticle; ii++)
		{
			auto pos = p + 3 * ii;
			points.emplace_back(pos[0], pos[1], pos[2]);
		}

		std::vector<uint32_t> id0, id1;
		XRwy::GridRaster<Point3f> pgrid(points, r, 1.01);
		pgrid.createGrid();
		pgrid.query(id0, id1);

		mf.update(id0, id1, p, nParticle, r);
	}

	return true;
}

bool check_find_all_pairs(double *r, unsigned n)
{
	char fileName[][128] = { "D:/Data/vpos/50k.vertex" , "D:/Data/vpos/50kf10.vertex" , "D:/Data/vpos/50kf20.vertex" };
	unsigned ncase = sizeof(fileName) / sizeof(fileName[0]);
	bool res = true;
	for (int i = 0; i < n; i++)
	{
		size_t nParticle;
		float* p;
		std::ifstream f;
		for (int j = 0; j < ncase; j++)
		{
			f.open(fileName[j], std::ios::binary);
			f.read((char*)&nParticle, sizeof(size_t));
			p = new float[3 * nParticle];
			f.read((char*)p, sizeof(float) * 3 * nParticle);
			f.close();

			std::vector<Point3f> points;
			for (size_t ii = 0; ii < nParticle; ii++)
			{
				auto pos = p + 3 * ii;
				points.emplace_back(pos[0], pos[1], pos[2]);
			}

			unibn::Octree<Point3f> octree;
			unibn::OctreeParams params(powf(2, 6));
			octree.initialize(points, params);
			std::vector<uint32_t> results;
			int octcount = 0;
			for (uint32_t ii = 0; ii < points.size(); ++ii)
			{
				octree.radiusNeighbors<unibn::L2Distance<Point3f> >(points[ii], r[i], results);
				octcount += results.size();
			}
			octree.clear();

			std::vector<uint32_t> id0, id1;
			XRwy::GridRaster<Point3f> pgrid(points, r[i], 1.01);
			pgrid.createGrid();
			pgrid.query(id0, id1, false);

			if (octcount != id0.size())
			{
				char buffer[1024];
				sprintf(buffer, "check_find_all_pairs:%d/%d, octree(gt)/grid:%d/%d, r:%0.2f", i+1, j+1, octcount, int(id0.size()), r[i]);
				BOOST_LOG_TRIVIAL(warning) << buffer;
				if (pgrid.checkPairs(id0, id1) > 0) res =  false;
			}
		}
	}
	return res;
}

int profiler_find_all_pair()
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

	float r = 0.02f;

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

	// 382548 ��ȥ��
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
		for (int i0 = 0; i0 < 10; i0++)
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
		for (int i0 = 0; i0 < 10; i0++)
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

