#pragma once

#include "stdafx.h"
#include "HairStructs.h"
#include "XConfigReader.hpp"
#include "HairConfiguration.h"

using XRwy::HairGeometry;

namespace HairCore {

	class HairSimulator {

	public:
		HairGeometry hair;

		HairSimulator(const HairConfiguration &conf) {}
		virtual void on_frame(float trans[16]) = 0;
	};
}