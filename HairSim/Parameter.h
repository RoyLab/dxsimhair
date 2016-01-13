#pragma once


const float K_SPRINGS[4] = { 0.0f /*null*/, 2.0e-3f, 20.0e-5f, 10.0e-5f };
const float PARTICLE_MASS = 5.0e-7f;  // kg
const float DAMPING_COEF = 1.0e-5f;
const float WIND_DAMPING_COEF = 1.e-5f;


#define COMPRESS

#ifdef COMPRESS
const int COMPRESS_RATIO = 1000;
#endif


const float K_ALTITUDE_SPRING = 0.e-6f;
