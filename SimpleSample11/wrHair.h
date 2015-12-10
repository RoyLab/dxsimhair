#pragma once
#include "wrStrand.h"

class wrHair
{
public:
    wrHair(int);
    ~wrHair();
    
    wrStrand* getStrands() { return strands; }
    wrStrand& getStrand(int idx) { return strands[idx]; }
    const wrStrand& getStrand(int idx) const { return strands[idx]; }

    int n_strand() const { return nStrands; }

private:
	wrStrand* strands;
    int       nStrands;
};

class wrHairTransformer
{
public:
    static void scale(wrHair&, float);
    static void mirror(wrHair&, bool, bool, bool);
};