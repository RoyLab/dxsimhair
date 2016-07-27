#pragma once
#include "wrStrand.h"
#include <Eigen\Dense>

struct wrStrand;
 
class wrTetrahedron
{
public:
    void init(wrStrand*, unsigned idx);
    void applySpring(Eigen::MatrixXf& C);

private:
    wrParticle *particles[4];

    float l_fp[4];
    float l_ee[3];

    bool bIsCoplanar = false;
    bool bIsNegativeVolumn = false;
    int idx = -1;
};