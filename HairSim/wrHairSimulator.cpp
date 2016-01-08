#include "precompiled.h"
#include "wrHairSimulator.h"
#include "wrLogger.h"
#include <Eigen\Dense>
#include "wrLevelsetOctree.h"

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

	HRESULT hr;

	Polyhedron_3 *P = WRG::readFile<Polyhedron_3>("../../models/head.off");
	pLVTree = new wrLevelsetOctree;
	V_RETURN(pLVTree->construct(*P, 4));
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


void wrHairSimulator::step(wrHair* hair, XMMATRIX& mWorld, float fTime, float fTimeElapsed)
{
    auto pStrands = hair->getStrands();
    int n_strands = hair->n_strands();

    mat4x4 mWorld2, mInvWorld;
    XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&mWorld2), mWorld);
	auto invMat = DirectX::XMMatrixInverse(nullptr, mWorld);
	XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&mInvWorld), invMat);

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

		for (size_t i = 0; i < nDim; i++)
		{
			vec3 p{ Xnp1(nDim * i, 0), Xnp1(nDim * i + 1, 0), Xnp1(nDim * i + 2, 0) }, p0;
			mat4x4_mul_vec3(p0, mInvWorld, p);

			auto dist = pLVTree->queryDistance(Point_3(p0[0], p0[1], p0[2]));
			if (dist < 0)
			{
				Vector_3 grad;
				pLVTree->queryGradient(Point_3(p0[0], p0[1], p0[2]), grad);
				Vector_3 delta = (dist / grad.squared_length()) * grad;
				p0[0] += delta.x();
				p0[1] += delta.y();
				p0[2] += delta.z();
				mat4x4_mul_vec3(p, mWorld2, p0);
				for (size_t j = 0; j < 3; j++)
					Xnp1(nDim* i + j, 0) = p[i];
			}
		}

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
