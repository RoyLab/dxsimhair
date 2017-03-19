#pragma once
#include "xhair.h"

namespace xhair
{
    struct CollisionEngineParameter
    {
        float over_correction_tol;
        float iter_rate;
        float max_step;
        std::string obj_filename;
    };

    class ICollisionEngine : public IFilter
    {
    protected:
        ICollisionEngine() {}
    public:
        virtual ~ICollisionEngine() {}
        virtual void filter(HairGeometry* hair) = 0;
    };

    //ICollisionEngine* CreateCollisionEngine(const CollisionEngineParameter& param);
    ICollisionEngine* CreateCollisionEngine(const ParamDict& param);
}