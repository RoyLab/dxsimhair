#pragma once
#include "wrHair.h"

class wrHairLoader
{
public:
    wrHairLoader(){}
    ~wrHairLoader(){}

	wrHair* loadFile(wchar_t*);
};
