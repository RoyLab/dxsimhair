#pragma once
#include <vector>
#include "IGroupPbd.h"

#include "MatrixFactory.hpp"
#include "GridRaster.h"

namespace xhair
{
    class CGroupPbd : public IGroupPbd
    {
        typedef std::vector<uint32_t> IdContainer;
    public:
        CGroupPbd(int nparticle, int pps, float dr, float balance, 
            const std::string& groups);
        ~CGroupPbd() { SAFE_DELETE(mf); }

        void filter(HairGeometry* hair);
    private:
        MatrixFactory<IdContainer>* mf = nullptr;
        GridRaster<Point3, XR::ArrayWrapper<Point3>> pgrid;
        std::vector<uint32_t> id[8], *id0, *id1, *old0, *old1;
        float r0;
    };
}
