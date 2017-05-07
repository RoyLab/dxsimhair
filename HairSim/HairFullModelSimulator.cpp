#include "HairFullModelSimulator.h"
#include "XRwy_h.h"
#include "wrConstants.h"
#include "EigenTypes.h"

namespace XRwy {
	HairFullModelSimulator::HairFullModelSimulator(const Collider *collision_obj) : HairSimulator() {
		const auto & hairmodel = g_paramDict.find("hairmodel")->second;
		assert(hairmodel == "full" || hairmodel == "reduced");

		if (hairmodel == "full") {
			//in the reduced model, N_PARTICLE_PER_STRAND is handle by reduced model
			N_PARTICLES_PER_STRAND = stoi(g_paramDict.find("particleperstrand")->second);
		}
		COMPRESS_RATIO = stoi(g_paramDict.find("full_compressratio")->second);

		//initialize parameter
		//dynamic parts
		K_SPRINGS[1] = stof(g_paramDict.find("full_spring1")->second);
		K_SPRINGS[2] = stof(g_paramDict.find("full_spring2")->second);
		K_SPRINGS[3] = stof(g_paramDict.find("full_spring3")->second);

		PARTICLE_MASS = stof(g_paramDict.find("full_particlemass")->second);
		DAMPING_COEF = stof(g_paramDict.find("full_springdamping")->second);
		WIND_DAMPING_COEF = stof(g_paramDict.find("full_winddamping")->second);

		GRAVITY[0] = stof(g_paramDict.find("full_gravityx")->second);
		GRAVITY[1] = stof(g_paramDict.find("full_gravityy")->second);
		GRAVITY[2] = stof(g_paramDict.find("full_gravityz")->second);

		APPLY_COLLISION = stoi(g_paramDict.find("collision")->second);
		APPLY_STRAINLIMIT = stoi(g_paramDict.find("full_strainlimit")->second);

		TIME_STEP = stof(g_paramDict.find("full_timestep")->second);
		N_PASS_PER_STEP = stoi(g_paramDict.find("full_npassperstep")->second);

		this->register_item("full_spring1", &K_SPRINGS[1]);
		this->register_item("full_spring2", &K_SPRINGS[2]);
		this->register_item("full_spring3", &K_SPRINGS[3]);
		this->register_item("full_particlemass", &PARTICLE_MASS);
		this->register_item("full_springdamping", &DAMPING_COEF);
		this->register_item("full_winddamping", &WIND_DAMPING_COEF);
		this->register_item("full_collision", &APPLY_COLLISION);
		this->register_item("full_strainlimit", &APPLY_STRAINLIMIT);

		//if the hair model is reduced, that we should control this part for initalization
		if (hairmodel == "full") {
			bool use_transform = stoi(g_paramDict.find("hair_usetransform")->second);
			float transform_scale = 1.0;
			bool mirror_x = false, mirror_y = false, mirror_z = false;
			if (use_transform) {
				transform_scale = stof(g_paramDict.find("hair_transformscale")->second);
				mirror_x = stoi(g_paramDict.find("hair_transformmirrorx")->second);
				mirror_y = stoi(g_paramDict.find("hair_transformmirrory")->second);
				mirror_z = stoi(g_paramDict.find("hair_transformmirrorz")->second);
			}

			this->wr_hair = WR::loadFile(g_paramDict.find("reffile")->second.c_str(), collision_obj, use_transform, transform_scale * (mirror_x ? -1.0f : 1.0f), transform_scale * (mirror_y ? -1.0f : 1.0f), transform_scale * (mirror_z ? -1.0f : 1.0f));

			WR::HairStrand::set_hair(this->wr_hair);
			WR::HairParticle::set_hair(this->wr_hair);
			this->wr_hair->init_simulation();
		}
	}

	void HairFullModelSimulator::on_frame(const float rigids[16], float *pos, float *dir, float delta_time, const Collider* collider, const float collision_world2local_mat[16]) {
		WR::Mat4 rigid_mat4, collision_world2local_mat4;
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j) {
				rigid_mat4(i, j) = rigids[i * 4 + j];
				collision_world2local_mat4(i, j)= collision_world2local_mat[i * 4 + j];
			}

		this->wr_hair->step(rigid_mat4, 0.0f, delta_time, collider, collision_world2local_mat4);
		
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