#pragma once

#include "HairSimulator.h"
#include "ICollisionObject.h"
#include "wrHair.h"

namespace XRwy {
	class HairReducedModelSimulator;

	class HairFullModelSimulator : public HairSimulator {
		friend class WR::Hair;
		friend class HairReducedModelSimulator;
	public:
		HairFullModelSimulator(const ICollisionObject* collision_obj);
		virtual void on_frame(const float rigids[16], float *pos, float *dir, float delta_time, ICollisionObject* collision_obj, const float collision_world2local_mat[16]);
		virtual int get_particle_count();
		virtual int get_particle_per_strand_count();
		virtual ~HairFullModelSimulator();
	private:
		WR::Hair *wr_hair;
	};
}