#pragma once
#include "wrStrand.h"

class wrHairSimulator;

class wrHair
{
    friend class wrHairSimulator;
public:
    wrHair(int);
    virtual ~wrHair();
    
    wrStrand* getStrands() { return strands; }
    wrStrand& getStrand(int idx) { return strands[idx]; }
    const wrStrand& getStrand(int idx) const { return strands[idx]; }
    int n_strands() const { return nStrands; }

    void updateReference();
    bool initSimulation();

protected:
	wrStrand* strands;
    int       nStrands;
};

class wrHairTransformer
{
public:
    static void scale(wrHair&, float);
    static void mirror(wrHair&, bool, bool, bool);
};