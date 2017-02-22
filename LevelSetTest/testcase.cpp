#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <iostream>
#include <cstdlib>
#include <time.h>
#include <deque>
#include <cmath>

#include <windows.h>

#include "Octree.hpp"
#include "utils.h"
#include "gridraster.h"

#define XTIMER_INSTANCE
#include "XTimer.hpp"

#include <MatrixFactory.hpp>
#include "SparseCholeskyUpdate.hpp"
#include "CholeskyUpdate.hpp"
#include "xmath.h"



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


bool testall();
bool check_matrix_update();
int profiler_find_all_pair();
bool check_find_all_pairs(double *r, unsigned n, bool = true);
bool check_matrix_update_naive();
bool check_chelosky_update(); // TODO
bool check_spd_lower_matrix();

void hairtest();

int trivial()
{
	M_PI;
	hairtest();
	return 0;
}

namespace logging = boost::log;

void init()
{
	logging::core::get()->set_filter
	(
		logging::trivial::severity >= logging::trivial::info
	);
}

int main(int argc, char** argv)
{
	init();
	trivial();
	//check_spd_lower_matrix();
	//check_chelosky_update();
	//if (check_matrix_update()) cout << "All cases are passed! :-)" << endl;
	//if (testall()) cout << "All cases are passed! :-)" << endl;
	//bool res0,res;
	//double r[] = { 0.01, 0.02, 0.03 };
	//TRUE_OR_ERROR_LOG(check_find_all_pairs, r+1, 1, false);

	system("pause");
	return 0;
}

bool testall()
{
	bool res, res0 = true;;
	double r[] = { 0.01, 0.02, 0.03 };
	TRUE_OR_ERROR_LOG(check_find_all_pairs, r, 3, true);
	TRUE_OR_ERROR_LOG(check_matrix_update);
	return res0;
}


WR::SparseMat genUpperSPD(const int N = 6)
{
	typedef WR::SparseMat Matrix;
	Matrix a(N, N);
	a.setIdentity();

	int pair[][2] = { 1, 2,   3, 4,   1, 5,   0, 3,   0, 2,   2, 3 };
	const int sz = sizeof(pair) / sizeof(int) / 2;
	for (int i = 0; i < sz; i++)
	{
		a.coeffRef(pair[i][0], pair[i][1]) = -1;
		a.coeffRef(pair[i][0], pair[i][0]) += 1;
		a.coeffRef(pair[i][1], pair[i][1]) += 1;
	}
	return a;
}

bool check_chelosky_update()
{
	typedef XR::SparseVector<float> XSparseVec;
	typedef XR::SPDLowerMatrix<float> XSparseMat;
	Eigen::JacobiRotation<float> rot;

	Eigen::SimplicialLLT<WR::SparseMat, Eigen::Upper, Eigen::NaturalOrdering<WR::SparseMat::Index>> llt;
	const int N = 6;

	auto A = genUpperSPD(N);
	float p = 0.6f, q = 0.44f;
	rot.makeGivens(p, q);

	llt.compute(A);
	XSparseMat L;
	WR::SparseMat L0;

	WR::SparseVec v(N);
	v.coeffRef(2) = 1;
	v.coeffRef(4) = -1;

	L = llt.matrixL();
	L0 = llt.matrixL();
	cout.precision(4);

	Eigen::Matrix<float, N, N> Lp = llt.matrixL();

	WR::SparseVec v2 = v;

	sparse_cholesky_update(L, v2);
	v2 = v;
	sparse_cholesky_downdate(L, v2);

	Eigen::Matrix<float, N, 1> v3 = v;
	cholesky_update<N>(Lp, v3);
	v3 = v;
	cholesky_downdate<N>(Lp, v3);

	WR::SparseVec v4 = v;
	sparse_cholesky_update(L0, v4);
	v4 = v;
	sparse_cholesky_downdate(L0, v4);

	//a += v*v.transpose();
	Eigen::MatrixXf LDense;
	XR::assign(LDense, L);
	Eigen::Matrix<float, N, N> diff = LDense - Lp;

	Eigen::MatrixXf recons = (LDense*LDense.transpose()).triangularView<Eigen::Upper>();
	Eigen::MatrixXf gt = A.triangularView<Eigen::Upper>();
	Eigen::MatrixXf diff2 = recons - gt;

	cout << "\nAs follows: \n" << diff.sum() << "\t" << diff2.sum() << std::endl;

	bool res = std::abs(diff.sum()) < 1e-5f && std::abs(diff2.sum()) < 1e-5f;

	cout << LDense << endl;
	//cout << Lp << endl;
	//cout << diff << endl;
	assert(res);

	//apply_jacobi_rotation(0, a, 1, v, rot);
	//apply_jacobi_rotation(ap.col(1).tail(N - 1), vp.tail(N - 1), rot);

	//for (int i = 0; i < 10000; i++)
	//{
	//	if (i % 100 == 0)
	//		XTIMER_HELPER(setClock("a"));
	//	v2 = v;
	//	sparse_cholesky_update(L, v2);
	//	vp3 = v;
	//	sparse_cholesky_downdate(L, vp3);

	//	if (i % 100 == 0)
	//	{
	//		cout << XTIMER_HELPER(milliseconds("a")) << "ms\n";
	//		Eigen::Matrix<float, N, N> diff = L.toDense().cast<float>() - Lp;
	//		cout << "error: " << diff.sum() << endl;
	//	}
	//}
	//cout << tmpa.toDense() << std::endl;
	//cout << SparseMatrix(L*L.transpose()).toDense() << std::endl;

	return res;
}

