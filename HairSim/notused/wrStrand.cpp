#include "precompiled.h"
#include "wrStrand.h"
#include <vector>
#include <Eigen\Dense>
#include "wrTetrahedron.h"
#include "Parameter.h"
#include "wrSpring.h"

using namespace Eigen;

namespace
{


    const int REAL_SPRING_DICT[][4] = {
        {1, 2, 3, 0},
        {1, 2, 0, 3},
        {1, 3, 2, 0},
        {1, 1, 3, 0},
        {1, 1, 2, 3}
    };

    const int VIRTUAL_SPRING_DICT[][3] = {
        { 1, 2, 3 },
        { 1, 3, 0 }
    };

    int classifyRealParticle(wrParticle* p)
    {
        int code = 0;
        for (int i = 0; i < 3; i++)
            code += ((((p - 1 - i)->isVirtual) ? 1 : 0) << i);

        switch (code)
        {
        case 0: return 0;
        case 1: return 1;
        case 2: return 2 ;
        case 5: return 3;
        case 4: return 4;
        default:
            assert(0);
            return -1;
        }
    }

    int classifyVirtualParticle(wrParticle* p)
    {
        if ((p - 2)->isVirtual) return 0;
        else return 1;
    }

    void genRandParticle(float* r, const float* a, const float* b)
    {
        vec3 diff;
        vec3_sub(diff, b, a);

        vec3 randv = { randSignedFloat(), randSignedFloat(), randSignedFloat() };
        while (isZero(randv) || isCollinear(diff, randv))
        {
            for (int i = 0; i < 3; i++)
                randv[i] = randSignedFloat();
        }

        vec3 perp;
        vec3_mul_cross(perp, diff, randv);
        vec3_norm(perp, perp);

        float l = vec3_len(diff) * 0.8660254f; // sqrt(3)/2
        vec3_scale(r, perp, l);

        vec3 middle;
        vec3_add(middle, a, b);
        vec3_scale(middle, middle, 0.5f);

        vec3_add(r, r, middle);
    }
}

size_t wrParticle::gCount = 0;

wrStrand::~wrStrand()
{
    SAFE_DELETE_ARRAY(particles);
    for (size_t i = 0; i < nSprings; i++)
        SAFE_DELETE(springs[i]);
    SAFE_DELETE_ARRAY(springs);
    SAFE_DELETE_ARRAY(tetras);
}

// init springs and tetrahedrons
bool wrStrand::initSimulation()
{
    nSprings = 3 * (nParticles - 3);
    springs = new wrISpring*[nSprings];
    memset(springs, 0, sizeof(wrISpring*)*nSprings);

    size_t springPtr = 0;
    for (size_t i = 3; i < nParticles; i++)
        pushSprings(particles + i, springPtr);

    const size_t nTetras = getNumberOfTetras();
    tetras = new wrTetrahedron[nTetras];
    for (size_t i = 0; i < nTetras; i++)
        tetras[i].init(this, i);
    return true;
}

bool wrStrand::init(float* positions)
{
    vec3* pos = reinterpret_cast<vec3*>(positions);

    // compute all edge springs length
    vec3 edgeSprings[N_PARTICLES_PER_STRAND - 1];
    for (int i = 0; i < N_PARTICLES_PER_STRAND - 1; i++)
        vec3_sub(edgeSprings[i], pos[i + 1], pos[i]);

    // check for any co-linear condition
    // the index means a virtual node should be added AFTER the (i)th real node
    std::vector<size_t> collinearPos; 
    bool flag = false; // indicate that the next particle is added
    for (int i = 0; i < N_PARTICLES_PER_STRAND - 2; i++)
    {
        // totally n-1 edges, then n-2 comparison
        if (isCollinear(edgeSprings[i], edgeSprings[i + 1]))
        {
            if (!flag) collinearPos.push_back(i);

            collinearPos.push_back(i + 1);
            flag = true;
        }
        else flag = false;
    }

    // apply the memory, two extra virtual particles for the root
    nParticles = N_PARTICLES_PER_STRAND + collinearPos.size() + 2;
    particles = new wrParticle[nParticles];

    // generate 2 virtual (but with [isVirtual] = false) hair root particles
    vec3_sub(particles[0].position, pos[0], edgeSprings[0]);
    genRandParticle(particles[1].position, particles[0].position, pos[0]);
    particles[0].isFixed = true;
    particles[1].isFixed = true;
    particles[2].isFixed = true;

    // copy information into normal particles
    bool isThisVirtual = false;
    for (size_t i = 2, i1 = 0, i2 = 0; i < nParticles; i++)
    {
        particles[i].mass_1 = 1.f / PARTICLE_MASS;
        if (isThisVirtual)
        {
            isThisVirtual = false;
            genRandParticle(particles[i].position, pos[i1], pos[i1 - 1]);
            particles[i].isVirtual = true;
            particles[i].idx = i;

        }
        else
        {
            vec3_copy(particles[i].position, pos[i1]);
            particles[i].isVirtual = false;
            particles[i].idx = i;

            pRealParticles[i1] = particles + i;
            if (collinearPos.size() > i2 && collinearPos[i2] == i1)
            {
                isThisVirtual = true;
                ++i2;
            }
            ++i1;
        }
    }

    // the last particle has a half mass
    particles[nParticles - 1].mass_1 *= 2.f;

    // assign position to reference
    for (size_t i = 0; i < nParticles; i++)
        vec3_copy(particles[i].reference, particles[i].position);

    return true;
}

