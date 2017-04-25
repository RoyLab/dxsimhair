#include "wrConstants.h"

int N_PARTICLES_PER_STRAND = 25;
float K_SPRINGS[4] = { 0.0f /*null*/, 5.0e-5f, 5.0e-9f, 2.5e-5f };
float PARTICLE_MASS = 5.0e-7f;  // kg
float DAMPING_COEF = 1.0e-4f;
float WIND_DAMPING_COEF = 1.e-6f;

float         MAX_TIME_STEP = 0.03f;
int           MAX_PASS_NUMBER = 1;

float          GRAVITY[3] = { 0.0f, -9.8f, 0.0f };

int COMPRESS_RATIO = 1000;

float K_ALTITUDE_SPRING = 0.e-6f;
bool APPLY_COLLISION = false;
bool APPLY_STRAINLIMIT = false;
bool APPLY_PCG = false;