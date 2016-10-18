#include "precompiled.h"
#include "HairLoader.h"
#include "XRwy_h.h"
#include "HairSampleSelector.h"

namespace XRwy
{
    class BinaryHelper :
        public HairAnimationLoader::IHelper
    {
    public:
        BinaryHelper(std::ifstream& input) :file(input){}

        void init(size_t &nf, size_t &np)
        {
            char bytes[4];
            file.read(bytes, 4);
            nf = *reinterpret_cast<int*>(bytes);

            file.read(bytes, 4);
            np = *reinterpret_cast<int*>(bytes);
        }

        void readFrame(float* pos, size_t np)
        {
            char* bytes = reinterpret_cast<char*>(pos);
            file.read(bytes, sizeof(float)*np * 3);
        }

        void readFrame20(float* rigidTrans, float* pos, float* dir, size_t np)
        {
            char* bytes = reinterpret_cast<char*>(rigidTrans);
            file.read(bytes, sizeof(float)* 16);

            bytes = reinterpret_cast<char*>(pos);
            file.read(bytes, sizeof(float)*np * 3);

            bytes = reinterpret_cast<char*>(dir);
            file.read(bytes, sizeof(float)*np * 3);
        }

		void readFrame20sample(float* rigidTrans, float* pos, float* dir, size_t ns, size_t factor, int sampleRate)
		{
			char* bytes = reinterpret_cast<char*>(rigidTrans);
			file.read(bytes, sizeof(float) * 16);

			float* buffer = new float[ns*factor * 3];

			bytes = reinterpret_cast<char*>(buffer);
			file.read(bytes, sizeof(float)*ns*factor * 3);

			size_t nSample = ns / sampleRate;
			for (int i = 0; i < nSample; i++)
				memcpy(pos + i*factor * 3, buffer + i*sampleRate*factor * 3, sizeof(float)*factor * 3);

			file.read(bytes, sizeof(float)*ns*factor * 3);
			for (int i = 0; i < nSample; i++)
				memcpy(dir + i*factor * 3, buffer + i*sampleRate*factor * 3, sizeof(float)*factor * 3);

			delete[] buffer;
		}

		void readFrame20sampleInstance(float* rigidTrans, float* pos, float* dir,
			size_t nRealSz, size_t factor, HairSampleSelector* sampler) 
		{
			char* bytes = reinterpret_cast<char*>(rigidTrans);
			file.read(bytes, sizeof(float) * 16);

			float* buffer = new float[nRealSz*factor * 3];

			bytes = reinterpret_cast<char*>(buffer);

			file.read(bytes, sizeof(float)*nRealSz*factor * 3);
			sampler->ResetIterator();
			for (int i = 0; i < sampler->GetNumberOfStrand(); i++)
				memcpy(pos + i*factor * 3, buffer + sampler->GetNextId()*factor * 3, sizeof(float)*factor * 3);

			file.read(bytes, sizeof(float)*nRealSz*factor * 3);
			sampler->ResetIterator();
			for (int i = 0; i < sampler->GetNumberOfStrand(); i++)
				memcpy(dir + i*factor * 3, buffer + sampler->GetNextId()*factor * 3, sizeof(float)*factor * 3);

			delete[] buffer;
		}

        bool hasNextFrame(size_t &id)
        {
            char bytes[4];
            file.read(bytes, 4);

            if (file.eof())
                return false;

            id = *reinterpret_cast<int*>(bytes);
            return true;
        }

    private:
        std::ifstream& file;
    };

    HairAnimationLoader::~HairAnimationLoader()
    {
        if (file && file.is_open())
            file.close();

		SAFE_DELETE(helper);
		SAFE_DELETE(sampler);
		SAFE_DELETE(restState);
    }

    bool HairAnimationLoader::loadFile(const char* fileName, HairGeometry * geom)
    {
        file = std::ifstream(fileName, std::ios::binary);
        helper = new BinaryHelper(file);

        if (!file.is_open()) throw std::exception("file not found!");

		/* for sample display */
		helper->init(m_nFrame, nRealParticle);
		int factor = std::stoi(g_paramDict["particleperstrand"]);
		nRealStrand = nRealParticle / factor;
		firstFrame = file.tellg();

		restState = new HairGeometry;
		geom->particlePerStrand = factor;
		geom->nParticle = nRealParticle;
		geom->nStrand = nRealStrand;

		sampler = new HairSampleSelector(geom, 1); // null sampler
		sampler->FillInHairStructs(restState);
		restState->allocMemory();

		pCurrentHair = restState;
		rewind();

		pCurrentHair = geom;
		SAFE_DELETE(sampler);

		int sampleRate = std::stoi(g_paramDict["hairsample"]);
		int groupSampleNumber = std::stoi(g_paramDict["hairsamplegroup"]);
		int groupSampleSeed = std::stoi(g_paramDict["hairsamplegroupseed"]);
		sampler = new HairSampleSelector(restState, sampleRate, groupSampleNumber, groupSampleSeed);
		sampler->FillInHairStructs(geom);
		geom->allocMemory();

		DirectX::XMStoreFloat4x4(&geom->worldMatrix, DirectX::XMMatrixIdentity());

		rewind();
        return true;
    }

    void HairAnimationLoader::rewind()
    {
		jumpTo(0);
    }

    void HairAnimationLoader::nextFrame()
    {
		if (!hasNextFrame())
		{
			file.clear();
            rewind();
		}
		else
		{
			set_curFrame(get_curFrame() + 1);
			readFrame();
		}
    }

    bool HairAnimationLoader::hasNextFrame()
    {
        return helper->hasNextFrame(m_curFrame);
    }

    void HairAnimationLoader::readFrame()
    {
        auto hair = pCurrentHair;
        helper->readFrame20sampleInstance((float*)(&hair->rigidTrans),
            (float*)hair->position, (float*)hair->direction, 
			nRealStrand, hair->particlePerStrand, sampler);
    }

    void HairAnimationLoader::jumpTo(int frameNo)
    {
        set_curFrame(frameNo);
		file.seekg(firstFrame + std::streamoff(get_curFrame()*(sizeof(int) +
			sizeof(float)*(16 + 3 * 2 * nRealParticle))));

		if (hasNextFrame())
			readFrame();
		else
		{
			file.clear();
			rewind();
		}
	}

}