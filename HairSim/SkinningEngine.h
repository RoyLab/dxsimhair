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
		HairSampleSelector* sampler = nullptr;

		ExternPtr HairGeometry*	skinResult = nullptr;
	};

	class SkinningAndHairBodyCollisionEngineCPU :
		public SkinningAndHairBodyCollisionEngine
	{
	public:
		SkinningAndHairBodyCollisionEngineCPU(bool flag);
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
		bool bHairBody;
	};

	class ReducedModel :
		public HairLoader
	{
	public:
		ReducedModel(int para);
		~ReducedModel();

		bool loadFile(const char* fileName, HairGeometry* geom);
		void rewind();
		void nextFrame();
		void jumpTo(int frameNo);

	private:
		HairLoader*			skinning = nullptr;
		IHairCorrection*	pPDB = nullptr;
		bool				bHairBody, bPDB;

		ExternPtr HairGeometry* hairGeom = nullptr;
	};
}
