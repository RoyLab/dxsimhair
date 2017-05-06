#define CGAL_EIGEN3_ENABLED
#define XTIMER_INSTANCE
#define XRWY_EXPORTS
#ifdef V
#undef V
#endif
#include <CGAL/Kd_tree.h>
#include <CGAL/Fuzzy_sphere.h>
#include <CGAL/Search_traits_3.h>

#include <fstream>
#include <tbb/tbb.h>

#include "EigenTypes.h"
#include "wrTripleMatrix.h"

#include "CGALKernel.h"

#define WR_EXPORTS
#include "macros.h"
#include "XConfigReader.hpp"
#include "LevelSet.h"
#include "GroupPBD.h"

#include "XTimer.hpp"

using namespace XR;

namespace XRwy
{
	namespace
	{
		typedef CGAL::Fuzzy_sphere<Traits> Fuzzy_sphere;
		typedef KDSearchPoint Point_3;

		// let the number > i
		void filter(std::list<IntPair>& l)
		{
			for (auto itr = l.begin(); itr != l.end(); itr++)
			{
				while (itr != l.end() && itr->i >= itr->number)
					itr = l.erase(itr);

				if (itr == l.end())
					break;
			}
		}

		void assembleMatrix(std::list<IntPair>& l, std::vector<Point_3>& pts,
			WR::SparseMat& m, WR::VecX& b, float dr)
		{
			size_t dim = 3 * pts.size();
			m.resize(dim, dim);
			b.resize(dim);

			// dummy assemble routine
			//m.setIdentity();

			std::list<Eigen::Triplet<float>> assemble;
			const float balance = 1.0f; // TODO time step related

			for (size_t i = 0; i < pts.size(); i++)
			{
				b[3 * i] = pts[i].x();
				b[3 * i + 1] = pts[i].y();
				b[3 * i + 2] = pts[i].z();
			}

			for (int i = 0; i < dim; i++)
				assemble.push_back(Eigen::Triplet<float>(i, i, 1.0f));

			for (auto& item : l)
			{
				assert(item.i < item.number);
				assemble.push_back(Eigen::Triplet<float>((3 * item.i), (3 * item.number), -balance));
				assemble.push_back(Eigen::Triplet<float>((3 * item.i + 1), (3 * item.number + 1), -balance));
				assemble.push_back(Eigen::Triplet<float>((3 * item.i + 2), (3 * item.number + 2), -balance));

				assemble.push_back(Eigen::Triplet<float>((3 * item.i), (3 * item.i), balance));
				assemble.push_back(Eigen::Triplet<float>((3 * item.i + 1), (3 * item.i + 1), balance));
				assemble.push_back(Eigen::Triplet<float>((3 * item.i + 2), (3 * item.i + 2), balance));

				assemble.push_back(Eigen::Triplet<float>((3 * item.number), (3 * item.number), balance));
				assemble.push_back(Eigen::Triplet<float>((3 * item.number + 1), (3 * item.number + 1), balance));
				assemble.push_back(Eigen::Triplet<float>((3 * item.number + 2), (3 * item.number + 2), balance));

				auto diff = (pts[item.i] - pts[item.number]);
				auto v3 = balance * dr * diff / sqrt(diff.squared_length());
				b[3 * item.i] += v3.x();
				b[3 * item.i + 1] += v3.y();
				b[3 * item.i + 2] += v3.z();
				b[3 * item.number] -= v3.x();
				b[3 * item.number + 1] -= v3.y();
				b[3 * item.number + 2] -= v3.z();
			}
			m.setFromTriplets(assemble.begin(), assemble.end());
		}

		void solveSingleGroupDeprecated(std::vector<Point_3>& points, WR::VecX& x, float dr)
		{
			// build KD-tree
			Tree tree(points.begin(), points.end());

			// find all neighboring points
			std::list<IntPair> output;
			for (size_t i = 0; i < points.size(); i++)
			{
				std::list<IntWrapper> tmps;
				Fuzzy_sphere fs(points[i], dr, 0.3f * dr);
				tree.search(std::back_inserter(tmps), fs);
				for (auto item : tmps)
					output.emplace_back(item.i, i);
			}
			size_t n = points.size();
			filter(output);

			// assemble matrix
			WR::SparseMatAssemble A;
			WR::VecX b;
			assembleMatrix(output, points, A, b, dr);

			Eigen::SimplicialLLT<WR::SparseMat, Eigen::Upper> solver;
			//solver.analyzePattern(A);
			//solver.factorize(A);
			solver.compute(A);
			if (Eigen::Success != solver.info())
			{
				std::cout << "Matrix A is not factorizable." << std::endl;
				std::ofstream f("D:/error.mylog");
				f << A;
				f.close();
				system("pause");
				exit(0);
			}
			x = solver.solve(b);
		}
	}

