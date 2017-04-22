
#include "DllExports.h"
#include "HairStructs.h"
#include "macros.h"
#include "XConfigReader.hpp"
#include "XRwy_h.h"
#include "ZhouHairLoader.hpp"
#include "SkinningEngine.h"
#include "HairLoader.h"
#include "HairSimulator.h"
#include "HairLoaderSimulator.h"

#include <iostream>
#include <string>
#include <fstream>
using namespace std;

using XR::ConfigReader;

//#define USE_DEBUG_MODE

namespace XRwy {

	HairSimulator *simulator = nullptr;

#ifdef USE_DEBUG_MODE
	string root_path = "C:\\Users\\vivid\\Desktop\\";
#endif

	int InitializeHairEngine(const HairParameter* param, const CollisionParameter* col, const SkinningParameter* skin, const PbdParameter* pbd
	) {
#ifndef USE_DEBUG_MODE
		//for now only hair parameter root is used, others are ignored
		ConfigReader conf_reader(param->root, ConfigReader::ConfigReaderConfiguration::CONFIG_READ_AS_DESCRIPTION);
		conf_reader.getParamDict(g_paramDict);
		conf_reader.close();

		simulator = new HairLoaderSimulator;
#else
		/* use for testing */
		ofstream fout;
		fout.open(root_path + "InitializeHairEngine.txt", ios::out);

		fout << "HairParameter" << endl;
		fout << "collision: " << param->b_collision << endl;
		fout << "pbd: " << param->b_pbd << endl;
		fout << "guide: " << param->b_guide << endl;
		fout << "root: " << param->root << endl;

		fout << endl;
		fout << "CollisionParameter" << endl;
		if (col) {
			fout << "correction rate: " << col->correction_rate << endl;
			fout << "correction tolerance: " << col->correction_tolerance << endl;
			fout << "maxstep: " << col->maxstep << endl;
		}
		else {
			fout << "null" << endl;
		}

		fout << endl;
		fout << "SkinningParameter" << endl;
		if (skin) {
			fout << "simulate guide: " << skin->simulateGuide << endl;
		}
		else {
			fout << "null" << endl;
		}

		fout << endl;
		fout << "PbdParameter" << endl;
		if (pbd) {
			fout << "chunksize: " << pbd->chunksize << endl;
			fout << "detect range: " << pbd->detectrange << endl;
			fout << "lambda: " << pbd->lambda << endl;
			fout << "max iteration" << pbd->maxiteration << endl;
		}
		else {
			fout << "null" << endl;
		}
#endif // !USE_DEBUG_MODE
		return 0;
	}

	int UpdateParameter(const char* key, const char* value) {
#ifndef USE_DEBUG_MODE
		g_paramDict[key] = value;
#else
		/* use for testing */
		ofstream fout;
		fout.open(root_path + "UpdateParameter.txt", ios::out);
		fout << "key: " << key << endl;
		fout << "value: " << value << endl;
#endif // !USE_DEBUG_MODE

		return 0;
	}

	int UpdateHairEngine(const float head_matrix[16], float *particle_positions, float *particle_directions) {
#ifndef USE_DEBUG_MODE
		simulator->on_frame(head_matrix);

		const auto &hair = simulator->hair;
		memcpy(particle_positions, hair.position, sizeof(float) * 3 * hair.nParticle);
		memcpy(particle_directions, hair.direction, sizeof(float) * 3 * hair.nParticle);
#else
		/* use for testing */
		ofstream fout;
		fout.open(root_path + "UpdateHairEngine.txt", ios::out);
		fout << "head_matrix: " << endl;
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j)
				fout << head_matrix[i * 4 + j] << " ";
			fout << endl;
		}

		particle_positions[0] = 1.0;
		particle_positions[1] = 2.0;
		particle_positions[2] = 3.0;
		particle_directions[0] = 4.0;
		particle_directions[1] = 5.0;
		particle_directions[2] = 6.0;
#endif // !USE_DEBUG_MODE

		return 0;
	}

	void ReleaseHairEngine() {
#ifndef USE_DEBUG_MODE
		SAFE_DELETE(simulator);
#else
		/* use for testing */
		ofstream fout;
		fout.open(root_path + "ReleaseHairEngine.txt", ios::out);
		fout << "Release Hair Engine" << endl;
#endif // !USE_DEBUG_MODE
	}

	int GetHairParticleCount() {
#ifndef USE_DEBUG_MODE
		return simulator->hair.nParticle;
#else
		/* use for testing */
		return 12345;
#endif // !USE_DEBUG_MODE
	}

	int GetParticlePerStrandCount() {
#ifndef USE_DEBUG_MODE
		return simulator->hair.particlePerStrand;
#else
		/* use for testing */
		return 67890;
#endif
	}
}