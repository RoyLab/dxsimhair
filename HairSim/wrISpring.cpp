#include "wrISpring.h"
#include <Eigen\Dense>
#include "wrStrand.h"

using namespace Eigen;

wrISpring::wrISpring()
{
}


wrISpring::~wrISpring()
{
}


void wrNormalSpring::applyForces(Eigen::SparseMatrix<float>& matK, Eigen::SparseMatrix<float>& matB, Eigen::SparseMatrix<float>& Const) const
{
	vec3 &p0 = nodes[0]->position;
	vec3 &p1 = nodes[1]->position;
	Vector3f d(p0[0] - p1[0], p0[1] - p1[1], p0[2] - p1[2]);

	Matrix3f block = d * d.transpose();

	//matK
}

void wrNormalSpring::setSpring(int type, wrParticle* p0, wrParticle* p1, float K)
{
	type = stride;

	nodes[0] = p0;
	nodes[1] = p1;

	setCoef(K, computeDistance(nodes[0]->position, nodes[1]->position));
}

void wrNormalSpring::setCoef(float k, float l)
{
	_K = k;
	L0 = l;
	KdivL0 = k / l;
}
