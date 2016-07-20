#include "macros.h"
#include "XRwy_h.h"
#include "SkinningEngine.h"
#include "ReconsReader.h"

namespace XRwy
{
	ReducedModel::ReducedModel()
	{
		skinning = new SkinningAndHairBodyCollisionEngineCPU;
		pPDB = new GroupPBD;
	}

	ReducedModel::~ReducedModel()
	{
		SAFE_DELETE(skinning);
		SAFE_DELETE(pPDB);
	}

	bool ReducedModel::loadFile(const char * fileName, HairGeometry * geom)
	{
		bool hr;
		hairGeom = geom;

		V_RETURN(skinning->loadFile(fileName, hairGeom));
		V_RETURN(pPDB->initialize(hairGeom));

		set_nFrame(skinning->get_nFrame());
		set_curFrame(-1);

		return true;
	}

	void ReducedModel::rewind()
	{
		skinning->rewind();
		pPDB->solve(hairGeom);
		set_curFrame(0);
	}

	void ReducedModel::nextFrame()
	{
		skinning->nextFrame();
		pPDB->solve(hairGeom);
		set_curFrame(skinning->get_curFrame());
	}

	void ReducedModel::jumpTo(int frameNo)
	{
		if (frameNo % get_nFrame() == get_curFrame()) return;

		skinning->jumpTo(frameNo);
		pPDB->solve(hairGeom);
		set_curFrame(skinning->get_curFrame());
	}

	SkinningAndHairBodyCollisionEngine::SkinningAndHairBodyCollisionEngine()
	{
		skinInfo = new SkinningInfo;
		reader = new ReconsReader;
	}

	SkinningAndHairBodyCollisionEngine::~SkinningAndHairBodyCollisionEngine()
	{
		if (skinInfo)
		{
			SAFE_DELETE(skinInfo->restState);
			SAFE_DELETE(skinInfo->guidances);
			delete skinInfo;
			skinInfo = nullptr;
		}

		SAFE_DELETE(reader);
	}

	bool SkinningAndHairBodyCollisionEngineCPU::loadFile(const char* fileName, HairGeometry * geom)
	{
		bool hr;
		skinResult = geom;
		sampleRate = std::stoi(g_paramDict["hairsample"]);

		reader->loadFile(fileName, skinInfo);
		set_nFrame(reader->get_nFrame());
		set_curFrame(-1);

		return true;
	}

	void SkinningAndHairBodyCollisionEngineCPU::rewind()
	{
		if (get_curFrame() == 0) return;

		reader->rewind();
		interpolate();
		hairBodyCollision();
		set_curFrame(0);
	}

	void SkinningAndHairBodyCollisionEngineCPU::nextFrame()
	{
		reader->nextFrame();
		interpolate();
		hairBodyCollision();

		set_curFrame(reader->get_curFrame());
	}

	void SkinningAndHairBodyCollisionEngineCPU::jumpTo(int frameNo)
	{
		if (get_curFrame() == frameNo) return;

		reader->jumpTo(frameNo);
		interpolate();
		hairBodyCollision();
		set_curFrame(reader->get_curFrame());
	}
}

