#pragma once
#include "macros.h"
#include "HairStructs.h"

namespace XRwy
{
	class SkinningAndHairBodyCollisionEngine:
		public HairLoader
	{
	public:
		bool loadFile(const char* fileName, HairGeometry * geom);
		void rewind();
		void nextFrame();
		void jumpTo(int frameNo);

	private:
		SkinningInfo* skinInfo = nullptr;
		ReconsReader* reader = nullptr;
	};

	class SkinningAndHairBodyCollisionEngineCPU :
		public SkinningAndHairBodyCollisionEngine
	{
	private:

	};

	class ReducedModel :
		public HairLoader
	{
	public:
		ReducedModel();
		~ReducedModel();

		bool loadFile(const char* fileName, HairGeometry * geom);
		void rewind();
		void nextFrame();
		void jumpTo(int frameNo);

	private:
		HairLoader*		skinning = nullptr;
		HairGeometry*	hairGeom = nullptr;
		GroupPBD*		PDB = nullptr;
	};
}
