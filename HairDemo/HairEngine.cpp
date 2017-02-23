#include "precompiled.h"

#include <string>
#include <xlogger.h>

#define XTIMER_INSTANCE
#include "xtimer.hpp"

#define XRWY_EXPORTS
#include "HairEngine.h"
#include "HairStructs.h"


using std::string;

namespace xhair
{
    class HairEngine
    {
    public:
        HairEngine(
            const char* static_hair_filename,
            const char* head_collision_filename,
            const char* guide_hair_parameter
        );
        ~HairEngine() {}


    private:
        string static_hair_name_;
        string head_collision_name_;

        HairGeometry hair_geometry_;
    };


    HairEngine *_engine_instance;
}

extern "C"
{
    using namespace xhair;

    XRWY_DLL int InitializeHairEngine(
        const char* static_hair_filename,
        const char* head_collision_filename,
        const char* guide_hair_parameter)
    {
        if (_engine_instance)
        {
            ReleaseHairEngine();
        }

        _engine_instance = new HairEngine(static_hair_filename,
            head_collision_filename, guide_hair_parameter);

        return 0;
    }

    XRWY_DLL int UpdateHairEngine(
        const float head_matrix[16],
        // used for check if the geometry is the same
        const char* static_hair_filename,
        float *particle_position)
    {
        return 0;
    }

    XRWY_DLL void ReleaseHairEngine()
    {
        SAFE_DELETE(_engine_instance);
    }

}


namespace xhair
{
    HairEngine::HairEngine(const char * static_hair_filename, 
        const char * head_collision_filename,
        const char * guide_hair_parameter)
    {

    }
}