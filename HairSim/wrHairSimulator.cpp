#include "precompiled.h"
#include "wrHairSimulator.h"
#include "wrLogger.h"
#include <Eigen\Dense>

#include <fstream>
#include <iomanip>

using namespace Eigen;
using namespace DirectX;

namespace
{
    const float         MAX_TIME_STEP =             30.0e-3f;
    const int           MAX_PASS_NUMBER =           1;

    const vec3          GRAVITY =                   { 0.0f, -10.0f, 0.0f };

    const int           N_STRAND_MATRIX_DIM =       3 * N_PARTICLES_PER_STRAND;
    wrHair*             g_pHair;

    typedef Matrix<float, N_STRAND_MATRIX_DIM, N_STRAND_MATRIX_DIM>     MatrixStrand;
    typedef Matrix<float, N_STRAND_MATRIX_DIM, 1>                       VectorStrand;

    template<typename _Scalar, int _Rows, int _Cols, int _Options, int _MaxRows, int _MaxCols>
    void write(char* fileName, Matrix<_Scalar, _Rows, _Cols, _Options, _MaxRows, _MaxCols>& mat, int width)
    {
        std::ofstream f(fileName);
        f.precision(2);
        for (int i = 0; i < mat.cols(); i++)
            f << std::setw(width) << i + 2 << " ";
        f << std::endl;
        f << mat;
        f.close();
    }
}


wrHairSimulator::wrHairSimulator()
{
}


wrHairSimulator::~wrHairSimulator()
{
}


bool wrHairSimulator::init(wrHair* hair)
{
    hair->initSimulation();
    return true;
}


void wrHairSimulator::onFrame(wrHair* hair, const XMMATRIX& mWorld, float fTime, float fTimeElapsed)
{
    static XMMATRIX lastmWorld = XMMatrixIdentity();

    float tStep = fTimeElapsed;
    int nPass = 1;
    if (fTimeElapsed > MAX_TIME_STEP)
    {
        nPass = static_cast<int>(fTimeElapsed / MAX_TIME_STEP) + 1;
        tStep = fTimeElapsed / static_cast<float>(nPass);
    }

    if (nPass > MAX_PASS_NUMBER) nPass = MAX_PASS_NUMBER;

    float start = fTime - fTimeElapsed;
    auto matStep = (mWorld - lastmWorld) / nPass;
    for (int i = 0; i < nPass; i++)
    {
        lastmWorld += matStep;
        step(hair, lastmWorld, (start += tStep), tStep);
        //Sleep(500);
    }

    //WR_LOG_DEBUG << " TIME: " << fTime << " e: " << fTimeElapsed << " nPass: " << nPass << std::endl;
}


void wrHairSimulator::step(wrHair* hair, XMMATRIX& mWorld, float fTime, float fTimeElapsed)
{
    auto pStrands = hair->getStrands();
    int n_strands = hair->n_strands();

    mat4x4 mWorld2;
    XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&mWorld2), mWorld);

    for (int i = 0; i < n_strands; i++)
    {
        auto &strand = pStrands[i];

        // move the fixed vertices
        for (int j = 0; j < 3; j++)
            mat4x4_mul_vec3(strand.particles[j].position, mWorld2, strand.particles[j].reference);
        
        MatrixXf K, B, C;
        size_t nDim = strand.computeMatrices(K, B, C);

        MatrixXf M_1 = MatrixXf::Identity(nDim, nDim);
        M_1 *= strand.particles[5].mass_1;

        MatrixXf Xn, Vn;
        strand.assignVectors(Xn, Vn);

        MatrixXf tmp, Vnp1, DIV;
        tmp = Vn + fTimeElapsed * (M_1 * (-K * Xn - C));
        DIV = MatrixXf::Identity(nDim, nDim) + fTimeElapsed / 2.0f * M_1 * B +
            fTimeElapsed * fTimeElapsed * M_1 * K;
        Vnp1 = DIV.inverse() * tmp;
        
        // compute Xn
        MatrixXf Xnp1;
        Xnp1 = Xn + fTimeElapsed * Vnp1;

        strand.assignBackVectors(Xnp1, Vnp1);

    } // total hair

#ifdef NUMERICAL_TRACE
    auto &sampler = hair->getStrand(3).getParticles()[1];
    auto &sampler2 = hair->getStrand(3).getParticles()[0];
    WR_LOG_TRACE << std::endl
        << "pos: " << sampler.position[0] << " " << sampler.position[1] << " " << sampler.position[2] << std::endl
        << "pos0: " << sampler2.position[0] << " " << sampler2.position[1] << " " << sampler2.position[2] << std::endl
        << "force: " << sampler.force[0] << " " << sampler.force[1] << " " << sampler.force[2] << std::endl
        << "vel: " << sampler.velocity[0] << " " << sampler.velocity[1] << " " << sampler.velocity[2] << std::endl
        << "len1: " << sampler2.cLen << " " << sampler2.springLens[0] << std::endl
        << "len2: " << sampler.cLen << " " << sampler.springLens[0] << std::endl
        << "acc1: " << sampler.acc1[0] << " " << sampler.acc1[1] << " " << sampler.acc1[2] << std::endl
        << "acc2: " << sampler.acc2[0] << " " << sampler.acc2[1] << " " << sampler.acc2[2] << std::endl;

#endif
}
