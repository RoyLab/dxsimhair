#define CGAL_EIGEN3_ENABLED
#include <fstream>

#include <macros.h>
#include <xtimer.hpp>
#include <xstruct.hpp>
#include <xlogger.h>
#include <XConfigReader.hpp>

#include "EigenTypes.h"
#include "wrTripleMatrix.h"
#include "CGALKernel.h"

#include "CGroupPbd.h"

using namespace XR;

namespace xhair
{
    CGroupPbd::CGroupPbd(
        int nparticle, 
        int pps,
        float dr,
        float balance,
        const std::string& groups):
        pgrid(dr)
	{
        const int * groupInfo;
        mf = new MatrixFactory<IdContainer>(groupInfo, nparticle, balance, pps);
		id0 = &id[0]; id1 = &id[1];
		old0 = &id[2]; old1 = &id[3];

		r0 = dr;
	}

	void CGroupPbd::filter(HairGeometry * hair)
	{
		ArrayWrapper<Point3> wrapper(hair->position, hair->nParticle);
		pgrid.initialize(wrapper);
		pgrid.query(*id0, *id1, true, old0, old1, id[4], id[5], id[6], id[7]);
		mf->update(*id0, *id1, id[4], id[5], id[6], id[7], reinterpret_cast<float*>(hair->position), hair->nParticle, r0);

		std::swap(id0, old0);
		std::swap(id1, old1);
		id0->clear(); id1->clear();
	}

    IGroupPbd * CreateGroupPdb(const ParamDict & param)
    {
        CGroupPbd* ret = new CGroupPbd(
            param.at(P_nparticle).intval,
            param.at(P_pps).intval,
            param.at(P_detectRange).floatval,
            param.at(P_lambda).floatval,
            param.at(P_groupFile).stringval);
        return ret;
    }
}