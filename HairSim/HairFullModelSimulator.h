#pragma once

#include "HairSimulator.h"
#include "wrHair.h"

namespace XRwy {
	class HairFullModelSimulator : public HairSimulator {
		friend class WR::Hair;
	public:
		HairFullModelSimulator();
		virtual void on_frame(const float rigids[16], float *pos, float *dir, float delta_time);
		virtual int get_particle_count();
		virtual int get_particle_per_strand_count();
		virtual ~HairFullModelSimulator();
	private:
		WR::Hair *wr_hair;
	};
}