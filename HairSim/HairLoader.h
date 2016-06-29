#pragma once
#include <fstream>

#include "wrMacro.h"
#include "HairGeometry.h"

namespace XRwy
{
    class HairAnimationLoader
    {
        COMMON_PROPERTY(size_t, nFrame);
        COMMON_PROPERTY(size_t, curFrame);

    public:
        class IHelper
        {
        public:
            virtual void init(size_t &nf, size_t &np) = 0;
            virtual void readFrame(float* pos, size_t np) = 0;
            virtual void readFrame20(float* rigidTrans, float* pos, float* dir, size_t np){ assert(0); }
            virtual bool hasNextFrame(size_t &id) = 0;
        };

    public:
        HairAnimationLoader(){}
        ~HairAnimationLoader();

        bool loadFile(const char* fileName, HairGeometry * geom);
        void rewind();
        void nextFrame();
        void jumpTo(int frameNo);

    private:
        void jumpTo();
        void readFrame();
        bool hasNextFrame();

        HairGeometry*   pCurrentHair = nullptr;
        IHelper*        helper = nullptr;

        std::ifstream   file;
        std::streampos  firstFrame = 0;
    };

}