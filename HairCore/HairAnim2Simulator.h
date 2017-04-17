#pragma once

#include "stdafx.h"
#include "HairSimulator.h"
#include "HairLoader.h"

using XRwy::HairAnimationLoader;

namespace HairCore {
	class HairAnim2Simulator : public HairSimulator {
	public:
		HairAnim2Simulator(const HairConfiguration &conf);
		void on_frame(float trans[16]);
	private:
		HairAnimationLoader anim_loader;
	};
}