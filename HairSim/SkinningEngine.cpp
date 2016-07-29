#include "LevelSet.h"
#include "macros.h"
#include "XRwy_h.h"
#include "SkinningEngine.h"
#include "ReconsReader.h"
#include "wrMath.h"

#ifdef _DEBUG
#define ADF_FILE L"../../models/head_d"
//#include <vld.h>
#else
#define ADF_FILE L"../../models/head"
#endif

namespace XRwy
{
	ReducedModel::ReducedModel()
	{
		skinning = new SkinningAndHairBodyCollisionEngineCPU;
		pPDB = CreateHairCorrectionObject();
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

	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	///////////////////////SkinningEngine/////////////////////////
	//////////////////////////////////////////////////////////////

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
			for (auto ptr : skinInfo->neighbor)
				delete ptr;

			delete skinInfo;
			skinInfo = nullptr;
		}

		SAFE_DELETE(reader);
	}

	SkinningAndHairBodyCollisionEngineCPU::SkinningAndHairBodyCollisionEngineCPU()
	{
		hairRendererVersion = std::stoi(g_paramDict["hairrendversion"]);
		pCollision = WR::loadCollisionObject(ADF_FILE);
	}

	SkinningAndHairBodyCollisionEngineCPU::~SkinningAndHairBodyCollisionEngineCPU()
	{
		SAFE_DELETE(pCollision);
	}

	bool SkinningAndHairBodyCollisionEngineCPU::loadFile(const char* fileName, HairGeometry * geom)
	{
		bool hr;
		skinResult = geom;
		sampleRate = std::stoi(g_paramDict["hairsample"]);

		V_RETURN(reader->loadFile(fileName, skinInfo));

		skinResult->nStrand = skinInfo->restState->nStrand / sampleRate;
		skinResult->particlePerStrand = skinInfo->restState->particlePerStrand;
		skinResult->nParticle = skinResult->nStrand * skinResult->particlePerStrand;

		skinResult->allocMemory();
		DirectX::XMStoreFloat4x4(&skinResult->worldMatrix, DirectX::XMMatrixIdentity());

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

	void SkinningAndHairBodyCollisionEngineCPU::interpolate()
	{
		CopyMemory(skinResult->rigidTrans, skinInfo->guidances->rigidTrans, sizeof(float) * 16);
		mat4x4 rigid;
		mat4x4_transpose(rigid, reinterpret_cast<vec4*>(skinResult->rigidTrans));
		auto factor = skinInfo->guidances->particlePerStrand;
		for (int i = 0; i < skinResult->nStrand; i++)
		{
			auto& weight = skinInfo->weights[i*sampleRate];
			for (int i2 = 0; i2 < factor; i2++)
			{
				size_t idx = i * factor + i2;
				size_t idxNosample = i * factor * sampleRate + i2;
				vec3 tmp{ 0, 0, 0 }, tmp2;
				for (int j = 0; j < weight.n; j++)
				{
					size_t guideId = weight.guideID[j];
					vec3_scale(tmp2,
						reinterpret_cast<float*>(&skinInfo->guidances->trans[guideId*factor+i2]),
						weight.weights[j]);
					vec3_add(tmp, tmp, tmp2);
				}
				auto dest = reinterpret_cast<float*>(skinResult->position + idx);
				auto rest = reinterpret_cast<float*>(&skinInfo->restState->position[idxNosample]);
				vec3 pos;
				mat4x4_mul_vec3(pos, rigid, rest);
				vec3_add(dest, pos, tmp);
			}

			if (hairRendererVersion == 1)
			{
				vec3 dir;
				float* pos1, *pos2;
				for (int j = 1; j < factor; j++)
				{
					pos1 = reinterpret_cast<float*>(skinResult->position + i*factor + j);
					pos2 = reinterpret_cast<float*>(skinResult->position + i*factor + j + 1);
					vec3_sub(dir, pos2, pos1);
					float norm = vec3_len(dir);
					vec3_scale(dir, dir, 1 / norm);
					memcpy(skinResult->direction + i*factor + j, dir, sizeof(vec3));
				}
				memcpy(skinResult->direction + i*factor, skinResult->direction + i*factor + 1, sizeof(vec3));
			}
		}
	}


	void SkinningAndHairBodyCollisionEngineCPU::hairBodyCollision()
	{
		mat4x4 rigid, rigidInv;
		mat4x4_transpose(rigid, reinterpret_cast<vec4*>(skinResult->rigidTrans));
		mat4x4_invert(rigidInv, rigid);
		for (size_t i = 0; i < skinResult->nParticle; i++)
		{
			if (i % skinResult->particlePerStrand == 0) continue;
			vec3 pos; mat4x4_mul_vec3(pos, rigidInv, reinterpret_cast<float*>(skinResult->position+i));
			WR::ICollisionObject::Point_3 p1, p0 = WR::ICollisionObject::Point_3(pos[0], pos[1], pos[2]);
			bool isCollide = pCollision->position_correlation(p0, &p1, 6e-2f);
			if (isCollide)
			{
				vec3 p;
				WR::convert3(p, p1);
				mat4x4_mul_vec3(pos, rigid, p);
				CopyMemory(skinResult->position + i, pos, sizeof(vec3));
			}
		}
	}
}

