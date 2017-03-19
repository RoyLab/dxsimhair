#pragma once
#include <map>
#include "ISkinningEngine.h"
//#include "GroupPBD.h

namespace xhair
{
    class StrandWeight
    {
    public:
        int getNumberOfGuides(int id) const;
        std::pair<int, float> getWeights(int id, int pid, int gid) const;
    };

    //struct SkinningInfo
    //{
    //    BlendHairGeometry*			guidances = nullptr;
    //    HairGeometry*				restState = nullptr;

    //    std::vector<int>						groupId;
    //    std::vector<std::vector<int>*>			neighbor;
    //    char									anim2[128];


    //    std::vector<WeightItem>		weights;
    //    std::map<int, int>			global2local;
    //    size_t						nFrame = 0;
    //    size_t						curFrame = -1;
    //};

    class CSkinningEngine : public ISkinningEngine
    {
    public:
        CSkinningEngine(const SkinningEngineParameter& param) {}
        ~CSkinningEngine() {}

        void transport(HairGeometry* hair0, HairGeometry* hair1);

        int getNLeft() const { return rest_state1->nStrand; }
        int getNRight() const { return rest_state2->nStrand; }

    private:
        void calcTrans(const HairGeometry* hair);

        int             pps_ = 0; // particle per strand
        HairGeometry*   rest_state1 = nullptr;
        HairGeometry*   rest_state2 = nullptr;
        StrandWeight*   weights_ = nullptr;

        std::vector<Point3> trans_;
    };

	//class SkinningAndHairBodyCollisionEngine:
	//	public IHairLoader
	//{
	//public:
	//	SkinningAndHairBodyCollisionEngine();
	//	~SkinningAndHairBodyCollisionEngine();

	//protected:
	//	SkinningInfo*	skinInfo = nullptr;
	//	ReconsReader*	reader = nullptr;
	//	HairSampleSelector* sampler = nullptr;

	//	ExternPtr HairGeometry*	skinResult = nullptr;
	//};

	//class SkinningAndHairBodyCollisionEngineCPU :
	//	public SkinningAndHairBodyCollisionEngine
	//{
	//public:
	//	SkinningAndHairBodyCollisionEngineCPU(bool flag);
	//	~SkinningAndHairBodyCollisionEngineCPU();

	//	bool loadFile(const char* fileName, HairGeometry * geom);
	//	void rewind();
	//	void nextFrame();
	//	void jumpTo(int frameNo);

	//private:
	//	void interpolate();
	//	void hairBodyCollision();

	//	int hairRendererVersion = 1;
	//	WR::ICollisionObject* pCollision = nullptr;
	//	bool bHairBody;
	//};

	//class ReducedModel :
	//	public IHairLoader
	//{
	//public:
	//	ReducedModel(int para);
	//	~ReducedModel();

	//	bool loadFile(const char* fileName, HairGeometry* geom);
	//	void rewind();
	//	void nextFrame();
	//	void jumpTo(int frameNo);

	//private:
	//	IHairLoader*			skinning = nullptr;
	//	IHairCorrection*	pPDB = nullptr;
	//	bool				bHairBody, bPDB;

	//	ExternPtr HairGeometry* hairGeom = nullptr;
	//};
}
