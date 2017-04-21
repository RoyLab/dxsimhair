
#include "DllExports.h"
#include "HairStructs.h"
#include "macros.h"
#include "XConfigReader.hpp"
#include "XRwy_h.h"
#include "ZhouHairLoader.hpp"
#include "SkinningEngine.h"
#include "HairLoader.h"

#include <iostream>
using namespace std;

using XR::ConfigReader;

namespace XRwy {

	HairLoader *loader;
	HairGeometry *hair;

	HairLoader* create_hair_loader(const char* fileName, HairGeometry* geom, void* para)
	{
		std::string name(fileName);
		int last = name.rfind('.');
		std::string posfix = name.substr(last);
		std::transform(posfix.begin(), posfix.end(), posfix.begin(), [](unsigned char c) { return std::tolower(c); });

		HairLoader* result = nullptr;
		if (posfix == ".anim2")
			result = new HairAnimationLoader;
		else if (posfix == ".hair")
			result = new ZhouHairLoader;
		else if (posfix == ".recons")
			result = new ReducedModel(*reinterpret_cast<int*>(para));

		if (result)
			result->loadFile(fileName, geom);

		return result;
	}

	int InitializeHairEngine(const HairParameter* param, const CollisionParameter* col, const SkinningParameter* skin, const PbdParameter* pbd
	) {
		//for now only hair parameter root is used, others are ignored
		ConfigReader conf_reader(param->root);
		conf_reader.getParamDict(g_paramDict);
		conf_reader.close();
		return 0;
	}

	int UpdateParameter(int key, const char* value, char type) {
		return 2;
	}

	int UpdateHairEngine(const float head_matrix[16], float *particle_positions, float *particle_directions) {
		return 3;
	}

	void ReleaseHairEngine() {
		SAFE_DELETE(hair);
		SAFE_DELETE(loader);
	}

	int GetHairParticleCount() {
		return 4;
	}

	int GetParticlePerStrandCount() {
		return 5;
	}
}