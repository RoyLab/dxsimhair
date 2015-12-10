#pragma once
#include "wrStrand.h"

class wrHair
{
public:
    wrHair(int);
    ~wrHair();
    
    wrStrand* getStrands() { return strands; }
    wrStrand& getStrand(int idx) { return strands[idx]; }
private:
	wrStrand* strands;
};