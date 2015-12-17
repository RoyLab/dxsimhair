#pragma once
#include "linmath.h"

#define N_PARTICLES_PER_STRAND 25
#define N_SPRING_USED 1

//#define NUMERICAL_TRACE

struct wrParticle
{
	vec3        position, pos_middle;
	vec3        velocity, v_middle;
	vec3        force;
	float       mass_1;

    float       springLens[N_SPRING_USED];
    vec3        diffs[N_SPRING_USED];
    wrParticle* siblings[N_SPRING_USED];

#ifdef NUMERICAL_TRACE
    float       cLen;
    vec3       acc1, acc2;
#endif
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



