#include "DXUT.h"
#include "wrHair.h"

wrHair::wrHair(int nStrand)
{
    assert(nStrand > 0);
    strands = new wrStrand[nStrand];
}


wrHair::~wrHair()
{
    SAFE_DELETE_ARRAY(strands);
}
