#include "wrSpring.h"
#include <Eigen\Dense>
#include "wrHair.h"

using namespace Eigen;


namespace WR
{
	void BiSpring::applyForces(Eigen::SparseMatrix<float>& matK, Eigen::SparseMatrix<float>& matB, Eigen::SparseMatrix<float>& Const) const
	{
		//vec3 &p0 = nodes[0]->;
		//vec3 &p1 = nodes[1]->position;
		//Vector3f d(p0[0] - p1[0], p0[1] - p1[1], p0[2] - p1[2]);

		//Matrix3f block = d * d.transpose();

		//matK
	}

	void BiSpring::setSpring(int type, const Particle* p0, const Particle* p1, float K)
	{
		//type = stride;

		//nodes[0] = p0;
		//nodes[1] = p1;

		//setCoef(K, computeDistance(nodes[0]->position, nodes[1]->position));
	}

	void BiSpring::setCoef(float k, float l)
	{
		_K = k;
		L0 = l;
		KdivL0 = k / l;
	}

}
