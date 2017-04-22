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
		HairGeometry hair;

		HairSimulator() = default;
		virtual void on_frame(const float rigids[16]) = 0;
		virtual ~HairSimulator() = default;
	};
}