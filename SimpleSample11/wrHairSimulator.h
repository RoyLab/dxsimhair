#pragma once
#include "wrHair.h"

class wrHairSimulator
{
public:
    wrHairSimulator();
    ~wrHairSimulator();
    
    bool init(wrHair* hair);
    void onFrame(wrHair* hair, float fTime, float fTimeElapsed);

protected:
    void step(wrHair* hair, float fTime, float fTimeElapsed);
};

