#pragma once

#include <vector>
#include <deque>
#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>
#include <CGAL/Kd_tree.h>
#include <CGAL/Search_traits_3.h>
#include "HairStructs.h"
#include "EigenTypes.h"
#include "CGALKernel.h"

namespace XRwy
{
	typedef CGAL::FloatEpick KernelPBD;

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

	struct HairGeometry;

	class IHairCorrection
	{
	public:
		virtual void solve(HairGeometry* hair) = 0;
		virtual bool initialize(HairGeometry* hair, float dr, float balance, const int* groupInfo, size_t ngi, int nGroup) = 0;
		virtual ~IHairCorrection() {}
	};

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
		void solveSingleGroup(int gId, const Tree* pTree, XMFLOAT3* p0, WR::VecX& x, int pps, bool bAssembleA);
		bool belongTo(int idp, int idg) { return rawGroupId[idp] == idg; }
		void assembleMatA(std::list<IntPair>& oldl, std::list<IntPair>& oldl2, int gId);
		void computeVecb(int gId, XMFLOAT3* pts, float dr, WR::VecX& b);

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
}
