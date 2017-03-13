#pragma once
#include "ISkinningEngine.h"
//#include "GroupPBD.h

namespace xhair
{
    struct SkinningInfo
    {
        BlendHairGeometry*			guidances = nullptr;
        HairGeometry*				restState = nullptr;

        std::vector<int>						groupId;
        std::vector<std::vector<int>*>			neighbor;
        char									anim2[128];


        std::vector<WeightItem>		weights;
        std::map<int, int>			global2local;
        size_t						nFrame = 0;
        size_t						curFrame = -1;
    };

    class CSkinningEngine : public ISkinningEngine
    {
    public:
        CSkinningEngine(const SkinningEngineParameter& param) {}
        ~CSkinningEngine() {}

        void transport(HairGeometry* hair0, HairGeometry* hair1);

    private:
        SkinningInfo*	skinInfo = nullptr;
    };

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