bool check_matrix_update_naive()
{
	typedef std::vector<uint32_t> IdContainer;
	const int nParticle = 2;
	int gid[nParticle] = { 0,0 };
	XRwy::Hair::MatrixFactory<IdContainer> mf(gid, nParticle, 1);

	float p[nParticle * 3] = { 0,0,-0.1f,0,0,0.1f };
	float r = 0.2f;

	std::vector<Point3f> points;
	for (size_t ii = 0; ii < nParticle; ii++)
	{
		auto pos = p + 3 * ii;
		points.emplace_back(pos[0], pos[1], pos[2]);
	}

	std::vector<uint32_t> id0, id1;
	XRwy::GridRaster<Point3f> pgrid(r);
	pgrid.initialize(points);
	//pgrid.query(id0, id1);

	//mf.update(id0, id1, p, nParticle, r);

	return true;
}

bool check_matrix_update()
{
	const size_t ntest = 30;
	void *tp = malloc(sizeof(char) * 128 * ntest);
	char(*fver)[128] = reinterpret_cast<char(*)[128]>(tp);

	for (int j = 0; j < ntest; j++)
	{
		sprintf(fver[j], "D:/Data/vpos/50kf%03d.vertex", j+1);
	}

	//for (int j = 0; j < ntest / 2; j++)
	//{
	//	sprintf(fver[j+ ntest / 2+1], "D:/Data/vpos/50kf%03d.vertex", ntest / 2-j);
	//}
	//{ "D:/Data/vpos/50kf001.vertex" , "D:/Data/vpos/50kf002.vertex" , "D:/Data/vpos/50kf003.vertex"};


	const char fgroup[] = "E:/c0514/indexcomp1/cg-100.group";

	typedef std::vector<uint32_t> IdContainer;
	XRwy::Hair::MatrixFactory<IdContainer> mf(fgroup, 3, 25);

	std::ifstream f;
	size_t nParticle;
	float* p;
	float r = 0.02f;
	unsigned ncase = sizeof(fver) / sizeof(fver[0]);

	XRwy::GridRaster<Point3f> pgrid(r);
	std::vector<uint32_t> id[8], 
		*id0=&id[0], *id1=&id[1], 
		*old0= &id[2], *old1= &id[3];

	for (int j = 0; j < ntest; j++)
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

		XR::Timer::getTimer().setClock("iterstart");

		BOOST_LOG_TRIVIAL(info) << "Frame: " << j;

		pgrid.initialize(points);
		pgrid.query(*id0, *id1, true, old0, old1, id[4], id[5], id[6], id[7]);

		BOOST_LOG_TRIVIAL(info) << "\tFind pairs " << XTIMER_HELPER(milliseconds("iterstart"));

		//std::set<uint64_t> olda;
		//std::set<uint64_t> newa;
		//for (int i = 0; i < id0->size(); i++)
		//{
		//	uint64_t idx = uint64_t(id0->at(i)) << 32 | id1->at(i);
		//	newa.insert(idx);
		//}
		//for (int i = 0; i < old0->size(); i++)
		//{
		//	uint64_t idx = uint64_t(old0->at(i)) << 32 | old1->at(i);
		//	olda.insert(idx);
		//}

		//// check add
		//size_t c__[6] = { 0 };
		//for (int i = 0; i < id[4].size(); i++)
		//{
		//	uint64_t idx = uint64_t(id[4][i]) << 32 | id[5][i];
		//	if (olda.find(idx) != olda.end())
		//	{
		//		BOOST_LOG_TRIVIAL(error) << 1 << " " << id[4][i] << " " << id[5][i];
		//	}
		//	if (newa.find(idx) == newa.end())
		//		BOOST_LOG_TRIVIAL(error) << 2 << " " << id[4][i] << " " << id[5][i];
		//}

		//// check sub
		//for (int i = 0; i < id[6].size(); i++)
		//{
		//	uint64_t idx = uint64_t(id[6][i]) << 32 | id[7][i];
		//	if (olda.find(idx) == olda.end())
		//		BOOST_LOG_TRIVIAL(error) << 3 << " " << id[6][i] << " " << id[7][i];
		//	if (newa.find(idx) != newa.end())
		//	{
		//		BOOST_LOG_TRIVIAL(error) << 4 << " " << id[6][i] << " " << id[7][i];
		//	}

		//}
		XTIMER_HELPER(setClock("solve"));
		mf.update(*id0, *id1, id[4], id[5], id[6], id[7], p, nParticle, r);

		BOOST_LOG_TRIVIAL(info) << "\tOn update " << XTIMER_HELPER(milliseconds("iterstart"));

		std::swap(id0, old0);
		std::swap(id1, old1);
		id0->clear(); id1->clear();
	}

	return true;
}

