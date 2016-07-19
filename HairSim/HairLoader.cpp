#include "HairLoader.h"
#include "XRwy_h.h"

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

			for (int i = 0, count = 0; i < ns; i += sampleRate, count++)
				memcpy(pos + count*factor * 3, buffer + i*factor * 3, sizeof(float)*factor * 3);

			file.read(bytes, sizeof(float)*ns*factor * 3);
			for (int i = 0, count = 0; i < ns; i += sampleRate, count++)
				memcpy(dir + count*factor * 3, buffer + i*factor * 3, sizeof(float)*factor * 3);

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
    }

    bool HairAnimationLoader::loadFile(const char* fileName, HairGeometry * geom)
    {
        pCurrentHair = geom;

        file = std::ifstream(fileName, std::ios::binary);
        helper = new BinaryHelper(file);

        if (!file.is_open()) throw std::exception("file not found!");

		/* for sample display */
		sampleRate = std::stoi(g_paramDict["hairsample"]);

		helper->init(m_nFrame, nRealParticle);
        geom->particlePerStrand = std::stoi(g_paramDict["particleperstrand"]);
		nRealStrand = nRealParticle / geom->particlePerStrand;
		geom->nStrand  = nRealStrand / sampleRate + (sampleRate > 1? 1:0);
		geom->nParticle = geom->nStrand * geom->particlePerStrand;
		DirectX::XMStoreFloat4x4(&geom->worldMatrix, DirectX::XMMatrixIdentity());

        firstFrame = file.tellg();
        m_curFrame = -1;

        geom->position = new XMFLOAT3[geom->nParticle];
        geom->direction = new XMFLOAT3[geom->nParticle];

		nextFrame();
        return true;
    }

    void HairAnimationLoader::rewind()
    {
        file.clear();
        file.seekg(firstFrame);
        set_curFrame(0);
    }

    void HairAnimationLoader::nextFrame()
    {
        if (!hasNextFrame())
        {
            rewind();
            hasNextFrame();
        }
        readFrame();
    }

    bool HairAnimationLoader::hasNextFrame()
    {
        return helper->hasNextFrame(m_curFrame);
    }

    void HairAnimationLoader::readFrame()
    {
        auto hair = pCurrentHair;
        helper->readFrame20sample((float*)(&hair->rigidTrans),
            (float*)hair->position, (float*)hair->direction, 
			nRealStrand, hair->particlePerStrand, sampleRate);

        set_curFrame(get_curFrame() + 1);
    }

    void HairAnimationLoader::jumpTo(int frameNo)
    {
        set_curFrame(frameNo);
        jumpTo();
    }

    void HairAnimationLoader::jumpTo()
    {
        auto hair = pCurrentHair;

        file.seekg(firstFrame + std::streamoff(get_curFrame()*(sizeof(int)+
            sizeof(float)*(16 + 3 * 2 * nRealParticle))));
        if (hasNextFrame())
        {
            helper->readFrame20sample((float*)(&hair->rigidTrans), (float*)hair->position,
                (float*)hair->direction, nRealStrand, hair->particlePerStrand, sampleRate);
        }
    }

}