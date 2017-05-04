#pragma once

#include "HairFullModelSimulator.h"
#include <vector>
#include <utility>

namespace XRwy {
	struct GuideStrands {
		typedef std::vector<std::pair<int, float>> InfoVector;

		int n_guide_strand;
		InfoVector id_and_weights;
	};

	class HairReducedModelSimulator : public HairSimulator {
	public:
		HairReducedModelSimulator(const ICollisionObject *collision_obj);
		~HairReducedModelSimulator();

		virtual void on_frame(const float rigids[16], float *pos, float *dir, float delta_time, ICollisionObject* collision_obj, const float collision_world2local_mat[16]);
		virtual int get_particle_count();
		virtual int get_particle_per_strand_count();
	protected:
		HairFullModelSimulator *full_simulator = nullptr;

		int rest_nparticle;
		int rest_nstrand;
		int rest_nguide;

		std::vector<int> guide_ids; //full simulation index to guide hair id
		std::vector<int> guide_ids_reverse; //guide hair id to full simulation index
		std::vector<GuideStrands> strand_infos;

		float *rest_pos = nullptr;
		float *rest_vel = nullptr;

		float *guide_pos_buffer = nullptr;
	private:
		void read_weightfile(const char *);
		void read_resthairfile(const char *);

		//helper function
		inline int full_sim_idx2guide_id(int idx) { return guide_ids[idx]; }
		inline int guide_id2full_sim_idx(int id) { return guide_ids_reverse[id]; }

		inline int strand_idx_from_particle(int paridx) { return paridx / N_PARTICLES_PER_STRAND; }
		inline int particle_idx_from_strand(int strandidx, int offset = 0) { return strandidx * N_PARTICLES_PER_STRAND + offset; }

		inline float* float_ptr_from_particle(float* begin, int paridx) { 
			return begin + paridx * 3; 
		}
		inline float* pos_ptr_from_particle(int paridx) { 
			return float_ptr_from_particle(this->rest_pos, paridx); 
		}
		inline float* vel_ptr_from_particle(int paridx) { 
			return float_ptr_from_particle(this->rest_vel, paridx); 
		}
		
		inline float* float_ptr_from_strand(float* begin, int strandidx) {
			return  begin + strandidx * N_PARTICLES_PER_STRAND * 3;
		}
		inline float* pos_ptr_from_strand(int strandidx) {
			return float_ptr_from_strand(this->rest_pos, strandidx);
		}
		inline float* vel_ptr_from_strand(int strandidx) {
			return float_ptr_from_strand(this->rest_vel, strandidx);
		}
	};
}