void wrStrand::pushSprings(wrParticle* p, size_t& sp)
{
    if (p->isVirtual)
    {
        int type = classifyVirtualParticle(p);
        for (int i = 0; i < 3; i++)
            pushSingleSpring(p, sp, VIRTUAL_SPRING_DICT[type][i]);
    }
    else
    {
        int type = classifyRealParticle(p);
        for (int i = 0; i < 4; i++) // TO-DO 可能在第2个粒子上会出问题
            pushSingleSpring(p, sp, REAL_SPRING_DICT[type][i]);
    }

}

void wrStrand::pushSingleSpring(wrParticle* p, size_t& sp, int stride)
{
    if (stride)
    {
        auto &ptr = springs[sp];
        auto spring = new wrNormalSpring;
        //spring->setSpring(stride, p, p - stride, K_SPRINGS[stride]);
        ptr = spring;

        ++sp;
    }
}
size_t wrStrand::assignVectors(MatrixXf& Xn, MatrixXf& Vn)
{
    const int offset = -3;
    const size_t nMovable = nParticles + offset;
    Xn.resize(3 * nMovable, 1);
    Vn.resize(3 * nMovable, 1);
    for (size_t i = 0; i < nMovable; i++)
    {
        for (int j = 0; j < 3; j++)
            Xn(3 * i + j, 0) = particles[i - offset].position[j];
        for (int j = 0; j < 3; j++)
            Vn(3 * i + j, 0) = particles[i - offset].velocity[j];
    }
    return 3 * nMovable;
}

void wrStrand::assignBackVectors(MatrixXf& Xn, MatrixXf& Vn)
{
    const int offset = -3;
    const size_t nMovable = nParticles + offset;
    for (size_t i = 0; i < nMovable; i++)
    {
        for (int j = 0; j < 3; j++)
            particles[i - offset].position[j] = Xn(3 * i + j, 0);
        for (int j = 0; j < 3; j++)
            particles[i - offset].velocity[j] = Vn(3 * i + j, 0);
    }
}

size_t wrStrand::computeMatrices(MatrixXf& K, MatrixXf& B, MatrixXf& C)
{
  //  const int offset = -3;
  //  const size_t nParticles = this->nParticles + offset;
  //  size_t nDim = 3 * nParticles;
  //  K = MatrixXf::Zero(nDim, nDim);
  //  B = MatrixXf::Zero(nDim, nDim);
  //  C = MatrixXf::Zero(nDim, 1);

  //  vec3 de, coefk, coefc, coefb;
  //  for (size_t i = 0; i < nSprings; i++)
  //  {
  //      auto &spring = *reinterpret_cast<wrNormalSpring*>(this->springs[i]);
  //      vec3_sub(de, spring.nodes[0]->position, spring.nodes[1]->position);
  //      vec3_norm(de, de);

  //      size_t a = spring.nodes[0]->idx;
  //      size_t b = spring.nodes[1]->idx;
  //      int ai = 3 * (a + offset);
  //      int bi = 3 * (b + offset);

  //      vec3_scale(coefk, de, spring.KdivL0);
  //      vec3_scale(coefc, de, spring.K());
  //      vec3_scale(coefb, de, DAMPING_COEF);

  //      if ((ai + 1) * (bi + 1) > 0)
  //      {
  //          // row j, column k
  //          for (int j = 0; j < 3; j++)
  //          {
  //              for (int k = 0; k < 3; k++)
  //              {
  //                  float value = coefk[j] * de[k];
  //                  float valueb = coefb[j] * de[k];

  //                  K(ai + j, ai + k) += value;
  //                  K(ai + j, bi + k) += -value;
  //                  K(bi + j, ai + k) += -value;
  //                  K(bi + j, bi + k) += value;

  //                  B(ai + j, ai + k) += valueb;
  //                  B(ai + j, bi + k) += -valueb;
  //                  B(bi + j, ai + k) += -valueb;
  //                  B(bi + j, bi + k) += valueb;
  //              }
  //              C(ai + j, 0) -= coefc[j];
  //              C(bi + j, 0) += coefc[j];
  //          }
  //      }
  //      else // since bi < ai, so bi < 0
  //      {
  //          // row j, column k
  //          for (int j = 0; j < 3; j++)
  //          {
  //              for (int k = 0; k < 3; k++)
  //              {
  //                  float value = coefk[j] * de[k];
  //                  float valueb = coefb[j] * de[k];

  //                  K(ai + j, ai + k) += value;
  //                  B(ai + j, ai + k) += valueb;
  //              }
  //              C(ai + j, 0) -= coefc[j];
  //              C(ai + j, 0) -= vec3_mul_inner(spring.nodes[1]->position, de) * coefk[j];
  //          }
  //      }
  //  }

  //  // for wind damping
  //  for (size_t i = 3; i < nParticles; i++)
  //  {
        //C(3 * (i + offset) + 1, 0) += 10.0 * PARTICLE_MASS;
        //for (int j = 0; j < 3; j++)
        //{
  //          B(3 * (i + offset) + j, 3 * (i + offset) + j) += WIND_DAMPING_COEF;
        //}
  //  }

  //  // add altitude spring
  //  //const auto nTetras = getNumberOfTetras();
  //  //for (size_t i = 0; i < nTetras; i++)
  //  //    tetras[i].applySpring(C);
  //  
  //  return nDim;
    return 0;
}


void wrStrand::updateReference()
{
    for (size_t i = 0; i < nParticles; i++)
        vec3_copy(particles[i].reference, particles[i].position);
}