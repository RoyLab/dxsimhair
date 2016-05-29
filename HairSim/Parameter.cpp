#include "Parameter.h"
#include "ConfigReader.h"
#include <cstring>

float K_SPRINGS[4] = { 0.0f /*null*/, 1.0e-3f, 20.0e-5f, 10.0e-5f };
float PARTICLE_MASS = 5.0e-7f;  // kg
float DAMPING_COEF = 1.0e-5f;
float WIND_DAMPING_COEF = 1.e-5f;

float         MAX_TIME_STEP = 1.0e-3f;
int           MAX_PASS_NUMBER = 1;

float          GRAVITY[3] = { 0.0f, -10.0f, 0.0f };

#ifdef COMPRESS

#ifdef _DEBUG
int COMPRESS_RATIO = 500;
#else
int COMPRESS_RATIO = 1000;
#endif

#endif

float K_ALTITUDE_SPRING = 0.e-6f;
bool APPLY_COLLISION = false;
bool APPLY_STRAINLIMIT = false;
bool APPLY_PCG = false;
std::string CACHE_FILE;
std::string GUIDE_FILE;
std::string GROUP_FILE;
std::string REF_FILE;
bool hasShadow = false;


void init_global_param()
{
    ConfigReader reader("../config.ini");
    MAX_TIME_STEP = std::stof(reader.getValue("timestep"));
    WIND_DAMPING_COEF = std::stof(reader.getValue("winddamping"));
    DAMPING_COEF = std::stof(reader.getValue("springdamping"));
    K_SPRINGS[1] = std::stof(reader.getValue("spring1"));
    K_SPRINGS[2] = std::stof(reader.getValue("spring2"));
    K_SPRINGS[3] = std::stof(reader.getValue("spring3"));
    GRAVITY[1] = -std::stof(reader.getValue("gravity"));
    APPLY_COLLISION = std::stoi(reader.getValue("collision"));
    APPLY_STRAINLIMIT = std::stoi(reader.getValue("strainlimit"));
    APPLY_PCG = std::stoi(reader.getValue("pcg"));
    CACHE_FILE = reader.getValue("cachefile");
    GUIDE_FILE = reader.getValue("guidefile");
    GROUP_FILE = reader.getValue("groupfile");
    REF_FILE = reader.getValue("reffile");
    hasShadow = bool(std::stoi(reader.getValue("shadow")));
}