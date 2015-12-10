#pragma once
#include "linmath.h"

#define N_PARTICLES 25

struct wrParticle
{
	vec3 position;
	vec3 velocity, v_middle;
	vec3 force;
	float mass;
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

private:
	wrParticle particles[N_PARTICLES];

};



