#pragma once
#include "GroupPBD.h"
#include "macros.h"

namespace WR
{
	class ICollisionObject;
}

namespace xhair
{
	class SkinningAndHairBodyCollisionEngine:
		public IHairLoader
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
		public IHairLoader
	{
	public:
		ReducedModel(int para);
		~ReducedModel();

		bool loadFile(const char* fileName, HairGeometry* geom);
		void rewind();
		void nextFrame();
		void jumpTo(int frameNo);

	private:
		IHairLoader*			skinning = nullptr;
		IHairCorrection*	pPDB = nullptr;
		bool				bHairBody, bPDB;

		ExternPtr HairGeometry* hairGeom = nullptr;
	};
}
