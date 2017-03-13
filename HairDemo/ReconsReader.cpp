#include "precompiled.h"
#include <afxwin.h>
#include "ReconsReader.h"

namespace xhair
{
	namespace
	{
		enum { 
            iGuideHair,
            iInitialFrame,
            iWeights, 
            iGroup, 
            iNeighboring, 
            iInterpolation 
        };
	}

	ReconsReader::ReconsReader()
	{
		set_nFrame(-1);
		set_curFrame(-1);
	}

	ReconsReader::~ReconsReader()
	{
		if (file && file.is_open())
			file.close();
	}

	bool ReconsReader::loadFile(const char* fileName, SkinningInfo* skinHair)
	{
		hair = skinHair;
		file = std::ifstream(fileName, std::ios::binary);

		if (!file.is_open()) throw std::exception("file not found!");

		hair->restState = new HairGeometry;
		hair->guidances = new BlendHairGeometry;

		readHead(hair->restState);
		readHead2(hair->guidances);

		readShortcut();

		setupGuideHair();
		setupRestState();
		setupWeights();

		if (fileShortcut[iGroup] > 0)
			setupGroupSection();

		if (fileShortcut[iNeighboring] > 0)
			setupNeighbSection();

		if (fileShortcut[iInterpolation] > 0)
			setupInplSection();

		file.clear();
		set_nFrame(hair->nFrame);
		rewind(); // only rewind guide hair

		return true;
	}

	void ReconsReader::readHead(HairGeometry* geom)
	{
		Read4Bytes(file, geom->nParticle);
		Read4Bytes(file, geom->nStrand);
		Read4Bytes(file, geom->particlePerStrand);
	}

	void ReconsReader::readHead2(BlendHairGeometry* guidance)
	{
		Read4Bytes(file, guidance->nStrand);
		guidance->particlePerStrand = hair->restState->particlePerStrand;
		guidance->nParticle = guidance->particlePerStrand * guidance->nStrand;

		Read4Bytes(file, hair->nFrame);
		char ch[128];
		file.read(ch, 120);
	}

	void ReconsReader::readShortcut()
	{
		ZeroMemory(fileShortcut, sizeof(size_t) * NUM_SECTION);
		for (int i = 0; i < NUM_SECTION; i++)
			Read4Bytes(file, fileShortcut[i]);
	}

	void ReconsReader::setupGuideHair()
	{
		file.seekg(fileShortcut[iGuideHair]);
		auto guide = hair->guidances;
		for (int i = 0; i < guide->nStrand; i++)
		{
			int id;
			Read4Bytes(file, id);
			hair->global2local[id] = i;
		}

        guide->position.reset(new Point3[guide->nParticle]);
        guide->direction.reset(new Point3[guide->nParticle]);
        guide->trans.resize(guide->nParticle);

		ReadNBytes(file, guide->position.get(), sizeof(Point3)*guide->nParticle);
		ReadNBytes(file, guide->direction.get(), sizeof(Point3)*guide->nParticle);

	}

	void ReconsReader::setupRestState()
	{
		file.seekg(fileShortcut[iInitialFrame]);
		auto rest = hair->restState;

        rest->position.reset(new Point3[rest->nParticle]);
        rest->direction.reset(new Point3[rest->nParticle]);

		ReadNBytes(file, rest->position.get(), sizeof(Point3)*rest->nParticle);
		ReadNBytes(file, rest->direction.get(), sizeof(Point3)*rest->nParticle);
		//DirectX::XMStoreFloat4x4(&rest->worldMatrix, DirectX::XMMatrixIdentity());
		ZeroMemory(&rest->rigidTrans, sizeof(rest->rigidTrans));
	}

	void ReconsReader::setupWeights()
	{
		file.seekg(fileShortcut[iWeights] + 4);
		size_t nStrand = hair->restState->nStrand;
		hair->weights.reserve(nStrand);

		for (int i = 0; i < nStrand; i++)
		{
			hair->weights.emplace_back();
			auto& back = hair->weights.back();
			Read4Bytes(file, back.n);
			assert(back.n <= WeightItem::MAX_GUIDANCE);
			ReadNBytes(file, back.guideID, back.n * sizeof(int));
			ReadNBytes(file, back.weights, back.n * sizeof(float));

			for (int j = 0; j < back.n; j++)
				back.guideID[j] = hair->global2local[back.guideID[j]];
		}
	}

	void ReconsReader::setupGroupSection()
	{
		file.seekg(fileShortcut[iGroup]);
		hair->groupId.clear();
		auto ptr = new int[hair->restState->nStrand];
		auto n = hair->restState->nStrand;

		ReadNBytes(file, ptr, sizeof(int) * n);
		hair->groupId.assign(ptr, ptr + n);
		delete[] ptr;
	}

	void ReconsReader::setupNeighbSection()
	{
		file.seekg(fileShortcut[iNeighboring]);
		auto n = hair->guidances->nStrand;
		assert(!hair->neighbor.size());
		for (int i = 0; i < n; i++)
		{
			int tmp;
			Read4Bytes(file, tmp);
			auto ptr = new std::vector<int>(tmp);
			hair->neighbor.push_back(ptr);
			for (int j = 0; j < tmp; j++)
				Read4Bytes(file, ptr->at(j));
		}
	}

	void ReconsReader::setupInplSection()
	{
		file.seekg(fileShortcut[iInterpolation]);
		ZeroMemory(hair->anim2, sizeof(char) * 128);
		file.read(hair->anim2, 128);
	}

	void ReconsReader::rewind()
	{
		jumpTo(0);
	}

	void ReconsReader::nextFrame()
	{
		set_curFrame(get_curFrame() + 1);
		if (get_curFrame() >= get_nFrame())
		{
			jumpTo(0);
			return;
		}

		int frameNo = -1;
		Read4Bytes(file, frameNo);
		assert(frameNo == get_curFrame());

		ReadNBytes(file, &hair->guidances->rigidTrans, sizeof(float) * 16);
		for (int i = 0; i < hair->guidances->nParticle; i++)
		{
			file.seekg(sizeof(float) * 9, std::ios_base::cur);
			ReadNBytes(file, &hair->guidances->trans[i], sizeof(float) * 3);
		}
	}

	void ReconsReader::jumpTo(int frameNo)
	{
		const size_t offset = 72 + 48 * hair->guidances->nParticle;

		file.seekg(fileShortcut[iGuideHair]
			+ hair->guidances->nStrand * 4
			+ hair->guidances->nParticle * 24
			+ offset * frameNo);

		set_curFrame(-1);
		nextFrame();
	}

}