#pragma once
#include "macros.h"
#include "HairStructs.h"
#include "GroupPBD.h"

namespace WR
{
	class ICollisionObject;
}

namespace XRwy
{
	class SkinningAndHairBodyCollisionEngine:
		public HairLoader
	{
	public:
		SkinningAndHairBodyCollisionEngine();
		~SkinningAndHairBodyCollisionEngine();

	protected:
		SkinningInfo*	skinInfo = nullptr;
		ReconsReader*	reader = nullptr;
		int				sampleRate = 1;

		ExternPtr HairGeometry*	skinResult = nullptr;
	};

	class SkinningAndHairBodyCollisionEngineCPU :
		public SkinningAndHairBodyCollisionEngine
	{
	public:
		SkinningAndHairBodyCollisionEngineCPU();
		~SkinningAndHairBodyCollisionEngineCPU();

		bool loadFile(const char* fileName, HairGeometry * geom);
		void rewind();
		void nextFrame();
		void jumpTo(int frameNo);

	private:
		void interpolate();
		void hairBodyCollision();

		int hairRendererVersion = 1;
		WR::ICollisionObject* pCollision = nullptr;
	};

	class ReducedModel :
		public HairLoader
	{
	public:
		ReducedModel();
		~ReducedModel();

		bool loadFile(const char* fileName, HairGeometry* geom);
		void rewind();
		void nextFrame();
		void jumpTo(int frameNo);

	private:
		HairLoader*			skinning = nullptr;
		IHairCorrection*	pPDB = nullptr;

		ExternPtr HairGeometry* hairGeom = nullptr;
	};
}
