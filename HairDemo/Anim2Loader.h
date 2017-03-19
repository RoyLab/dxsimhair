#pragma once
#include <fstream>

#include "xhair.h"

namespace xhair
{

    class Anim2Loader : public IHairLoader
    {
    public:
        Anim2Loader(const char* fileName, HairGeometry * geom);
        ~Anim2Loader() { close(); }

        void filter(HairGeometry* hair);
        void setFilter(int param) { command = param; }
        void close() { file.close(); }

    private:
        void readFrame(HairGeometry* hair);
        bool hasNextFrame(int *id);

        int nparticle = 0; // for check validation
        int command;

        std::ifstream   file;
        std::streampos  firstFrame = 0;
    };

}