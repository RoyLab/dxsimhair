#pragma once


#ifndef N_PARTICLES_PER_STRAND
#define N_PARTICLES_PER_STRAND      25
#endif

extern float K_SPRINGS[4];
extern float PARTICLE_MASS;  // kg
extern float DAMPING_COEF;
extern float WIND_DAMPING_COEF;

extern float         MAX_TIME_STEP;
extern int           MAX_PASS_NUMBER;

extern float          GRAVITY[3];


#define COMPRESS

extern int COMPRESS_RATIO;
extern float K_ALTITUDE_SPRING;

void init_global_param();
