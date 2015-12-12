#pragma once
#include "linmath.h"

#define N_PARTICLES_PER_STRAND 25
#define N_SPRING_USED 1

struct wrParticle
{
	vec3        position;
	vec3        velocity, v_middle;
	vec3        force;
	float       mass_1;

    float       springLens[N_SPRING_USED];
    vec3        diffs[N_SPRING_USED];
    wrParticle* siblings[N_SPRING_USED];
};

class wrStrand
{
public:
    wrStrand(){}
    ~wrStrand(){}

	bool init(float*);
	void release();
	void onFrame();

    const wrParticle* getParticles() const { return particles; }
    wrParticle* getParticles() { return particles; }

private:
	wrParticle particles[N_PARTICLES_PER_STRAND];                                                                                                 

};



