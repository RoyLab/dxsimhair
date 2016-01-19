#include "precompiled.h"
#include "wrHairSimulator.h"
#include "wrLogger.h"
#include <Eigen\Dense>
#include <Eigen\SparseCore>
#include "wrLevelsetOctree.h"
#include <CGAL\Aff_transformation_3.h>
#include "wrStrand.h"

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
    SAFE_DELETE(pLSTree);
}


bool wrHairSimulator::init(wrHair* hair)
{
    //hair->initSimulation();

    HRESULT hr;

    Polyhedron_3 *P = WRG::readFile<Polyhedron_3>("../../models/head.off");
    pLSTree = new wrLevelsetOctree;
    V_RETURN(pLSTree->construct(*P, 0));
    SAFE_DELETE(P);

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


void wrHairSimulator::step(wrHair* hair, XMMATRIX& w, float fTime, float fTimeElapsed)
{
 //   auto pStrands = hair->getStrands();
 //   int n_strands = hair->n_strands();

 //   mat4x4 mWorld, mInvWorld;
    //XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&mWorld), w);
    //CGAL::Aff_transformation_3<K> aff(mWorld[0][0], mWorld[0][1], mWorld[0][2], mWorld[0][3],
    //    mWorld[1][0], mWorld[1][1], mWorld[1][2], mWorld[1][3],
    //    mWorld[2][0], mWorld[2][1], mWorld[2][2], mWorld[2][3]);

    //auto invAff = aff.inverse();

    //auto invMat = DirectX::XMMatrixInverse(nullptr, w);
    //XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&mInvWorld), invMat);

    //size_t nMatDim = wrParticle::n_particles() * 3;
    //SparseMatrix<float> K(nMatDim, nMatDim), B(nMatDim, nMatDim), C(nMatDim, 1);
    //vec3 fixedPos[3], fixedVel[3], displace;
 //   for (int i = 0; i < n_strands; i++)
 //   {
 //       auto &strand = pStrands[i];

 //       // move the fixed vertices
    //    // modify velocity
    //    for (int j = 0; j < 3; j++)
    //    {
    //        mat4x4_mul_vec3(fixedPos[j], mWorld, strand.particles[j].reference);
    //        vec3_sub(displace, fixedPos[j], strand.particles[j].position);
    //        vec3_scale(strand.particles[j].velocity, displace, fTimeElapsed);

    //        vec3_copy(strand.particles[j].position, fixedPos[j]);
    //    }



 //   } // total hair

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


//MatrixXf K, B, C;
//      size_t nDim = strand.computeMatrices(K, B, C);

//      MatrixXf M_1 = MatrixXf::Identity(nDim, nDim);
//      M_1 *= strand.particles[5].mass_1;
//
//      MatrixXf Xn, Vn;
//      strand.assignVectors(Xn, Vn);
//
//      MatrixXf tmp, Vnp1, DIV;
//      tmp = Vn + fTimeElapsed * (M_1 * (-K * Xn - C));
//      DIV = MatrixXf::Identity(nDim, nDim) + fTimeElapsed / 2.0f * M_1 * B +
//          fTimeElapsed * fTimeElapsed * M_1 * K;
//      Vnp1 = DIV.inverse() * tmp;
//      
//      // compute Xn
//      MatrixXf Xnp1;
//      Xnp1 = Xn + fTimeElapsed * Vnp1;

//for (size_t i = 0; i < nDim /3; i++)
//{
//    Point_3 p( Xnp1(3 * i, 0), Xnp1(3 * i + 1, 0), Xnp1(3 * i + 2, 0) ), p0;
//    p0 = p.transform(invAff);

//    auto dist = pLSTree->queryInexactDistance(Point_3(p0[0], p0[1], p0[2]));

//          //WR_LOG_TRACE << "point: " << Point_3(p0[0], p0[1], p0[2]);
//          //WR_LOG_TRACE << "distance: " << dist;

//    if (dist < 0)
//    {
//        //Vector_3 grad;
//              auto dir = (p0 - pLSTree->center());
//              auto dire = dir / sqrt(dir.squared_length());
//              p0 = pLSTree->center() + dire * pLSTree->radius();
//              //pLSTree->queryGradient(Point_3(p0[0], p0[1], p0[2]), grad);

//        //Vector_3 delta = (dist / grad.squared_length()) * grad;
//              //p0 = p0 + delta;
//              
//              p = p0.transform(aff);
//              for (size_t j = 0; j < 3; j++)
//              {
//            Xnp1(3* i + j, 0) = p[j];
//                  Vnp1(3 * i + j, 0) = 0;
//              }
//    }
//}

//      strand.assignBackVectors(Xnp1, Vnp1);