#define WR_EXPORTS
#include "LevelSet.h"
#include "GroupPBD.h"

namespace XRwy
{
	extern "C" WR_API IHairCorrection* CreateHairCorrectionObject()
	{
		return new GroupPBD;
	}

	bool GroupPBD::initialize(HairGeometry* hair)
	{
		// currently there is only one group
		return true;
	}

	void GroupPBD::solve(HairGeometry* hair)
	{

	}

}