	extern "C" WR_API IHairCorrection* CreateHairCorrectionObject()
	{
		return new GroupPBD;
	}

	GroupPBD::~GroupPBD()
	{
		SAFE_DELETE_ARRAY(groupIds);
		SAFE_DELETE_ARRAY(groupMatrixMapping);
	}

	bool GroupPBD::initialize(HairGeometry* hair, float dr, float balance, const int* groupInfo, size_t ngi, int nGroup)
	{
		//ConfigReader reader("../config2.ini");
		//reader.getParamDict(g_PBDParas);
		//reader.close();

		this->dr = dr;
		this->nHairParticleGroup = nGroup;
		this->nHairParticle = ngi;
		this->balance = balance;

		bMatrixInited = false;

		rawGroupId.assign(groupInfo, groupInfo + ngi);
		groupIds = new std::deque<int>[nGroup];
		groupMatrixMapping = new std::deque<int>[nGroup];
		matrixSeq.resize(ngi);

		int *counter = new int[nGroup];
		memset(counter, 0, sizeof(int) * nGroup);

		for (size_t i = 0; i < ngi; i++)
		{
			int groupId = groupInfo[i];
			if (i % hair->particlePerStrand != 0)
			{
				matrixSeq[i] = counter[groupId]++;
				groupIds[groupId].push_back(i);
				groupMatrixMapping[groupId].push_back(i);
			}
			else matrixSeq[i] = -1;
		}
		delete[]counter;

		for (size_t i = 0; i < nGroup; i++)
		{
			size_t dim = 3 * groupIds[i].size();
			solverCacheBuffer.emplace_back();
			solverCacheBuffer.back().A.resize(dim, dim);
		}

		return true;
	}

	void GroupPBD::solve(HairGeometry* hair)
	{

		if (hair->nParticle == nHairParticle)
			solveFull(hair);
		else
			solveSampled(hair);
	}

	void GroupPBD::assembleMatA(std::list<IntPair>& oldl, std::list<IntPair>& oldl2, int gId)
	{
		auto& infos = solverCacheBuffer[gId];
		auto& m = infos.A;
		m.setZero();
		std::deque<Eigen::Triplet<float>> assemble;
		const float balance = this->balance;
		for (int i = 0; i < m.rows(); i++)
			assemble.push_back(Eigen::Triplet<float>(i, i, 1.0f));

		for (auto& item : infos.l)
		{
			assert(item.i < item.number);

			int id0 = matrixSeq[item.i];
			int id1 = matrixSeq[item.number];

			assemble.push_back(Eigen::Triplet<float>((3 * id0), (3 * id1), -balance));
			assemble.push_back(Eigen::Triplet<float>((3 * id0 + 1), (3 * id1 + 1), -balance));
			assemble.push_back(Eigen::Triplet<float>((3 * id0 + 2), (3 * id1 + 2), -balance));

			assemble.push_back(Eigen::Triplet<float>((3 * id0), (3 * id0), balance));
			assemble.push_back(Eigen::Triplet<float>((3 * id0 + 1), (3 * id0 + 1), balance));
			assemble.push_back(Eigen::Triplet<float>((3 * id0 + 2), (3 * id0 + 2), balance));

			assemble.push_back(Eigen::Triplet<float>((3 * id1), (3 * id1), balance));
			assemble.push_back(Eigen::Triplet<float>((3 * id1 + 1), (3 * id1 + 1), balance));
			assemble.push_back(Eigen::Triplet<float>((3 * id1 + 2), (3 * id1 + 2), balance));
		}

		for (auto& item : infos.l2)
		{
			assert(!belongTo(item.i, gId) || item.i % 25 == 0);
			int id1 = matrixSeq[item.number];
			assemble.push_back(Eigen::Triplet<float>((3 * id1), (3 * id1), balance));
			assemble.push_back(Eigen::Triplet<float>((3 * id1 + 1), (3 * id1 + 1), balance));
			assemble.push_back(Eigen::Triplet<float>((3 * id1 + 2), (3 * id1 + 2), balance));
		}

		m.setFromTriplets(assemble.begin(), assemble.end());
	}

