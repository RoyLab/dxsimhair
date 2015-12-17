#include "precompiled.h"
#include "wrStrand.h"

#define PARTICLE_MASS 5.0e-7f  // kg

bool wrStrand::init(float* positions)
{
    memset(particles, 0, sizeof(wrStrand));
    for (int i = 0; i < N_PARTICLES_PER_STRAND; i++)
    {
        particles[i].mass_1 = 1.f / PARTICLE_MASS;
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