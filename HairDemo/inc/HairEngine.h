#pragma once
#include <macros.h>

extern "C"
{
#define MAX_PATH_LENGTH 256

    struct HairParameter
    {
        bool b_guide;
        bool b_collision;
        bool b_pbd;

        char root[MAX_PATH_LENGTH]; // default main.hair
    };

    struct CollisionParameter
    {
        float correction_tolerance;
        float correction_rate;
        float maxstep;
    };

    struct SkinningParameter
    {
        bool b_simulateGuid;
    };

    struct PbdParameter
    {
        float lambda; // for solving optimization
        float chunksize; // parallel computing chunk
        int maxiteration;
    };

    XRWY_DLL int InitializeHairEngine(
        const HairParameter* param,
        const CollisionParameter* col,
        const SkinningParameter* skin,
        const PbdParameter* pbd
    );

    XRWY_DLL int UpdateParameter(int key, const char* value, char type);

    XRWY_DLL int UpdateHairEngine(
        const float head_matrix[16],
        float *particle_positions,
        float *particle_directions = nullptr
    );

    XRWY_DLL void ReleaseHairEngine();
}