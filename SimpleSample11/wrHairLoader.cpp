#include "DXUT.h"
#include "wrHairLoader.h"
#include <fstream>
#include <boost\log\trivial.hpp>
#include <iostream>
#include <string>

wrHair* wrHairLoader::loadFile(wchar_t* path)
{
    std::ifstream file(path, std::ios::binary);
	if (file)
	{
        wrHair *hair = nullptr;
		char cbuffer[sizeof(float)*3*N_PARTICLES];
        wchar_t info[100];

        file.read(cbuffer, sizeof(int));
		int n_particles = *reinterpret_cast<int*>(cbuffer);

		file.seekg(3 * n_particles * sizeof(float) + sizeof(int));
        file.read(cbuffer, sizeof(int));
        int n_strands = *reinterpret_cast<int*>(cbuffer);

        wprintf(L"Loading %s, total particles: %d, total strands: %d\n", path, n_particles, n_strands);

        file.seekg(sizeof(int));
        hair = new wrHair(n_strands);
        for (int i = 0; i < n_strands; i++)
        {
            file.read(cbuffer, sizeof(float) * 3 * N_PARTICLES);
            if (file.eof())
            {
                BOOST_LOG_TRIVIAL(fatal) << "unexpected eof flag";
                SAFE_DELETE(hair);
                break;
            }
            hair->getStrand(i).init(reinterpret_cast<float*>(cbuffer));
        }

		file.close();
        return hair;
    }
    else return nullptr;

}