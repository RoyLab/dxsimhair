#define CGAL_EIGEN3_ENABLED
#include <CGAL/Kd_tree.h>
#include <CGAL/Fuzzy_sphere.h>
#include <CGAL/Search_traits_3.h>

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
        const int * groupInfo,
        size_t ngi,
        int nGroup): 
        mf(groupInfo, nparticle, balance, pps), pgrid(dr)
	{
		id0 = &id[0]; id1 = &id[1];
		old0 = &id[2]; old1 = &id[3];

		r0 = dr;
	}

	void CGroupPbd::filter(HairGeometry * hair)
	{
		ArrayWrapper<Point3> wrapper(hair->position.get(), hair->nParticle);
		pgrid.initialize(wrapper);
		pgrid.query(*id0, *id1, true, old0, old1, id[4], id[5], id[6], id[7]);
		mf.update(*id0, *id1, id[4], id[5], id[6], id[7], reinterpret_cast<float*>(hair->position.get()), hair->nParticle, r0);

		std::swap(id0, old0);
		std::swap(id1, old1);
		id0->clear(); id1->clear();
	}

    IGroupPbd * CreateGroupPdb(const ParamDict & param)
    {
        CGroupPbd* ret = new CGroupPbd(
            param.nparticle,
            param.particle_per_strand,
            param.detect_range,
            param.solve_balance_factor,
            param.group_ids.data(),
            param.group_ids.size(),
            param.number_of_groups);
        return ret;
    }
}