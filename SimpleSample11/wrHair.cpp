#include "DXUT.h"
#include "wrHair.h"

wrHair::wrHair(int nStrand)
{
    assert(nStrand > 0);
    this->nStrands = nStrand;
    strands = new wrStrand[nStrand];
}


wrHair::~wrHair()
{
    SAFE_DELETE_ARRAY(strands);
}

void wrHairTransformer::scale(wrHair& hair, float scale)
{
    int n_strands = hair.n_strand();
    for (int i = 0; i < n_strands; i++)
    {
        auto particles = hair.getStrand(i).getParticles();
        for (int j = 0; j < N_PARTICLES_PER_STRAND; j++)
            vec3_scale(particles[j].position, particles[j].position, scale);
    }
}

void wrHairTransformer::mirror(wrHair& hair, bool x, bool y, bool z)
{
    int n_strands = hair.n_strand();

    int c[3] = { (x) ? -1 : 1, (y) ? -1 : 1, (z) ? -1 : 1 };

    for (int i = 0; i < n_strands; i++)
    {
        auto particles = hair.getStrand(i).getParticles();
        for (int j = 0; j < N_PARTICLES_PER_STRAND; j++)
            for (int k = 0; k < 3; k++)
                particles[j].position[k] *= c[k];
    }
}
