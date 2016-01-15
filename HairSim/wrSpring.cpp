#include "wrSpring.h"
#include "wrHair.h"
#include <Eigen\Dense>

using namespace Eigen;


namespace WR
{
	void BiSpring::applyForces(Eigen::SparseMatrix<float>& matK, Eigen::SparseMatrix<float>& matB, VecX& Const) const
	{
		//vec3 &p0 = nodes[0]->;
		//vec3 &p1 = nodes[1]->position;
		//Vector3f d(p0[0] - p1[0], p0[1] - p1[1], p0[2] - p1[2]);

		//Matrix3f block = d * d.transpose();

		//matK
	}

	void BiSpring::setSpring(int type, const Particle* p0, const Particle* p1, float K)
	{
		type = stride;

		nodes[0] = p0;
		nodes[1] = p1;

		setCoef(K, (nodes[0]->get_ref()- nodes[1]->get_ref()).norm());
	}

	void BiSpring::setCoef(float k, float l)
	{
		_K = k;
		L0 = l;
		KdivL0 = k / l;
	}

}
