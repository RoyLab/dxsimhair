#include "HairLoader.h"

namespace XRwy
{
    namespace
    {
        const int N_PARTICLE_PER_STRAND = 25;
    }

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

        helper->init(m_nFrame, geom->nParticle);
        geom->particlePerStrand = N_PARTICLE_PER_STRAND;
        geom->nStrand = geom->nParticle / geom->particlePerStrand;

        firstFrame = file.tellg();
        m_curFrame = -1;

        geom->position = new XMFLOAT3[geom->nParticle];
        geom->direction = new XMFLOAT3[geom->nParticle];

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
        helper->readFrame20((float*)(&hair->rigidTrans), 
            (float*)hair->position, (float*)hair->direction, hair->nParticle);

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
            sizeof(float)*(16 + 3 * 2 * hair->nParticle))));
        if (hasNextFrame())
        {
            helper->readFrame20((float*)(&hair->rigidTrans), (float*)hair->position,
                (float*)hair->direction, hair->nParticle);
        }
    }

}