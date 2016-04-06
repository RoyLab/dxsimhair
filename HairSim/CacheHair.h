#pragma once
#include <vector>
#include <fstream>

#include "IHair.h"
#include "Parameter.h"
#include "wrMacro.h"

namespace WR
{



    class CacheHair :
        public IHair
    {
        COMMON_PROPERTY(size_t, nParticle);
        COMMON_PROPERTY(size_t, nFrame);
        COMMON_PROPERTY(size_t, curFrame);

        class IHelper
        {
        public:
            virtual void init(size_t &nf, size_t &np) = 0;
            virtual void readFrame(float* pos, size_t np) = 0;
            virtual bool hasNextFrame(size_t &id) = 0;
        };

        class AsciiHelper:
            public IHelper
        {
        public:
            AsciiHelper(std::ifstream& input) :file(input){}

            void init(size_t &nf, size_t &np);
            void readFrame(float* pos, size_t np);
            bool hasNextFrame(size_t &id);

        private:
            std::ifstream& file;
        };

        class BinaryHelper:
            public IHelper
        {
        public:
            BinaryHelper(std::ifstream& input) :file(input){}

            void init(size_t &nf, size_t &np);
            void readFrame(float* pos, size_t np);
            bool hasNextFrame(size_t &id);

        private:
            std::ifstream& file;
        };

    public:
        CacheHair(){}
        ~CacheHair();

        bool loadFile(const char* fileName, bool binary = true);
        void rewind();
        void nextFrame() { bNextFrame = true; }
        size_t getFrameNumber() const;

        virtual size_t n_strands() const;
        virtual const float* get_visible_particle_position(size_t i, size_t j) const;
        virtual void onFrame(Mat3 world, float fTime, float fTimeElapsed, void* = nullptr);

    private:
        void readFrame();
        bool hasNextFrame();

        std::streampos firstFrame = 0;
        std::ifstream file;
        bool bNextFrame = false;
        float* position = nullptr;

        IHelper* helper = nullptr;
        float timeBuffer = 0.f;
    };
}