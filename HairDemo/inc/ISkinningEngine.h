#pragma once
#include "xhair.h"

namespace xhair
{
    struct SkinningEngineParameter
    {

    };

    class ISkinningEngine: public IHairTransport
    {
    public:
        virtual void transport(HairGeometry* hair0, HairGeometry* hair1) = 0;
    };

    ISkinningEngine* CreateSkinningEngine(const ParamDict& param);
}
