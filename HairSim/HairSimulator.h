#pragma once

#include "HairStructs.h"
#include "XRwy_h.h"
#include "ZhouHairLoader.hpp"
#include "HairLoader.h"
#include "SkinningEngine.h"
#include <string>
using namespace std;

namespace XRwy {
	class HairSimulator {
	public:
		HairSimulator() = default;
		virtual void on_frame(const float rigids[16], float *pos, float *dir) = 0;
		virtual int get_particle_count() = 0;
		virtual int get_particle_per_strand_count() = 0;
		virtual ~HairSimulator() = default;
	};
}