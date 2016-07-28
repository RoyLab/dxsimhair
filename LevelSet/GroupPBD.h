#pragma once
#include "HairStructs.h"

namespace XRwy
{
	struct HairGeometry;

	class IHairCorrection
	{
	public:
		virtual void solve(HairGeometry* hair) = 0;
		virtual bool initialize(HairGeometry* hair) = 0;
		virtual ~IHairCorrection() {}
	};

	class GroupPBD :
		public IHairCorrection
	{
	public:
		bool initialize(HairGeometry* hair);
		void solve(HairGeometry* hair);

	private:
		int			nWorker;
		int			nHairParticleGroup;
	};
}
