#pragma once
#include <vector>
#include "HairStructs.h"

namespace XRwy
{
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
	public:
		GroupPBD() {}
		~GroupPBD();
		bool initialize(HairGeometry* hair, float dr, const int* groupInfo, size_t ngi, int nGroup);
		void solve(HairGeometry* hair);

	private:
		void solveSampled(HairGeometry* hair);
		void solveFullMulti(HairGeometry* hair);
		void solveFullSingle(HairGeometry* hair);

		int			nWorker;
		int			nHairParticleGroup;
		float		dr;
		std::vector<int>* groupIds;
		size_t		nHairParticle;
	};
}
