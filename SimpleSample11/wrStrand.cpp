#include "DXUT.h"
#include "wrStrand.h"

#define PARTICLE_MASS 1.0e-6f

bool wrStrand::init(float* positions)
{
    memset(particles, 0, sizeof(wrStrand));
    for (int i = 0; i < N_PARTICLES; i++)
    {
        particles[i].mass = PARTICLE_MASS;
        vec3_copy(particles[i].position, positions + 3 * i);
    }
    return true;
}

void wrStrand::release()
{

}
void wrStrand::onFrame()
{

}