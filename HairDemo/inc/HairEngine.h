#pragma once
#include <macros.h>

extern "C"
{
    XRWY_DLL int InitializeHairEngine(
        const char* static_hair_filename,
        const char* head_collision_filename,
        const char* guide_hair_parameter
    );

    XRWY_DLL int UpdateHairEngine(
        const float head_matrix[16],
        // used for check if the geometry is the same
        const char* static_hair_filename,
        float *particle_position
    );

    XRWY_DLL void ReleaseHairEngine();
}