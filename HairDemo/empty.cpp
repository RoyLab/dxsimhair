#define XRWY_EXPORTS
#include "HairEngine.h"

#include <iostream>

extern "C"
{
    XRWY_DLL int InitializeHairEngine(
        const HairParameter* param,
        const CollisionParameter* col,
        const SkinningParameter* skin,
        const PbdParameter* pbd)
    {
        std::cout << "Init\n";
        return 0;
    }

    XRWY_DLL int UpdateHairEngine(
        const float head_matrix[16],
        float *particle_positions,
        float *particle_directions)
    {
        std::cout << "update\n";
        return 0;
    }

    XRWY_DLL int UpdateParameter(const char* key, const char* value)
    {
        std::cout << "setting update\n";
        return 0;
    }

    XRWY_DLL void ReleaseHairEngine()
    {
        std::cout << "Release\n";
    }

}
