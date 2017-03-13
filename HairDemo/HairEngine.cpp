#include "precompiled.h"

#include <string>
#include <xlogger.h>

#define XTIMER_INSTANCE
#include <xtimer.hpp>

#define XRWY_EXPORTS
#include "HairEngine.h"

#include "xhair.h"
#include "SkinningEngine.h"

using std::string;

namespace xhair
{
    class HairEngine
    {
    public:
        HairEngine() {}
        ~HairEngine() {}

        int initialize(const char* static_hair_filename,
            const char* head_collision_filename,
            const char* guide_hair_parameter);

        int update(const float head_matrix[16],
            // used for check if the geometry is the same
            const char* static_hair_filename,
            float *particle_position);

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

        _engine_instance = new HairEngine;
        int ret = 0;
        if (_engine_instance)
        {
            ret = _engine_instance->initialize(static_hair_filename,
                head_collision_filename, guide_hair_parameter);

            if (ret != 0) return ret;
        }
        else return -1;

        return 0;
    }

    XRWY_DLL int UpdateHairEngine(
        const float head_matrix[16],
        // used for check if the geometry is the same
        const char* static_hair_filename,
        float *particle_position)
    {
        return _engine_instance->update(head_matrix,
            static_hair_filename, particle_position);
    }

    XRWY_DLL void ReleaseHairEngine()
    {
        SAFE_DELETE(_engine_instance);
    }

}

namespace xhair
{
    int HairEngine::initialize(const char * static_hair_filename, 
        const char * head_collision_filename,
        const char * guide_hair_parameter)
    {
        HairGeometry* hair = load_hair_geometry(static_hair_filename);
        if (!hair) return -1;

        ICollision
    }

    int HairEngine::update(const float head_matrix[16], 
        const char * static_hair_filename, 
        float * particle_position)
    {
        return 0;
    }
}