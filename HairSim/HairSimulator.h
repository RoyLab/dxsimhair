#pragma once

#include "HairStructs.h"
#include "XRwy_h.h"
#include "ZhouHairLoader.hpp"
#include "HairLoader.h"
#include "SkinningEngine.h"
#include <string>
#include <map>
#include <utility>
#include "ICollisionObject.h"
#include "Collider.h"
using namespace std;

namespace XRwy {
	using ICollisionObject = WR::ICollisionObject;

	class HairSimulator {
	public:
		HairSimulator() = default;
		virtual void on_frame(const float rigids[16], float *pos, float *dir, float delta_time, const Collider* collider, const float collision_world2local_mat[16]) = 0;
		virtual int get_particle_count() = 0;
		virtual int get_particle_per_strand_count() = 0;
		virtual ~HairSimulator() = default;

		void register_item(string key, int* value_ptr) {
			register_item(key, 'i', value_ptr);
		}

		void register_item(string key, float* value_ptr) {
			register_item(key, 'f', value_ptr);
		}

		void register_item(string key, string* value_ptr) {
			register_item(key, 's', value_ptr);
		}

		void register_item(string key, bool* value_ptr) {
			register_item(key, 'b', value_ptr);
		}

		void on_change_parameter(string key, string value) {
			auto it = dynamic_items.find(key);
			if (it != dynamic_items.end()) {
				char t = it->second.first;
				void* ptr = it->second.second;
				switch (t) {
				case 'i':
					*(reinterpret_cast<int*>(ptr)) = stoi(value);
					break;
				case 'f':
					*(reinterpret_cast<float*>(ptr)) = stof(value);
					break;
				case 's':
					*(reinterpret_cast<string*>(ptr)) = value;
					break;
				case 'b':
					*(reinterpret_cast<bool*>(ptr)) = static_cast<bool>(stoi(value));
					break;
				default:
					break;
				}
			}
		}

	private:
		map<string, pair<char, void*>> dynamic_items;
		
		void register_item(string key, char type, void* value_ptr) {
			dynamic_items[key] = make_pair(type, value_ptr);
		}
	};
}