	void GroupPBD::computeVecb(int gId, XMFLOAT3* pts, float dr, WR::VecX& b)
	{
		auto& particleList = groupIds[gId];
		auto& infos = solverCacheBuffer[gId];
		auto n = particleList.size();
		const float balance = this->balance;

		for (size_t i = 0; i < n; i++)
		{
			auto& point = pts[particleList[i]];
			b[3*i] = point.x;
			b[3*i+1] = point.y;
			b[3*i+2] = point.z;
		}

		for (auto& item : infos.l)
		{
			auto &pi = pts[item.i], &pj = pts[item.number];

			KernelPBD::Point_3 p0(pi.x, pi.y, pi.z);
			KernelPBD::Point_3 p1(pj.x, pj.y, pj.z);
			int id0 = matrixSeq[item.i];
			int id1 = matrixSeq[item.number];

			auto diff = (p0 - p1);
			auto v3 = balance * dr * diff / sqrt(diff.squared_length());

			b[3*id0] +=		v3.x();
			b[3*id0+1] +=	v3.y();
			b[3*id0+2] +=	v3.z();

			b[3*id1] -=		v3.x();
			b[3*id1+1] -=	v3.y();
			b[3*id1+2] -=	v3.z();
		}

		for (auto& item : infos.l2)
		{
			auto &pi = pts[item.i], &pj = pts[item.number];

			KernelPBD::Point_3 p0(pi.x, pi.y, pi.z);
			KernelPBD::Point_3 p1(pj.x, pj.y, pj.z);
			int id1 = matrixSeq[item.number];

			auto diff = (p0 - p1);
			auto v3 = balance * dr * diff / sqrt(diff.squared_length());

			b[3*id1] -=		(v3.x() - balance * pi.x);
			b[3*id1+1] -=	(v3.y() - balance * pi.y);
			b[3*id1+2] -=	(v3.z() - balance * pi.z);
		}
	}

	void GroupPBD::solveSingleGroup(int gId, const Tree* pTree, XMFLOAT3* p0, WR::VecX& x, int pps, bool pass0)
	{
		Eigen::SimplicialLLT<WR::SparseMat, Eigen::Upper>* pSolver = nullptr;
		size_t dim = 3 * groupIds[gId].size();
		if (!bMatrixInited)
		{
			void(0);
		}
		auto &solverCache = solverCacheBuffer[gId];

		if (pass0)
		{
			auto& particleList = groupIds[gId];
			std::list<IntPair> l, l2;

			for (size_t i = 0; i < particleList.size(); i++)
			{
				int particleId = particleList[i]; //global id
				if (particleId % pps == 0) continue; // do not check follicle

				std::list<IntWrapper> tmps;
				auto &v = p0[particleId];
				Fuzzy_sphere fs(Point_3(v.x, v.y, v.z, particleId), dr, 0.5f * dr);
				pTree->search(std::back_inserter(tmps), fs);
				for (auto item : tmps)
				{
					if (item.i % pps != 0 && belongTo(item.i, gId)) // not follicle and belong to group
					{
						if (item.i < particleId)
							l.emplace_back(item.i, particleId);
					}
					else l2.emplace_back(item.i, particleId);
				}
			}

			solverCache.l.swap(l);
			solverCache.l2.swap(l2);
			assembleMatA(l, l2, gId);

			//solver.analyzePattern(A);
			//solver.factorize(A);
			solverCache.solver.compute(solverCache.A);//????????? TODO
		}
		pSolver = &solverCache.solver;

		WR::VecX b;
		b.resize(dim);
		computeVecb(gId, p0, dr, b);

		if (Eigen::Success != pSolver->info())
		{
			// TODO thread lock needed
			std::cout << "Matrix A is not factorizable." << std::endl;
			system("pause");
			exit(0);
		}
		x = pSolver->solve(b);
	}

	void GroupPBD::solveSampled(HairGeometry* hair)
	{
		// import points
		std::vector<Point_3> points;
		for (size_t i = 0; i < hair->nParticle; i++)
		{
			points.emplace_back(hair->position[i].x,
				hair->position[i].y, hair->position[i].z, i);
		}

		WR::VecX x;
		solveSingleGroupDeprecated(points, x, dr);

		// export points
		for (size_t i = 0; i < hair->nParticle; i++)
			hair->position[i] = XMFLOAT3(x[3 * i], x[3 * i + 1], x[3 * i + 2]);
	}

	using namespace tbb;

	class TbbPbdItem {
		XMFLOAT3* pos0;
		XMFLOAT3* newpos;
		float dr;
		const int iPass;

		GroupPBD* thiz;
		const Tree* tree;
		HairGeometry* pHair;


	public:
		TbbPbdItem(GroupPBD* pp, Tree* tr, XMFLOAT3* p0, XMFLOAT3* p1, float d, HairGeometry* hair, int pass):
			tree(tr), iPass(pass)
		{
			pos0 = p0;
			newpos = p1;
			thiz = pp;
			pHair = hair;
			dr = d;
		}

