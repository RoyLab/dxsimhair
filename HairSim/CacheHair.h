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

    protected:
        class IHelper
        {
        public:
            virtual void init(size_t &nf, size_t &np) = 0;
            virtual void readFrame(float* pos, size_t np) = 0;
            virtual void readFrame20(float* rigidTrans, float* pos, float* dir, size_t np){ assert(0); }
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
            void readFrame20(float* rigidTrans, float* pos, float* dir, size_t np);
            bool hasNextFrame(size_t &id);

        private:
            std::ifstream& file;
        };

    public:
        CacheHair(){}
        virtual ~CacheHair();

        bool loadFile(const char* fileName, bool binary = true);
        void rewind();
        void nextFrame() { bNextFrame = true; }
        size_t getFrameNumber() const;

        virtual size_t n_strands() const;
        virtual const float* get_visible_particle_position(size_t i, size_t j) const;
        virtual void onFrame(Mat3 world, float fTime, float fTimeElapsed, void* = nullptr);

    protected:
        virtual void readFrame();
        bool hasNextFrame();

        std::streampos firstFrame = 0;
        std::ifstream file;
        bool bNextFrame = false;
        float* position = nullptr;

        IHelper* helper = nullptr;
        float timeBuffer = 0.f;
    };

    class CacheHair20 :
        public CacheHair
    {
    public:
        ~CacheHair20();

        bool loadFile(const char* fileName, bool binary = true);
        const float* get_visible_particle_direction(size_t i, size_t j) const;
        const float* get_rigidMotionMatrix() const;

    protected:
        void readFrame();

        float* direction = nullptr;
        float* rigidTrans = nullptr;
    };
}