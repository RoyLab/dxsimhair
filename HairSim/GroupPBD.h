#pragma once
#include "HairStructs.h"

namespace XRwy
{
	class GroupPBD
	{
	public:
		bool initialize(HairGeometry* hair) { return true; }
		void solve(HairGeometry* hair) {}

	private:
		int			nWorker;
		int			nHairParticleGroup;
	};
}
