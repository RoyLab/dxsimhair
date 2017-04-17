#pragma once

#include "stdafx.h"
#include <string>
using std::string;

class HairConfiguration {
public:
	HairConfiguration(const ParameterDictionary &param_) : param(param_) {}

	//what type of simulator we will use
	int get_hair_simulator_model() const { return get_param_int("headmodel"); }

	//the simulator refference file
	string get_ref_file_path() const { return get_param("reffile"); }

	int get_collision_param() const { return stoi(get_param("para0"), 0, 2); }

	//should the simulator use the pbd
	bool get_use_pbd() { return get_collision_param() & 1; }

	//should the simulator use the level set
	bool get_use_level_set() { return get_collision_param() & 2; }

	//later changed: the head mesh model path
	string get_head_fbx_path() { return get_param("headfbx"); }

	//use to srand
	int get_rand_seed() { return get_param_int("randseed"); }

	//particle per strand
	int get_particle_per_strand() { return get_param_int("particleperstrand"); }

protected:
	ParameterDictionary param;

	string get_param(const string &key) const {
		return this->param.find(key)->second;
	}

	int get_param_int(const string &key) const {
		return stoi(this->get_param(key));
	}

	int get_param_bool(const string &key) const {
		return bool(get_param_int(key));
	}
};