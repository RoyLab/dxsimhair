#pragma once
#include "wrMath.h"
#include <Eigen\Dense>
#include "wrSpring.h"

#define N_PARTICLES_PER_STRAND      25
//#define N_SPRING_USED               3

//#define NUMERICAL_TRACE
class wrTetrahedron;
typedef WR::ISpring wrISpring;

struct wrParticle
{
    vec3        position, reference;
    vec3        velocity;
    vec3        force;
    float       mass_1 = 0;

    bool        isVirtual = false; // mark only the swing nodes, NOT the root ones.
    bool            isFixed = false;
    size_t      idx = 0, gId = getGlobalId();

#ifdef NUMERICAL_TRACE
    //vec3        diffs[N_SPRING_USED];
    float       cLen;
    vec3        acc1, acc2;
#endif

    static size_t getGlobalId(){ return gCount++; }
    static size_t n_particles() { return gCount; }

protected:

    static size_t gCount;
};
//
//struct wrSpring
//{
//    float       strength;
//    float       coef;
//    wrParticle* nodes[2];  // 0 big index, 1 small index
//
//    // for dynamically change the spring strength, 1 edge, 2 blend, 3 torsion
//    int         type;
//};

struct wrStrand
{
    // must be in the sequence for strain limit
    wrParticle*     particles;
    size_t          nParticles;

    wrISpring**     springs;
    size_t          nSprings;

    wrParticle*     pRealParticles[N_PARTICLES_PER_STRAND]; // pointers to the real particles

    wrStrand(){}
    ~wrStrand();

    bool init(float*);
    void updateReference();
    size_t assignVectors(Eigen::MatrixXf& vn, Eigen::MatrixXf& xn);
    void assignBackVectors(Eigen::MatrixXf& vn, Eigen::MatrixXf& xn);
    size_t computeMatrices(Eigen::MatrixXf& K, Eigen::MatrixXf& B, Eigen::MatrixXf& C);
    bool initSimulation();

private:
    void pushSprings(wrParticle*, size_t&);
    void pushSingleSpring(wrParticle*, size_t&, int);
    size_t getNumberOfTetras() const { return nParticles - 3; }

    wrTetrahedron* tetras;
};



