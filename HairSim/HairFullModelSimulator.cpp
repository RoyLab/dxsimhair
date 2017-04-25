#include "HairFullModelSimulator.h"
#include "XRwy_h.h"
#include "wrConstants.h"
#include "EigenTypes.h"

namespace XRwy {
	HairFullModelSimulator::HairFullModelSimulator() : HairSimulator() {
		assert(g_paramDict.find("hairmodel")->second == "full");

		N_PARTICLES_PER_STRAND = stoi(g_paramDict.find("particleperstrand")->second);

		this->wr_hair = WR::loadFile(g_paramDict.find("reffile")->second.c_str());

		int use_transform = stoi(g_paramDict.find("full_usetransform")->second);
		if (use_transform) {
			float transform_scale = stof(g_paramDict.find("full_transformscale")->second);
			bool transform_x = stoi(g_paramDict.find("full_transformmirrorx")->second);
			bool transform_y = stoi(g_paramDict.find("full_transformmirrory")->second);
			bool transform_z = stoi(g_paramDict.find("full_transformmirrorz")->second);

			this->wr_hair->scale(transform_scale);
			this->wr_hair->mirror(transform_x, transform_y, transform_z);
		}

		//initialize parameter
		K_SPRINGS[1] = stof(g_paramDict.find("full_spring1")->second);
		K_SPRINGS[2] = stof(g_paramDict.find("full_spring2")->second);
		K_SPRINGS[3] = stof(g_paramDict.find("full_spring3")->second);

		PARTICLE_MASS = stof(g_paramDict.find("full_particlemass")->second);
		DAMPING_COEF = stof(g_paramDict.find("full_springdamping")->second);
		WIND_DAMPING_COEF = stof(g_paramDict.find("full_winddamping")->second);

		GRAVITY[0] = stof(g_paramDict.find("full_gravityx")->second);
		GRAVITY[1] = stof(g_paramDict.find("full_gravityy")->second);
		GRAVITY[2] = stof(g_paramDict.find("full_gravityz")->second);

		APPLY_COLLISION = stoi(g_paramDict.find("full_collision")->second);
		APPLY_STRAINLIMIT = stoi(g_paramDict.find("full_strainlimit")->second);

		//this->register_item("full_spring1", &K_SPRINGS[1]);
		//this->register_item("full_spring2", &K_SPRINGS[2]);
		//this->register_item("full_spring3", &K_SPRINGS[3]);
		//this->register_item("full_particlemass", &PARTICLE_MASS);
		//this->register_item("full_springdamping", &DAMPING_COEF);
		//this->register_item("full_winddamping", &WIND_DAMPING_COEF);
		//this->register_item("full_collision", &APPLY_COLLISION);
		//this->register_item("full_strainlimit", &APPLY_STRAINLIMIT);

		WR::HairStrand::set_hair(this->wr_hair);
		WR::HairParticle::set_hair(this->wr_hair);
		this->wr_hair->init_simulation();
	}

	void HairFullModelSimulator::on_frame(const float rigids[16], float *pos, float *dir, float delta_time) {
		WR::Mat3 rigid_mat;
		rigid_mat << rigids[0], rigids[1], rigids[2], rigids[4], rigids[5], rigids[6], rigids[8], rigids[9], rigids[10];

		this->wr_hair->step(rigid_mat, 0.0f, delta_time);
		
		int cur = 0;
		for (int i = 0; i < this->wr_hair->n_strands(); ++i)
			for (int j = 0; j < N_PARTICLES_PER_STRAND; ++j) {
				memcpy(pos + cur, this->wr_hair->get_visible_particle_position(i, j), sizeof(float) * 3);
				cur += 3;
			}
	}

	HairFullModelSimulator::~HairFullModelSimulator() {
		SAFE_DELETE(this->wr_hair);
	}

	int HairFullModelSimulator::get_particle_count() {
		return this->wr_hair->n_strands() * N_PARTICLES_PER_STRAND;
	}

	int HairFullModelSimulator::get_particle_per_strand_count() {
		return N_PARTICLES_PER_STRAND;
	}
} 