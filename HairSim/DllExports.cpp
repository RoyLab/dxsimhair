
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

#include <iostream>
#include <string>
#include <fstream>
using namespace std;

using XR::ConfigReader;

//#define USE_DEBUG_MODE

namespace {
	template <class HDS>
	struct PolyhedronBuilder: public CGAL::Modifier_base<HDS> {
		PolyhedronBuilder(const int nvertices_, const int nfaces_, const float *vertices_, const int *faces_) : nvertices(nvertices_), nfaces(nfaces_), vertices(vertices_), faces(faces_) {}

		void operator() (HDS &hds) {
			CGAL::Polyhedron_incremental_builder_3<HDS> incr(hds, true);

			incr.begin_surface(nvertices, nfaces);
			using Vertex = HDS::Vertex;
			using Point = HDS::Point;
			for (int i = 0; i < nvertices * 3; i += 3)
				incr.add_vertex(Point(vertices[i], vertices[i + 1], vertices[i + 2]));
			for (int i = 0; i < nfaces * 3; i += 3) {
				incr.begin_facet();
				incr.add_vertex_to_facet(faces[i]);
				incr.add_vertex_to_facet(faces[i + 1]);
				incr.add_vertex_to_facet(faces[i + 2]);
				incr.end_facet();
			}
		}

	private:
		int nvertices;
		int nfaces;
		float *vertices;
		int *faces;
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

		const string hairmodel_type = g_paramDict.find("hairmodel")->second;
		if (hairmodel_type == "loader")
			simulator = new HairLoaderSimulator;
		else if (hairmodel_type == "full")
			simulator = new HairFullModelSimulator;
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
		simulator->on_frame(head_matrix, particle_positions, particle_directions, delta_time);
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

	int InitCollisionObject(const int nvertices, const int nfaces, const float *vertices, const int *faces) {

		using Poly3 = WR::Polyhedron_3_FaceWithId;
		Poly3 poly;
		PolyhedronBuilder<Poly3::HalfedgeDS> builder(nvertices, nfaces, vertices, faces);
		poly.delegate(builder);

		collision_object = createCollisionObject(poly);
		

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