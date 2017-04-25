#pragma once

#include "XRwy_h.h"
#include "ZhouHairLoader.hpp"
#include "HairLoader.h"
#include "SkinningEngine.h"
#include "HairSimulator.h"
#include "macros.h"
#include <string>
using namespace std;

namespace XRwy {
	class HairLoaderSimulator : public HairSimulator {
	public:
		HairLoader *loader;
		HairGeometry *hair;

		HairLoaderSimulator() : HairSimulator() {
			assert(g_paramDict.find("hairmodel")->second == "loader");

			string filepath = g_paramDict.find("reffile")->second;
			string suffix = filepath.substr(filepath.rfind('.'));

			if (suffix == ".anim2")
				this->loader = new HairAnimationLoader;
			else if (suffix == ".hair")
				this->loader = new ZhouHairLoader;
			else if (suffix == ".recons") {
				int reduced_init_param = stoi(g_paramDict["para0"], 0, 2);
				this->loader = new ReducedModel(reduced_init_param);
			}

			hair = new HairGeometry;
			this->loader->loadFile(filepath.c_str(), this->hair);
		}

		virtual void on_frame(const float rigids[16], float *pos, float *dir, float delta_time) {
			loader->nextFrame();

			size_t size = sizeof(float) * 3 * hair->nParticle;
			if (pos)
				memcpy(pos, hair->position, size);
			if (dir)
				memcpy(dir, hair->direction, size);
		}

		virtual int get_particle_count() {
			return hair->nParticle;
		}

		virtual int get_particle_per_strand_count() {
			return hair->particlePerStrand;
		}

		virtual ~HairLoaderSimulator() {
			delete loader;
			loader = nullptr;

			delete hair;
			hair = nullptr;
		}
	};
}