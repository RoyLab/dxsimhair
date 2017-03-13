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
        CGroupPbd(int nparticle, int pps, float dr, float balance, const int* groupInfo, size_t ngi, int nGroup);
        ~CGroupPbd() {}

        void filter(HairGeometry* hair);
    private:
        MatrixFactory<IdContainer> mf;
        GridRaster<Point3, XR::ArrayWrapper<Point3>> pgrid;
        std::vector<uint32_t> id[8], *id0, *id1, *old0, *old1;
        float r0;
    };
}
