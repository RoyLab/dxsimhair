
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
#include "HairFullModelSimulator.h"
#include "LevelSet.h"

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/bounding_box.h>
#include "ICollisionObject.h"
#include "GridCollisionObject.h"
#include "UnitTest.h"

#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <functional>
#include <utility>
#include <unordered_map>
using namespace std;

using XR::ConfigReader;

//#define USE_DEBUG_MODE

namespace XRwy {
	struct FaceIndex {
		int a, b, c;
	};
}

namespace {
	using Poly3 = CGAL::Polyhedron_3<CGAL::FloatKernel>;
	using HalfedgeDS3 = Poly3::HalfedgeDS;
	using Vertex3 = HalfedgeDS3::Vertex;
	using Point3 = Vertex3::Point;

	template <class HDS>
	struct PolyhedronBuilder : public CGAL::Modifier_base<HDS> {
		PolyhedronBuilder(const vector<Point3> &vs, const vector<XRwy::FaceIndex> &fs) : vs_ptr(&vs), fs_ptr(&fs) {}

		void operator() (HDS &hds) {
			using Vertex = HDS::Vertex;
			using Point = Vertex::Point;

			const auto &vs = *vs_ptr;
			const auto &fs = *fs_ptr;

			CGAL::Polyhedron_incremental_builder_3<HDS> incr(hds, true);

			incr.begin_surface(vs.size(), fs.size());

			for (int i = 0; i < vs.size(); ++i)
				incr.add_vertex(vs[i]);
			for (int i = 0; i < fs.size(); ++i) {
				incr.begin_facet();
				incr.add_vertex_to_facet(fs[i].a);
				incr.add_vertex_to_facet(fs[i].b);
				incr.add_vertex_to_facet(fs[i].c);
				incr.end_facet();
			}

			incr.end_surface();
		}

	private:
		const vector<Point3> *vs_ptr;
		const vector<XRwy::FaceIndex> *fs_ptr;
	};
}

namespace XRwy {

	using ICollisionObject = WR::ICollisionObject;

	HairSimulator *simulator = nullptr;
	ICollisionObject *collision_object = nullptr;

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

		//first initialize the collision object
		const auto & col_it = g_paramDict.find("collisionfile");
		if (col_it != g_paramDict.end()) {
			const auto & col_file_path = col_it->second;
			collision_object = WR::CreateGridCollisionObject(col_file_path.c_str());
		}

		const string hairmodel_type = g_paramDict.find("hairmodel")->second;
		if (hairmodel_type == "loader")
			simulator = new HairLoaderSimulator;
		else if (hairmodel_type == "full")
			simulator = new HairFullModelSimulator(collision_object);
		else
			assert(false);
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
		simulator->on_change_parameter(key, value);
#else
		/* use for testing */
		ofstream fout;
		fout.open(root_path + "UpdateParameter.txt", ios::out);
		fout << "key: " << key << endl;
		fout << "value: " << value << endl;
#endif // !USE_DEBUG_MODE

		return 0;
	}

	int UpdateHairEngine(const float head_matrix[16], float *particle_positions, float *particle_directions, float delta_time) {
#ifndef USE_DEBUG_MODE
		float mat4[16] = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		};
		simulator->on_frame(head_matrix, particle_positions, particle_directions, delta_time, collision_object, mat4);
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
		SAFE_DELETE(collision_object);
#else
		/* use for testing */
		ofstream fout;
		fout.open(root_path + "ReleaseHairEngine.txt", ios::out);
		fout << "Release Hair Engine" << endl;
#endif // !USE_DEBUG_MODE
	}

	int GetHairParticleCount() {
#ifndef USE_DEBUG_MODE
		return simulator->get_particle_count();
#else
		/* use for testing */
		return 12345;
#endif // !USE_DEBUG_MODE
	}

	int GetParticlePerStrandCount() {
#ifndef USE_DEBUG_MODE
		return simulator->get_particle_per_strand_count();
#else
		/* use for testing */
		return 67890;
#endif
	}

	struct UnorderedMapEqualToPoint3 {
		bool operator() (const Point3 &p1, const Point3 &p2) const {
			return abs(p1[0] - p2[0]) + abs(p1[1] - p2[1]) + abs(p1[2] - p2[2]) < 1e-7;
		}
	};

	struct UnorderedMapHashPoint3 {
		size_t operator() (const Point3 &p) const {
			auto func = std::hash<float>{};
			return func(p[0]) ^ func(p[1]) ^ func(p[2]);
		}
	};

	int InitCollisionObject(int nvertices, int nfaces, const float *vertices, const int *faces) {

		size_t buck_size = 100000;

		Poly3 poly;

		unordered_map<Point3, int, UnorderedMapHashPoint3, UnorderedMapEqualToPoint3> remap_v;
		vector<FaceIndex> remap_fv;
		vector<Point3> remap_vv;


		for (int i = 0, count = 0; i < nvertices * 3; i += 3) {
			auto v = Point3(vertices[i], vertices[i + 1], vertices[i + 2]);
			if (remap_v.find(v) == remap_v.end()) {
				remap_v.insert(pair<Point3, int>(v, count++));
			}
		}

		remap_vv.reserve(remap_v.size());
		for (auto it = remap_v.begin(); it != remap_v.end(); ++it)
			remap_vv.push_back(it->first);

		for (int i = 0; i < nfaces * 3; i += 3) {
			auto find_new_idx = [&remap_v, &vertices](const int idx) -> int {
				auto v = Point3(vertices[idx * 3], vertices[idx * 3 + 1], vertices[idx * 3 + 2]);
				return remap_v.find(v)->second;
			};

			remap_fv.push_back(FaceIndex{ find_new_idx(faces[i]), find_new_idx(faces[i + 1]), find_new_idx(faces[i + 2]) });
		}

		PolyhedronBuilder<Poly3::HalfedgeDS> builder(remap_vv, remap_fv);
		poly.delegate(builder);

		collision_object = WR::CreateGridCollisionObject2(poly);

		WriteGridCollisionObject(collision_object, "C:\\Codes\\Projects\\SJTU Final Project\\dxsimhairdata\\grid\\box-1-1-1-078.grid2");
		
		//creating test point


		/* use for testing */
		//ofstream fout("C:\\Users\\vivid\\Desktop\\InitCollisionObject.txt", ios::out);
		//
		//fout << "vertices: " << endl;
		//for (int i = 0; i < nvertices * 3; i += 3)
		//	fout << vertices[i] << " " << vertices[i + 1] << " " << vertices[i + 2] << endl;
		//fout << "faces: " << endl;
		//for (int i = 0; i < nfaces * 3; i += 3)
		//	fout << faces[i] << " " << faces[i + 1] << " " << faces[i + 2] << endl;

		return 0;
	}
}