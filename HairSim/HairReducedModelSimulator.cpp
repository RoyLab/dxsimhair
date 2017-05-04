#include "HairReducedModelSimulator.h"
#include "macros.h"
#include <fstream>
#include <cstdint>
#include <unordered_set>

using std::int32_t;

namespace {
	void float_ptr_set_zero(float *ptr, size_t sz) {
		float *ptr_end = ptr + sz;
		for (float *p = ptr; p != ptr_end; ++p)
			*p = 0.0;
	}

	void float_ptr_add_with_weight(float *dst, float *src, size_t sz, float weight) {
		for (int i = 0; i < sz; ++i)
			*(dst + i) += (*(src + i)) * weight;
	}
}

namespace XRwy {
	HairReducedModelSimulator::HairReducedModelSimulator(const ICollisionObject* collision_obj) {
		assert(g_paramDict.find("hairmodel")->second == "reduced");

		N_PARTICLES_PER_STRAND = stoi(g_paramDict.find("particleperstrand")->second);

		read_weightfile(g_paramDict.find("reduced_weightsfile")->second.c_str());
		read_resthairfile(g_paramDict.find("reffile")->second.c_str());

		assert(N_PARTICLES_PER_STRAND * rest_nstrand == rest_nparticle);

		//load the guide hair to the full simulator
		full_simulator = new HairFullModelSimulator(collision_obj);
		full_simulator->wr_hair = new WR::Hair;
		full_simulator->wr_hair->reserve(this->rest_nguide * N_PARTICLES_PER_STRAND, this->rest_nguide);
		WR::Hair *full_hair = full_simulator->wr_hair;
		for (const auto guide_id : guide_ids) {
			full_hair->add_strand(pos_ptr_from_strand(guide_id), collision_obj, N_PARTICLES_PER_STRAND);
		}
		WR::HairStrand::set_hair(full_hair);
		WR::HairParticle::set_hair(full_hair);
		full_hair->init_simulation();

		//try to print weights information
		//cout << "rest state number of strand: " << this->rest_nstrand << endl;
		//cout << "rest state number of guide: " << this->rest_nguide << endl;

		//cout << "guide ids: count=" << guide_ids.size() << endl;
		//for (const auto & guide_id : guide_ids)
		//	cout << guide_id << ' ';
		//cout << endl;

		//cout << "strand information: count=" << strand_infos.size() << endl;
		//for (int i = 0; i < min(100, (int)strand_infos.size()); ++i) {
		//	cout << "strand " << i << ":" << endl;
		//	for (const auto & id_and_weight : strand_infos[i].id_and_weights)
		//		cout << "\tid=" << id_and_weight.first << ", weight=" << id_and_weight.second << endl;
		//}

		//unordered_set<int> guide_id_set = unordered_set<int>(guide_ids.begin(), guide_ids.end());
		//for (const auto &strand_info : strand_infos)
		//	for (const auto & id_and_weight : strand_info.id_and_weights)
		//		assert(guide_id_set.find(id_and_weight.first) != guide_id_set.end());
		//cout << "Every guide id is right, assert complete";

		//try to print the hair information
		//cout << "particle number: " << rest_nparticle << endl;
		//for (int i = 0; i < min(1000, this->rest_nparticle); ++i) {
		//	cout << '(' << rest_pos[i * 3] << ',' << rest_pos[i * 3 + 1] << ',' << rest_pos[i * 3 + 2] << ')' << endl;
		//}
	}

	void HairReducedModelSimulator::read_weightfile(const char *file_path) {
		ifstream fweights(file_path, ios::binary);
		assert(fweights.is_open());

		int32_t nstrand, nguide;
		ReadNBytes(fweights, &nstrand, 4);
		ReadNBytes(fweights, &nguide, 4);
		this->rest_nstrand = nstrand;
		this->rest_nguide = nguide;

		guide_ids.reserve(rest_nguide);
		for (int i = 0; i < rest_nguide; ++i) {
			int32_t guide_id;
			ReadNBytes(fweights, &guide_id, 4);
			this->guide_ids.push_back(guide_id);
		}

		//constructing guide_id_reverse
		guide_ids_reverse = vector<int>(this->rest_nstrand, INT_MIN); //a negative value is useful to debug
		for (int i = 0; i < guide_ids.size(); ++i)
			guide_ids_reverse[guide_ids[i]] = i;

		for (int i = 0; i < rest_nstrand; ++i) {
			int32_t nguide_strand;
			ReadNBytes(fweights, &nguide_strand, 4);

			if (nguide_strand != 0) {
				this->strand_infos.push_back({ nguide_strand, GuideStrands::InfoVector() });
				auto &l = strand_infos.back();
				l.id_and_weights.reserve(l.n_guide_strand);
				for (int j = 0; j < l.n_guide_strand; ++j) {
					int32_t guide_id;
					float guide_weight;
					ReadNBytes(fweights, &guide_id, 4);
					ReadNBytes(fweights, &guide_weight, 4);
					l.id_and_weights.push_back(make_pair(static_cast<int>(guide_id), guide_weight));
				}
			}
			else {
				//this hair is a guide hair, then the total guide strand is itself, and the weight is 1.0
				assert(find(guide_ids.begin(), guide_ids.end(), i) != guide_ids.end());
				this->strand_infos.push_back({ 1, { make_pair(i, 1.0) } });
			}
		}

		//initalizate the fbuffer
		guide_pos_buffer = new float[this->rest_nguide * 3 * N_PARTICLES_PER_STRAND];

		fweights.close();
	}

