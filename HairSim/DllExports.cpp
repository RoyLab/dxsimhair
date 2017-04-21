
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

	HairLoader* create_hair_loader(const char* fileName, HairGeometry* geom)
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
		else if (posfix == ".recons") {
			//since for now we only use one instance of hair loader, the "para" will always be para0 in g_paramDict
			int reduced_init_param = stoi(g_paramDict["para0"], 0, 2);
			result = new ReducedModel(reduced_init_param);
		}

		if (result)
			result->loadFile(fileName, geom);

		return result;
	}

	int InitializeHairEngine(const HairParameter* param, const CollisionParameter* col, const SkinningParameter* skin, const PbdParameter* pbd
	) {
		//for now only hair parameter root is used, others are ignored
		ConfigReader conf_reader(param->root, ConfigReader::ConfigReaderConfiguration::CONFIG_READ_AS_DESCRIPTION);
		conf_reader.getParamDict(g_paramDict);
		conf_reader.close();

		hair = new HairGeometry;

		//we now use reffile only since there's only one instance
		loader = create_hair_loader((g_paramDict.find("reffile")->second).c_str(), hair);

		return 0;
	}

	int UpdateParameter(const char* key, const char* value) {
		g_paramDict[key] = value;
		return 0;
	}

	int UpdateHairEngine(const float head_matrix[16], float *particle_positions, float *particle_directions) {
		loader->nextFrame();
		memcpy(particle_positions, hair->position, sizeof(float) * 3 * hair->nParticle);
		memcpy(particle_directions, hair->direction, sizeof(float) * 3 * hair->nParticle);

		return 0;
	}

	void ReleaseHairEngine() {
		SAFE_DELETE(hair);
		SAFE_DELETE(loader);
	}

	int GetHairParticleCount() {
		return hair->nParticle;
	}

	int GetParticlePerStrandCount() {
		return hair->particlePerStrand;
	}
}