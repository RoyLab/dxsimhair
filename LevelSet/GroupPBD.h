#pragma once

#include <vector>

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
		virtual bool initialize(HairGeometry* hair, float dr, const int* groupInfo, size_t ngi, int nGroup) = 0;
		virtual ~IHairCorrection() {}
	};

	class GroupPBD :
		public IHairCorrection
	{
		friend class TbbPbdItem;
	public:
		GroupPBD() {}
		~GroupPBD();
		bool initialize(HairGeometry* hair, float dr, const int* groupInfo, size_t ngi, int nGroup);
		void solve(HairGeometry* hair);

	private:
		void solveSampled(HairGeometry* hair);
		void solveFull(HairGeometry* hair);
		void solveSingleGroup(int gId, Tree* pTree, XMFLOAT3* p0, WR::VecX& x, int pps);
		bool belongTo(int idp, int idg) { return rawGroupId[idp] == idg; }
		void assembleMatrix(std::list<IntPair>& l, std::list<IntPair>& l2,
			XMFLOAT3* pts, int gId, WR::VecX& b, float dr);

		int			nWorker;
		int			nHairParticleGroup;
		float		dr;
		std::vector<int>*	groupIds;
		std::vector<int>	rawGroupId;
		std::vector<int>	matrixSeq;
		size_t				nHairParticle;

		std::vector<WR::SparseMat> buffer;
	};
}
