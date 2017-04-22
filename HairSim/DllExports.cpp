
#include "DllExports.h"
#include "HairStructs.h"
#include "macros.h"
#include "XConfigReader.hpp"
#include "XRwy_h.h"
#include "ZhouHairLoader.hpp"
#include "SkinningEngine.h"
#include "HairLoader.h"

#include <iostream>
#include <string>
#include <fstream>
using namespace std;

using XR::ConfigReader;

namespace XRwy {

	HairLoader *loader;
	HairGeometry *hair;

	/* use for testing */
	//string root_path = "C:\\Users\\vivid\\Desktop\\";

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

		/* use for testing */
		//ofstream fout;
		//fout.open(root_path + "InitializeHairEngine.txt", ios::out);
		//
		//fout << "HairParameter" << endl;
		//fout << "collision: " << param->b_collision << endl;
		//fout << "pbd: " << param->b_pbd << endl;
		//fout << "guide: " << param->b_guide << endl;
		//fout << "root: " << param->root << endl;

		//fout << endl;
		//fout << "CollisionParameter" << endl;
		//if (col) {
		//	fout << "correction rate: " << col->correction_rate << endl;
		//	fout << "correction tolerance: " << col->correction_tolerance << endl;
		//	fout << "maxstep: " << col->maxstep << endl;
		//}
		//else {
		//	fout << "null" << endl;
		//}
		//
		//fout << endl;
		//fout << "SkinningParameter" << endl;
		//if (skin) {
		//	fout << "simulate guide: " << skin->simulateGuide << endl;
		//}
		//else {
		//	fout << "null" << endl;
		//}

		//fout << endl;
		//fout << "PbdParameter" << endl;
		//if (pbd) {
		//	fout << "chunksize: " << pbd->chunksize << endl;
		//	fout << "detect range: " << pbd->detectrange << endl;
		//	fout << "lambda: " << pbd->lambda << endl;
		//	fout << "max iteration" << pbd->maxiteration << endl;
		//}
		//else {
		//	fout << "null" << endl;
		//}
			
		return 0;
	}

	int UpdateParameter(const char* key, const char* value) {
		g_paramDict[key] = value;

		int d = 4;

		/* use for testing */
		//ofstream fout;
		//fout.open(root_path + "UpdateParameter.txt", ios::out);
		//fout << "key: " << key << endl;
		//fout << "value: " << value << endl;
		return 0;
	}

	int UpdateHairEngine(const float head_matrix[16], float *particle_positions, float *particle_directions) {
		loader->nextFrame();
		memcpy(particle_positions, hair->position, sizeof(float) * 3 * hair->nParticle);
		memcpy(particle_directions, hair->direction, sizeof(float) * 3 * hair->nParticle);

		/* use for testing */
		//ofstream fout;
		//fout.open(root_path + "UpdateHairEngine.txt", ios::out);
		//fout << "head_matrix: " << endl;
		//for (int i = 0; i < 4; ++i) {
		//	for (int j = 0; j < 4; ++j)
		//		fout << head_matrix[i * 4 + j] << " ";
		//	fout << endl;
		//}

		//particle_positions[0] = 1.0;
		//particle_positions[1] = 2.0;
		//particle_positions[2] = 3.0;
		//particle_directions[0] = 4.0;
		//particle_directions[1] = 5.0;
		//particle_directions[2] = 6.0;

		return 0;
	}

	void ReleaseHairEngine() {
		SAFE_DELETE(hair);
		SAFE_DELETE(loader);

		/* use for testing */
		//ofstream fout;
		//fout.open(root_path + "ReleaseHairEngine.txt", ios::out);
		//fout << "Release Hair Engine" << endl;
	}

	int GetHairParticleCount() {
		int size = hair->nParticle;

		return size;

		/* use for testing */
		//return 2147483646;
	}

	int GetParticlePerStrandCount() {
		int size = hair->particlePerStrand;
		return size;

		/* use for testing */
		//return 67890;
	}
}