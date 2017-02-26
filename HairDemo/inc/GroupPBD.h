#pragma once
#include <vector>
#include <deque>

#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>

#include <CGAL/Kd_tree.h>
#include <CGAL/Search_traits_3.h>

#include "xhair.h"
#include "EigenTypes.h"
#include "CGALKernel.h"
#include "MatrixFactory.hpp"
#include "GridRaster.h"
#include "XStruct.hpp"

namespace xhair
{
	typedef CGAL::Floatipick KernelPBD;

	class KDSearchPoint :
		public KernelPBD::Point_3
	{
	public:
		KDSearchPoint() :KernelPBD::Point_3(), id(-1) {}
		KDSearchPoint(float x, float y, float z, int i) :
			KernelPBD::Point_3(x, y, z), id(i) {}

		int id;
	};

	class KDSearchTraits :
		public CGAL::Search_traits_3<KernelPBD>
	{
	public:
		typedef KDSearchPoint Point_d;
	};

	typedef KDSearchTraits Traits;
	typedef CGAL::Kd_tree<Traits> Tree;

	class GroupPBD :
		public IHairCorrection
	{
		friend class TbbPbdItem;
	public:
		GroupPBD() {}
		~GroupPBD();
		bool initialize(HairGeometry* hair, float dr, float balance, const int* groupInfo, size_t ngi, int nGroup);
		void solve(HairGeometry* hair);

	private:
		void solveSampled(HairGeometry* hair);
		void solveFull(HairGeometry* hair);
		void solveSingleGroup(int gId, const Tree* pTree, Point3* p0, WR::VecX& x, int pps, bool bAssembleA);
		bool belongTo(int idp, int idg) { return rawGroupId[idp] == idg; }
		void assembleMatA(std::list<IntPair>& oldl, std::list<IntPair>& oldl2, int gId);
		void computeVecb(int gId, Point3* pts, float dr, WR::VecX& b);

		std::deque<int>*	groupIds;  // n_group vectors. each containing non-follicle id for that group
		std::deque<int>*	groupMatrixMapping;  // n_group vectors. map from matrix indices (without 3x) to global Id
		std::deque<int>		rawGroupId;
		std::deque<int>		matrixSeq; // length: n_particle, map from global Id to matrix indices (without 3x)

		float				dr, balance;
		int					nHairParticleGroup;
		size_t				nHairParticle;
		bool				bMatrixInited;

		struct SolveGroupCache
		{
			std::list<IntPair> l, l2;
			WR::SparseMat A;
			Eigen::SimplicialLLT<WR::SparseMat, Eigen::Upper> solver;
		};

		std::deque<SolveGroupCache> solverCacheBuffer;
	};

	class GroupPBD2:
		public IHairCorrection
	{
		friend class TbbPbdItem;
		typedef std::vector<uint32_t> IdContainer;
	public:
		GroupPBD2(HairGeometry* hair, float dr, float balance, const int* groupInfo, size_t ngi, int nGroup);
		~GroupPBD2() {}
		bool initialize(HairGeometry* hair, float dr, float balance, const int* groupInfo, size_t ngi, int nGroup) { return true; }
		void solve(HairGeometry* hair);

	private:
		XRwy::Hair::MatrixFactory<IdContainer> mf;
		XRwy::GridRaster<Point3, XR::ArrayWrapper<Point3>> pgrid;

		std::vector<uint32_t> id[8], *id0, *id1, *old0, *old1;
		float r0;
	};
}