	void HairReducedModelSimulator::read_resthairfile(const char *file_path) {
		using namespace std;
		ifstream fhair(file_path, ios::binary);

		int32_t nparticle;
		ReadNBytes(fhair, &nparticle, 4);
		this->rest_nparticle = nparticle;

		rest_pos = new float[nparticle * 3];
		rest_vel = new float[nparticle * 3];

		ReadNBytes(fhair, rest_pos, sizeof(float) * 3 * nparticle);

		bool use_transform = stoi(g_paramDict.find("hair_usetransform")->second);
		float transform_scale = 1.0;
		bool mirror_x = false, mirror_y = false, mirror_z = false;
		if (use_transform) {
			transform_scale = stof(g_paramDict.find("hair_transformscale")->second);
			mirror_x = stoi(g_paramDict.find("hair_transformmirrorx")->second);
			mirror_y = stoi(g_paramDict.find("hair_transformmirrory")->second);
			mirror_z = stoi(g_paramDict.find("hair_transformmirrorz")->second);
		}
		float scale_x = transform_scale * (mirror_x ? -1.0 : 1.0);
		float scale_y = transform_scale * (mirror_y ? -1.0 : 1.0);
		float scale_z = transform_scale * (mirror_z ? -1.0 : 1.0);

		for (int i = 0; i < nparticle * 3; i += 3) {
			rest_vel[i] = rest_vel[i + 1] = rest_vel[i + 2] = 0.0;
			rest_pos[i] *= scale_x;
			rest_pos[i + 1] *= scale_y;
			rest_pos[i + 2] *= scale_z;
		}

		fhair.close();
	}

	void HairReducedModelSimulator::on_frame(const float rigids[16], float *pos, float *dir, float delta_time, ICollisionObject* collision_obj, const float collision_world2local_mat[16]) {

		for (int i = 0; i < this->rest_nstrand; ++i)
			for (const auto &id_and_weight : strand_infos[i].id_and_weights)
				assert(id_and_weight.first < rest_nstrand && id_and_weight.first >= 0);

		full_simulator->on_frame(rigids, guide_pos_buffer, nullptr, delta_time, collision_obj, collision_world2local_mat);

		//interploration
		for (int i = 0; i < this->rest_nstrand; ++i) {
			float *strand_ptr = pos_ptr_from_strand(i);
			size_t sz = 3 * N_PARTICLES_PER_STRAND;
			float_ptr_set_zero(strand_ptr, sz);

			for (const auto &id_and_weight : strand_infos[i].id_and_weights) {
				int id = id_and_weight.first;
				float weight = id_and_weight.second;
				float *sim_ptr = float_ptr_from_strand(guide_pos_buffer, guide_id2full_sim_idx(id));
				float_ptr_add_with_weight(strand_ptr, sim_ptr, sz, weight);
			}
		}

		memcpy(pos, this->rest_pos, sizeof(float) * 3 * this->rest_nparticle);
	}

	int HairReducedModelSimulator::get_particle_count() {
		return rest_nstrand * N_PARTICLES_PER_STRAND;
	}

	int HairReducedModelSimulator::get_particle_per_strand_count() {
		return N_PARTICLES_PER_STRAND;
	}

	HairReducedModelSimulator::~HairReducedModelSimulator() {
		SAFE_DELETE(this->full_simulator);
		SAFE_DELETE_ARRAY(this->rest_pos);
		SAFE_DELETE_ARRAY(this->rest_vel);
		SAFE_DELETE_ARRAY(this->guide_pos_buffer);
	}
}