bool check_find_all_pairs(double *r, unsigned n, bool check)
{
	const size_t ncase = 50;
	void *tp = malloc(sizeof(char) * 128 * ncase);
	char(*fileName)[128] = reinterpret_cast<char(*)[128]>(tp);

	for (int j = 0; j < ncase; j++)
		sprintf(fileName[j], "D:/Data/vpos/50kf%03d.vertex", j + 1);

	bool res = true;
	for (int i = 0; i < n; i++)
	{
		size_t nParticle;
		float* p;
		std::ifstream f;
		XRwy::GridRaster<Point3f> pgrid(r[i]);
		std::vector<uint32_t> id0, id1, id2, id3, *p0=&id0, *p1=&id1, *o0=&id2, *o1=&id3;
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

			XTIMER_HELPER(setClock("1"));
			pgrid.initialize(points);
			p0->clear();
			p1->clear();
			//pgrid.query(*p0, *p1, true, o0, o1);
			BOOST_LOG_TRIVIAL(info) << "Frame " << j+1 << "\t" <<  XTIMER_HELPER(milliseconds("1"));

			if (check)
			{
				unibn::Octree<Point3f> octree;
				unibn::OctreeParams params(powf(2, 6));
				octree.initialize(points, params);

				int octcount = 0;
				std::vector<uint32_t> results;
				for (uint32_t ii = 0; ii < points.size(); ++ii)
				{
					octree.radiusNeighbors<unibn::L2Distance<Point3f> >(points[ii], r[i], results);
					octcount += results.size();
				}
				octree.clear();

				if (octcount != p0->size())
				{
					char buffer[1024];
					sprintf(buffer, "check_find_all_pairs:%d/%d, octree(gt)/grid:%d/%d, r:%0.2f", i + 1, j + 1, octcount, int(p0->size()), r[i]);
					BOOST_LOG_TRIVIAL(warning) << buffer;
					if (pgrid.checkPairs(*p0, *p1) > 0) res =  false;
				}
			}

			std::swap(p0, o0);
			std::swap(p1, o1);
		}
	}
	return res;
}

int profiler_find_all_pair()
{
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
	pgrid = new XRwy::GridRaster<Point3f>( r);
	for (int i = 0; i < 3; i++)
	{
		cout << "Sequence: " << i << endl;
		double accum = 0.0;
		auto &grid = *pgrid;
		QueryPerformanceCounter(&t1);
		grid.reset();
		//grid.createGrid();
		QueryPerformanceCounter(&t2);
		tmp = (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart;
		std::cout << "Grid construction in " << tmp << std::endl;
		grid.stat();

		std::cout << "Grid construct in ";
		for (int i0 = 0; i0 < 10; i0++)
		{
			QueryPerformanceCounter(&t1);

			grid.reset();
			//grid.createGrid();

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
			//grid.query(id0, id1);
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


int LLTtest()
{

	typedef  Eigen::SimplicialLLT<WR::SparseMat, Eigen::Upper, Eigen::NaturalOrdering<WR::SparseMat::Index>> solver_t;
	typedef  Eigen::SimplicialLLT<WR::SparseMat, Eigen::Upper> solver_t2;

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
	cout << "1: \n" << x << endl;

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

bool check_spd_lower_matrix()
{
	typedef  Eigen::SimplicialLLT<WR::SparseMat, Eigen::Upper, Eigen::NaturalOrdering<WR::SparseMat::Index>> solver_t;

	auto A = genUpperSPD();
	solver_t solver;
	solver.compute(A);

	const size_t sz = A.rows();
	Eigen::VectorXf b(sz), x0;
	b.setRandom();

	x0 = solver.solve(b);

	XR::SPDLowerMatrix<float> L = solver.matrixL();
	L.forwardSubstitution(b);
	L.backwardSubstitution(b);

	cout << "spd solve error: " << (x0 - b).sum() << endl;

	return true;
}
