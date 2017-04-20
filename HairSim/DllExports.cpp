
#include "DllExports.h"

int XRwy::InitializeHairEngine(const HairParameter* param, const CollisionParameter* col, const SkinningParameter* skin, const PbdParameter* pbd
) {
	return 1;
}

int XRwy::UpdateParameter(int key, const char* value, char type) {
	return 2;
}

int XRwy::UpdateHairEngine(const float head_matrix[16], float *particle_positions, float *particle_directions) {
	return 3;
}

void XRwy::ReleaseHairEngine() {
}

int XRwy::GetHairParticleCount() {
	return 4;
}

int XRwy::GetParticlePerStrandCount() {
	return 5;
}