		void operator()(const blocked_range<size_t>& r) const {

			size_t end = r.end();
			for (size_t i = r.begin(); i != end; ++i)
			{
				auto& arr = thiz->groupIds[i];
				WR::VecX x;
				thiz->solveSingleGroup(i, tree, pos0, x, pHair->particlePerStrand, iPass == 0);

				for (size_t i = 0; i < arr.size(); i++)
					newpos[arr[i]] = XMFLOAT3(x[3*i], x[3*i+1], x[3*i+2]);
			}
		}
	};

	void GroupPBD::solveFull(HairGeometry* hair)
	{
		// dump code begin
		//static int count = 0;
		//count++;
		//char fn[128];
		//sprintf(fn, "D:/Data/50kf%03d.vertex", count);
		//std::ofstream f(fn, std::ios::binary);
		//f.write((char*)&hair->nParticle, sizeof(size_t));
		//f.write((char*)hair->position, sizeof(XMFLOAT3)*hair->nParticle);
		//f.close();
		return;
		// dump code end

		// construct tree and use dummy search to init precomputation.
		// if not doing dummy search, the thread safety is not guranteed.
		LARGE_INTEGER freq, t1, t2;
		QueryPerformanceFrequency(&freq);

		std::vector<Point_3> points;
		for (size_t i = 0; i < hair->nParticle; i++)
		{
			auto &pos = hair->position[i];
			points.emplace_back(pos.x, pos.y, pos.z, i);
		}

		Tree tree;
		//for (int i = 0; i < 100; i++)
		//{
			tree.clear();
			QueryPerformanceCounter(&t1);
			//for (int i = 0; i < nHairParticleGroup; i++)
			//{
				//std::vector<Point_3> dummy;
				//Fuzzy_sphere dummyp(Point_3(1e3, 1e3, 1e3, 0), 0, 0);
				//tree.search(std::back_inserter(dummy), dummyp);

				//tree.clear();
				//tree.insert(points.begin() + i * 5000, points.begin() + i * 5000 + 25 * 200);
			//}

			tree.insert(points.begin(), points.end());
			tree.build();

			QueryPerformanceCounter(&t2);
			XLOG_TRACE << "Tree initialization: " << (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart;
			XLOG_TRACE << "Vertex Count: " << points.size();
		//}


		// prepare for the iterations
		XMFLOAT3* allocMem = new XMFLOAT3[hair->nParticle];
		XMFLOAT3 *p0 = hair->position, *p1 = allocMem;

		for (size_t i = 0; i < 3 /* max iteration*/; i++)
		{
			float t = 0.0f;
			std::memcpy(p1, p0, sizeof(XMFLOAT3) * hair->nParticle);
			for (int j = 0; j < nHairParticleGroup; j++)
			{
				WR::VecX x;
				auto& gIds = groupIds[j];

				QueryPerformanceCounter(&t1);
				solveSingleGroup(j, &tree, p0, x, hair->particlePerStrand, i == 0);
				QueryPerformanceCounter(&t2);
				t += (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart;
				for (size_t k = 0; k < gIds.size(); k++)
					p1[gIds[k]] = XMFLOAT3(x[3*k], x[3*k+1], x[3*k+2]);
			}

			//TbbPbdItem tbbCls(this, &tree, p0, p1, dr, hair, i);
			//parallel_for(blocked_range<size_t>(0, nHairParticleGroup, chunksize), tbbCls);

			XLOG_TRACE << "Iteration: " << i << ", Timer: " << t << "/" << t / nHairParticleGroup;

			std::swap(p0, p1);
			if (!bMatrixInited)
				bMatrixInited = true;
		}

		std::list<IntPair> l, l2;
		WR::SparseMat A;
		Eigen::SimplicialLLT<WR::SparseMat, Eigen::Upper> solver;

		if (hair->position != p1)
			std::swap(allocMem, hair->position);

		SAFE_DELETE_ARRAY(allocMem);
	}

	GroupPBD2::GroupPBD2(HairGeometry * hair, float dr, float balance, const int * groupInfo, size_t ngi, int nGroup):
		mf(groupInfo, hair->nParticle, balance, hair->particlePerStrand), pgrid(dr)
	{
		id0 = &id[0]; id1 = &id[1];
		old0 = &id[2]; old1 = &id[3];

		r0 = dr;
	}

	void GroupPBD2::solve(HairGeometry * hair)
	{
		ArrayWrapper<DirectX::XMFLOAT3> wrapper(hair->position, hair->nParticle);
		pgrid.initialize(wrapper);
		pgrid.query(*id0, *id1, true, old0, old1, id[4], id[5], id[6], id[7]);
		mf.update(*id0, *id1, id[4], id[5], id[6], id[7], reinterpret_cast<float*>(hair->position), hair->nParticle, r0);

		std::swap(id0, old0);
		std::swap(id1, old1);
		id0->clear(); id1->clear();
	}
}