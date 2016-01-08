#pragma once
#include "wrHair.h"

class wrLevelsetOctree;

class wrHairSimulator
{
public:
    wrHairSimulator();
    ~wrHairSimulator();
    
    bool init(wrHair* hair);
    void onFrame(wrHair* hair, const DirectX::XMMATRIX& mWorld, float fTime, float fTimeElapsed);

protected:
    void step(wrHair* hair, DirectX::XMMATRIX& mWorld, float fTime, float fTimeElapsed);

	wrLevelsetOctree*			pLVTree = nullptr;
};

