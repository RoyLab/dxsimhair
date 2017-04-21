#pragma once


//The "DebugDll" and "ReleaseDll" configuration define "SIMHAIR_EXPORTS"
//The "Debug" and "Release" configuration define "USE_EXE"
//So when we use "DebugDll" and "ReleaseDll" configuration, "SIMHAIR_DLL" will be "__declspec(dllexport)"
//When we use "Debug" and "Release", "SIMHAIR_DLL" will be <none>
//When other program use ".h" files, since they don't define "SIMHAIR_EXPORTS" and "USE_EXE", then "SIMHAIR_DLL" will be "__declspec(dllimport)"

#ifdef SIMHAIR_EXPORTS
#define SIMHAIR_DLL __declspec(dllexport)
#else
#ifdef USE_EXE
#define SIMHAIR_DLL 
#else
#define SIMHAIR_DLL __declspec(dllimport)
#endif
#endif

#define MAX_PATH_LENGTH 65536

namespace XRwy {
	extern "C" {
		struct HairParameter
		{
			bool b_guide;
			bool b_collision;
			bool b_pbd;

			char root[MAX_PATH_LENGTH];
		};

		struct CollisionParameter
		{
			float correction_tolerance;
			float correction_rate;
			float maxstep;
		};

		struct SkinningParameter
		{
			bool simulateGuide;
		};

		struct PbdParameter
		{
			float detectrange;
			float lambda; // for solving optimization
			float chunksize; // parallel computing chunk
			int maxiteration;
		};

		SIMHAIR_DLL int InitializeHairEngine(
			const HairParameter* param,
			const CollisionParameter* col,
			const SkinningParameter* skin,
			const PbdParameter* pbd
		);

		SIMHAIR_DLL int UpdateParameter(const char* key, const char* value);

		SIMHAIR_DLL int UpdateHairEngine(
			const float head_matrix[16],
			float *particle_positions,
			float *particle_directions = nullptr
		);

		SIMHAIR_DLL void ReleaseHairEngine();

		SIMHAIR_DLL int GetHairParticleCount();

		SIMHAIR_DLL int GetParticlePerStrandCount();
	}
}