#pragma once
#include "IHair.h"
#include "Parameter.h"
#include "wrMacro.h"
#include <fstream>

namespace WR
{
    class CacheHair :
        public IHair
    {
        COMMON_PROPERTY(size_t, nParticle);
    public:
        CacheHair(){}
        ~CacheHair(){}

        bool loadFile(const char* fileName);
        bool rewind();
        bool nextFrame();
        bool getFrameNumber() const;

        virtual size_t n_strands() const;
        virtual const float* get_visible_particle_position(size_t i, size_t j);
        virtual void onFrame(Mat3 world, float fTime, float fTimeElapsed, void* = nullptr);

    private:
        std::streampos firstFrame;
    };
}