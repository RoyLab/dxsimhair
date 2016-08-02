#define CGAL_EIGEN3_ENABLED
#include <CGAL/Kd_tree.h>
#include <CGAL/Fuzzy_sphere.h>
#include <CGAL/Search_traits_3.h>

#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>
#include "EigenTypes.h"
#include "wrTripleMatrix.h"

#include "CGALKernel.h"

#define WR_EXPORTS
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
				number = curNumber;
			}

			IntWrapper& operator=(const KDSearchPoint& p)
			{
				i = p.id;
				number = curNumber;
				return *this;
			}

			IntWrapper& operator=(KDSearchPoint& p)
			{
				i = p.id;
				number = curNumber;
				return *this;
			}

			int i;
			int number;

		public:
			static int curNumber;
		};

		int IntWrapper::curNumber;

		// let the number > i
		void filter(std::list<IntWrapper>& l)
		{
			for (auto itr = l.begin(); itr != l.end(); itr++)
			{
				while (itr!= l.end() && itr->i >= itr->number)
					itr = l.erase(itr);

				if (itr == l.end())
					break;
			}
		}

		void assembleMatrix(std::list<IntWrapper>& l, std::vector<Point_3>& pts,
			WR::SparseMatAssemble& m, WR::VecX& b, float dr)
		{
			size_t dim = 3*pts.size();
			m.resize(dim, dim);
			b.resize(dim);

			// dummy assemble routine
			//m.setIdentity();

			//for (size_t i = 0; i < pts.size(); i++)
			//{
			//	b[3*i] = pts[i].x();
			//	b[3*i+1] = pts[i].y();
			//	b[3*i+2] = pts[i].z();
			//}

			// assume
			const float balance = 1.0f; // TODO time step related
			m.setIdentity();
			for (auto& item : l)
			{
				assert(item.i > item.number);
				m.
			}
		}
	}

	extern "C" WR_API IHairCorrection* CreateHairCorrectionObject()
	{
		return new GroupPBD;
	}

	bool GroupPBD::initialize(HairGeometry* hair)
	{
		// currently there is only one group
		nWorker = 1;
		dr = 0.1f;
		
		return true;
	}

	void GroupPBD::solve(HairGeometry* hair)
	{
		// import points
		std::vector<Point_3> points;
		for (size_t i = 0; i < hair->nParticle; i++)
		{
			points.emplace_back(hair->position[i].x,
				hair->position[i].y, hair->position[i].z, i);
		}

		// build KD-tree
		Tree tree(points.begin(), points.end());

		// find all neighboring points
		std::list<IntWrapper> output;
		for (size_t i = 0; i < points.size(); i++)
		{
			IntWrapper::curNumber = i;
			Fuzzy_sphere fs(points[i], dr, 0.3f * dr);
			tree.search(std::back_inserter(output), fs);
		}
		filter(output);

		// assemble matrix
		WR::SparseMatAssemble A;
		WR::VecX b, x;
		assembleMatrix(output, points, A, b, dr);

		Eigen::SimplicialLLT<WR::SparseMat, Eigen::Upper> solver;
		//solver.analyzePattern(A);
		//solver.factorize(A);
		solver.compute(A);
		if (Eigen::Success != solver.info())
		{
			std::cout << solver.info() << std::endl;
			system("pause");
			exit(0);
		}
		x = solver.solve(b);

		// export points
		for (size_t i = 0; i < hair->nParticle; i++)
		{
			hair->position[i] = XMFLOAT3(x[3*i], x[3*i+1], x[3*i+2]);
		}
	}

}