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
		typedef CGAL::FloatEpick K;

		class KDSearchPoint :
			public K::Point_3
		{
		public:
			KDSearchPoint() :K::Point_3(), id(-1) {}
			KDSearchPoint(float x, float y, float z, int i) :
				K::Point_3(x, y, z), id(i) {}

			int id;
		};

		typedef KDSearchPoint Point_3;

		class KDSearchTraits :
			public CGAL::Search_traits_3<K>
		{
		public:
			typedef KDSearchPoint Point_d;
		};

		typedef KDSearchTraits Traits;
		typedef CGAL::Kd_tree<Traits> Tree;
		typedef CGAL::Fuzzy_sphere<Traits> Fuzzy_sphere;

		class IntWrapper
		{
		public:
			IntWrapper() {}
			IntWrapper(const KDSearchPoint& p)
			{
				i = p.id;
			}

			IntWrapper& operator=(const KDSearchPoint& p)
			{
				i = p.id;
				return *this;
			}

			IntWrapper& operator=(KDSearchPoint& p)
			{
				i = p.id;
				return *this;
			}

			int i;
		};

		struct IntPair
		{
			IntPair(int a, int b) :i(a), number(b) {}
			int i, number;
		};


		// let the number > i
		void filter(std::list<IntPair>& l)
		{
			for (auto itr = l.begin(); itr != l.end(); itr++)
			{
				while (itr!= l.end() && itr->i >= itr->number)
					itr = l.erase(itr);

				if (itr == l.end())
					break;
			}
		}

#define BATCH3_MAT_PUSH(m, i, j, value) \
	(m).push_back(Eigen::Triplet<float>((3*i), (3*j), value));\
	(m).push_back(Eigen::Triplet<float>((3*i+1), (3*j+1), value));\
	(m).push_back(Eigen::Triplet<float>((3*i+2), (3*j+2), value));\
	(m).push_back(Eigen::Triplet<float>((3*i), (3*i), -value));\
	(m).push_back(Eigen::Triplet<float>((3*i+1), (3*i+1), -value));\
	(m).push_back(Eigen::Triplet<float>((3*i+2), (3*i+2), -value))

#define BATCH3_VEC_SUM(m, i, vec)


		void assembleMatrix(std::list<IntPair>& l, std::vector<Point_3>& pts,
			WR::SparseMat& m, WR::VecX& b, float dr)
		{
			size_t dim = 3*pts.size();
			m.resize(dim, dim);
			b.resize(dim);

			// dummy assemble routine
			//m.setIdentity();

			std::list<Eigen::Triplet<float>> assemble;
			const float balance = 1.0f; // TODO time step related

			for (size_t i = 0; i < pts.size(); i++)
			{
				b[3*i] = pts[i].x();
				b[3*i+1] = pts[i].y();
				b[3*i+2] = pts[i].z();
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

		groupIds = new std::vector<int>[nGroup];
		for (size_t i = 0; i < ngi; i++)
			groupIds[groupInfo[i]].push_back(i);

		nHairParticle = ngi;
		return true;
	}

	void GroupPBD::solve(HairGeometry* hair)
	{

		if (hair->nParticle == nHairParticle)
			solveFullMulti(hair);
		else
			solveSampled(hair);
	}

	namespace
	{
		void solveSingleGroup(std::vector<Point_3>& points, WR::VecX& x, float dr)
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
		solveSingleGroup(points, x, dr);

		// export points
		for (size_t i = 0; i < hair->nParticle; i++)
			hair->position[i] = XMFLOAT3(x[3 * i], x[3 * i + 1], x[3 * i + 2]);
	}

	namespace
	{
		using namespace tbb;

		class TbbPbdItem {
			XMFLOAT3* pos0;
			XMFLOAT3* newpos;
			std::vector<int>* thiz;
			float dr;

		public:
			TbbPbdItem(XMFLOAT3* p0, XMFLOAT3* p1, std::vector<int>* idx, float dr)
			{
				pos0 = p0;
				newpos = p1;
				thiz = idx;
				this->dr = dr;
			}

			void operator()(const blocked_range<size_t>& r) const {
				XMFLOAT3 *p0 = pos0;
				XMFLOAT3 *p1 = newpos;
				auto& idx = thiz;
				size_t end = r.end();

				for (size_t i = r.begin(); i != end; ++i)
					solveGroup(i);
			}

		private:
			void solveGroup(int id) const
			{
				std::vector<Point_3> points;
				auto& arr = thiz[id];
				for (size_t i = 0; i < arr.size(); i++)
				{
					auto &pos = pos0[arr[i]];
					points.emplace_back(pos.x, pos.y, pos.z, i);
				}

				WR::VecX x;
				solveSingleGroup(points, x, dr);

				for (size_t i = 0; i < arr.size(); i++)
					newpos[arr[i]] = XMFLOAT3(x[3 * i], x[3 * i + 1], x[3 * i + 2]);
			}

		};
	}

	void GroupPBD::solveFullSingle(HairGeometry* hair)
	{
		XMFLOAT3* allocMem = new XMFLOAT3[hair->nParticle];
		XMFLOAT3 *p0 = hair->position, *p1 = allocMem;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < nHairParticleGroup; j++)
			{
				std::vector<Point_3> points;
				auto& arr = groupIds[j];
				for (size_t k = 0; k < arr.size(); k++)
				{
					auto &pos = p0[arr[k]];
					points.emplace_back(pos.x, pos.y, pos.z, k);
				}

				WR::VecX x;
				solveSingleGroup(points, x, dr);

				for (size_t k = 0; k < arr.size(); k++)
					p1[arr[k]] = XMFLOAT3(x[3 * k], x[3 * k + 1], x[3 * k + 2]);
			}
			std::swap(p0, p1);
		}

		if (hair->position != p1)
			std::swap(allocMem, hair->position);

		SAFE_DELETE_ARRAY(allocMem);
	}

	void GroupPBD::solveFullMulti(HairGeometry* hair)
	{
		using namespace tbb;

		XMFLOAT3* allocMem = new XMFLOAT3[hair->nParticle];
		XMFLOAT3 *p0 = hair->position, *p1 = allocMem;
		for (int i = 0; i < 4; i++)
		{
			TbbPbdItem tbbCls(p0, p1, groupIds, dr);
			parallel_for(blocked_range<size_t>(0, nHairParticleGroup, 10), tbbCls);
			std::swap(p0, p1);
		}

		if (hair->position != p1)
			std::swap(allocMem, hair->position);

		SAFE_DELETE_ARRAY(allocMem);
	}

}