#include "precompiled.h"
#include "Anim2Loader.h"

namespace xhair
{
    Anim2Loader::Anim2Loader(const char* fileName, HairGeometry * geom)
    {
        file = std::ifstream(fileName, std::ios::binary);
        if (!file.is_open()) throw std::exception("file not found!");

        char bytes[4];
        file.read(bytes, 4);
        m_nFrame = *reinterpret_cast<int*>(bytes);

        file.read(bytes, 4);
        nparticle = *reinterpret_cast<int*>(bytes);

        firstFrame = file.tellg();

        geom->nParticle = nparticle;
        allocateHair(geom);

        // read the first frame
        command = Next;
        filter(geom);
    }

    void Anim2Loader::filter(HairGeometry * hair)
    {
        assert(hair->nParticle == nparticle);
        if (command == Next)
        {
            if (!hasNextFrame(&m_nFrame))
            {
                file.clear();
                jumpTo(hair, 0);
            }
            else
            {
                readFrame(hair);
            }
        }
        else
        {
            int jumpNo = command - Jump;
            command = Next; // default behavior of filter function

            set_curFrame(jumpNo);
            file.seekg(firstFrame + std::streamoff(get_curFrame()*(sizeof(int) +
                sizeof(float)*(16 + 3 * 2 * nparticle))));

            if (hasNextFrame(&m_nFrame))
                readFrame(hair);
            else
            {
                file.clear();
                jumpTo(hair, 0);
            }
        }
    }

    bool Anim2Loader::hasNextFrame(int *id)
    {
        char bytes[4];
        file.read(bytes, 4);

        if (file.eof())
            return false;

        *id = *reinterpret_cast<int*>(bytes);
        return true;
    }

    void Anim2Loader::readFrame(HairGeometry* hair)
    {
        char* bytes = reinterpret_cast<char*>(&hair->rigidTrans);
        file.read(bytes, sizeof(float) * 16);

        bytes = reinterpret_cast<char*>(&hair->position);
        file.read(bytes, sizeof(float)*nparticle * 3);

        bytes = reinterpret_cast<char*>(&hair->direction);
        file.read(bytes, sizeof(float)*nparticle * 3);
    }
}