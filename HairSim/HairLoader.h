#pragma once
#include <fstream>

#include "macros.h"
#include "HairStructs.h"

namespace XRwy
{

    class HairAnimationLoader:
		public HairLoader
    {
    public:
        class IHelper
        {
        public:
            virtual void init(size_t &nf, size_t &np) = 0;
            virtual void readFrame(float* pos, size_t np) = 0;
			virtual void readFrame20(float* rigidTrans, float* pos, float* dir, size_t np) { assert(0); }
			virtual void readFrame20sample(float* rigidTrans, float* pos, float* dir, 
				size_t ns, size_t factor, int sampleRate){ assert(0); }
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

		size_t			nRealParticle, nRealStrand;

        std::ifstream   file;
        std::streampos  firstFrame = 0;

		int				sampleRate = 1;
    };

}