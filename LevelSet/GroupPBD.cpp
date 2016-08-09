#define CGAL_EIGEN3_ENABLED
#include <CGAL/Kd_tree.h>
#include <CGAL/Fuzzy_sphere.h>
#include <CGAL/Search_traits_3.h>

#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>
#include <fstream>

#include <tbb/tbb.h>

#include "EigenTypes.h"
#include "wrTripleMatrix.h"

#include "CGALKernel.h"

#define WR_EXPORTS
#include "macros.h"
#include "LevelSet.h"
#include "GroupPBD.h"


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
	}

	bool GroupPBD::initialize(HairGeometry* hair, float dr, const int* groupInfo, size_t ngi, int nGroup)
	{
		this->nWorker = 1;
		this->dr = dr;
		this->nHairParticleGroup = nGroup;

		rawGroupId.assign(groupInfo, groupInfo + ngi);
		groupIds = new std::vector<int>[nGroup];
		int *counter = new int[nGroup];
		memset(counter, 0, sizeof(int) * nGroup);
		matrixSeq.resize(ngi);

		for (size_t i = 0; i < ngi; i++)
		{
			int id = groupInfo[i];
			if (id % hair->particlePerStrand != 0)
			{
				matrixSeq[id] = counter[rawGroupId[id]]++;
				groupIds[groupInfo[i]].push_back(i);
			}
			else matrixSeq[id] = -1;
		}

		nHairParticle = ngi;
		return true;
	}

	void GroupPBD::solve(HairGeometry* hair)
	{

		if (hair->nParticle == nHairParticle)
			solveFull(hair);
		else
			solveSampled(hair);
	}

	void GroupPBD::assembleMatrix(std::list<IntPair>& l, std::list<IntPair>& l2,
		XMFLOAT3* pts, int gId, WR::VecX& b, float dr)
	{
		auto& m = buffer[gId];
		auto& particleList = groupIds[gId];
		size_t dim = 3 * particleList.size();

		m.setZero();
		m.resize(dim, dim);
		b.resize(dim);

		std::list<Eigen::Triplet<float>> assemble;
		const float balance = 1.0f; // TODO time step related

		for (size_t i = 0; i < particleList.size(); i++)
		{
			b[3 * i] = pts[particleList[i]].x;
			b[3 * i + 1] = pts[particleList[i]].y;
			b[3 * i + 2] = pts[particleList[i]].z;
		}

		for (int i = 0; i < dim; i++)
			assemble.push_back(Eigen::Triplet<float>(i, i, 1.0f));

		for (auto& item : l)
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

			KernelPBD::Point_3 p0(pts[item.i].x, pts[item.i].y, pts[item.i].z);
			KernelPBD::Point_3 p1(pts[item.number].x, pts[item.number].y, pts[item.number].z);
			auto diff = (p0 - p1);
			auto v3 = balance * dr * diff / sqrt(diff.squared_length());

			b[3 * id0] += v3.x();
			b[3 * id0 + 1] += v3.y();
			b[3 * id0 + 2] += v3.z();
			b[3 * id1] -= v3.x();
			b[3 * id1 + 1] -= v3.y();
			b[3 * id1 + 2] -= v3.z();
		}
		m.setFromTriplets(assemble.begin(), assemble.end());
	}

	void GroupPBD::solveSingleGroup(int gId, Tree* pTree, XMFLOAT3* p0, WR::VecX& x, int pps)
	{
		std::list<IntPair> output, output2;
		auto& particleList = groupIds[gId];
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
						output.emplace_back(item.i, particleId);
				}
				else output2.emplace_back(item.i, particleId);
			}
		}

		// assemble matrix
		WR::VecX b;
		assembleMatrix(output, output2, p0, gId, b, dr);

		Eigen::SimplicialLLT<WR::SparseMat, Eigen::Upper> solver;
		//solver.analyzePattern(A);
		//solver.factorize(A);
		solver.compute(buffer[gId]);
		if (Eigen::Success != solver.info())
		{
			// TODO thread lock needed
			std::cout << "Matrix A is not factorizable." << std::endl;
			std::ofstream f("D:/error.mylog");
			f << buffer[gId];
			f.close();
			system("pause");
			exit(0);
		}
		x = solver.solve(b);
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

		GroupPBD* thiz;
		Tree* tree;
		HairGeometry* pHair;


	public:
		TbbPbdItem(GroupPBD* pp, Tree* tree, XMFLOAT3* p0, XMFLOAT3* p1, float dr, HairGeometry* hair)
		{
			pos0 = p0;
			newpos = p1;
			thiz = pp;
			this->tree = tree;
			pHair = hair;
			this->dr = dr;
		}

		void operator()(const blocked_range<size_t>& r) const {

			size_t end = r.end();
			for (size_t i = r.begin(); i != end; ++i)
			{
				std::vector<Point_3> points;
				auto& arr = thiz->groupIds[i];
				WR::VecX x;
				thiz->solveSingleGroup(i, tree, pos0, x, pHair->particlePerStrand);

				for (size_t i = 0; i < arr.size(); i++)
					newpos[arr[i]] = XMFLOAT3(x[3 * i], x[3 * i + 1], x[3 * i + 2]);
			}
		}
	};

	void GroupPBD::solveFull(HairGeometry* hair)
	{
		XMFLOAT3* allocMem = new XMFLOAT3[hair->nParticle];
		XMFLOAT3 *p0 = hair->position, *p1 = allocMem;
		for (int i = 0; i < 2; i++)
		{
			std::vector<Point_3> points;
			for (size_t j = 0; j < hair->nParticle; j++)
			{
				auto &pos = p0[j];
				points.emplace_back(pos.x, pos.y, pos.z, j);
			}
			Tree tree(points.begin(), points.end());


			for (int j = 0; j < nHairParticleGroup; j++)
			{
				WR::VecX x;
				auto& arr = groupIds[j];
				solveSingleGroup(j, &tree, p0, x, hair->particlePerStrand);

				for (size_t k = 0; k < arr.size(); k++)
					p1[arr[k]] = XMFLOAT3(x[3 * k], x[3 * k + 1], x[3 * k + 2]);
			}

			TbbPbdItem tbbCls(this, &tree, p0, p1, dr, hair);
			parallel_for(blocked_range<size_t>(0, nHairParticleGroup, 10), tbbCls);

			std::swap(p0, p1);
		}

		if (hair->position != p1)
			std::swap(allocMem, hair->position);

		SAFE_DELETE_ARRAY(allocMem);
	}
}