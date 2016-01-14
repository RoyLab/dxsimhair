#pragma once


const float K_SPRINGS[4] = { 0.0f /*null*/, 2.0e-3f, 20.0e-5f, 10.0e-5f };
const float PARTICLE_MASS = 5.0e-7f;  // kg
const float DAMPING_COEF = 1.0e-5f;
const float WIND_DAMPING_COEF = 1.e-5f;

const float         MAX_TIME_STEP = 30.0e-3f;
const int           MAX_PASS_NUMBER = 1;

const vec3          GRAVITY = { 0.0f, -10.0f, 0.0f };

const int           N_STRAND_MATRIX_DIM = 3 * N_PARTICLES_PER_STRAND;


#define COMPRESS

#ifdef COMPRESS
const int COMPRESS_RATIO = 1000;
#endif


const float K_ALTITUDE_SPRING = 0.e-6f;
