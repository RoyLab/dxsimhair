#include "precompiled.h"
#include "wrHairLoader.h"
#include <fstream>
#include <iostream>
#include <string>
#include "wrHairSimulator.h"
#include "wrMath.h"
#include "Parameter.h"

wrHair* wrHairLoader::loadFile(wchar_t* path)
{
    std::ifstream file(path, std::ios::binary);
	if (file)
	{
        wrHair *hair = nullptr;
		char cbuffer[sizeof(float)*3*N_PARTICLES_PER_STRAND];

        file.read(cbuffer, sizeof(int));
		int n_particles = *reinterpret_cast<int*>(cbuffer);

		file.seekg(3 * n_particles * sizeof(float) + sizeof(int));
        file.read(cbuffer, sizeof(int));
        int n_strands = *reinterpret_cast<int*>(cbuffer);

#ifdef COMPRESS
        n_strands /= COMPRESS_RATIO;
        n_particles /= COMPRESS_RATIO;
#endif

        wprintf(L"Loading %s, total particles: %d, total strands: %d\n", path, n_particles, n_strands);

        file.seekg(sizeof(int));
        hair = new wrHair(n_strands);
        for (int i = 0; i < n_strands; i++)
        {
            file.read(cbuffer, sizeof(float) * 3 * N_PARTICLES_PER_STRAND);
            if (file.eof())
            {
                WR_LOG_FATAL << "unexpected eof flag";
                SAFE_DELETE(hair);
                break;
            }
            hair->getStrand(i).init(reinterpret_cast<float*>(cbuffer));

#ifdef COMPRESS
            file.seekg(sizeof(int) + (COMPRESS_RATIO * i + static_cast<int>(COMPRESS_RATIO * randf())) * sizeof(float) * 3 * N_PARTICLES_PER_STRAND);
#endif
        }

		file.close();
        return hair;
    }
    else return nullptr;

}