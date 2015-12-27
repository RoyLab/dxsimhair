#include "precompiled.h"
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

bool wrHair::initSimulation()
{
    bool hr;
    for (int i = 0; i < nStrands; i++)
        V_RETURN(strands[i].initSimulation());

    return true;
}


void wrHair::updateReference()
{
    for (int i = 0; i < nStrands; i++)
        strands[i].updateReference();
}


void wrHairTransformer::scale(wrHair& hair, float scale)
{
    int n_strands = hair.n_strands();
    for (int i = 0; i < n_strands; i++)
    {
        auto particles = hair.getStrand(i).particles;
        int np = hair.getStrand(i).nParticles;
        for (int j = 0; j < np; j++)
            vec3_scale(particles[j].position, particles[j].position, scale);

        auto springs = hair.getStrand(i).springs;
        int ns = hair.getStrand(i).nSprings;
        for (int j = 0; j < ns; j++)
            springs[j].coef /= scale;
    }

    hair.updateReference();
}

void wrHairTransformer::mirror(wrHair& hair, bool x, bool y, bool z)
{
    int n_strands = hair.n_strands();

    int c[3] = { (x) ? -1 : 1, (y) ? -1 : 1, (z) ? -1 : 1 };

    for (int i = 0; i < n_strands; i++)
    {
        auto particles = hair.getStrand(i).particles;
        int np = hair.getStrand(i).nParticles;
        for (int j = 0; j < np; j++)
            for (int k = 0; k < 3; k++)
                particles[j].position[k] *= c[k];
    }

    hair.updateReference();
}
