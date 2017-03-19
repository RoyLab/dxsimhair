#pragma once
#include <vector>
#include "xhair.h"

namespace xhair
{
    struct GroupPbdParameter
    {
        int nparticle;
        int particle_per_strand;
        float detect_range;
        float solve_balance_factor;
        int number_of_groups;
        std::vector<int> group_ids;
    };

	class IGroupPbd: public IFilter
	{
    protected:
        IGroupPbd() {}
	public:
        virtual ~IGroupPbd() {}
        virtual void filter(HairGeometry* hair) = 0;
	};

    IGroupPbd* CreateGroupPdb(const ParamDict& param);
}
