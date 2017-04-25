#include "wrSpring.h"
#include "wrHair.h"
#include <Eigen\Dense>

using namespace Eigen;


namespace WR
{
	void BiSpring::applyForces(MatX& mK, MatX& mB, VecX& vC) const
	{
		Vec3 d = (nodes[0]->get_pos() - nodes[1]->get_pos());
		d.normalize();
		Mat3 d3x3 = d * d.transpose();
		Mat3 d3x3K = KdivL0() * d3x3;
		Mat3 d3x3B = DAMPING_COEF * d3x3;
		Vec3 d3C = K() * d;

		//const int id0 = nodes[0]->get_Id();
		//const int id1 = nodes[1]->get_Id();
		const size_t id0 = nodes[0]->get_LocalId() * 3;
		const size_t id1 = nodes[1]->get_LocalId() * 3;

		mK.block<3, 3>(id1, id1) += d3x3K;
		mK.block<3, 3>(id1, id0) += -d3x3K;
		mK.block<3, 3>(id0, id1) += -d3x3K;
		mK.block<3, 3>(id0, id0) += d3x3K;

		mB.block<3, 3>(id1, id1) += d3x3B;
		mB.block<3, 3>(id1, id0) += -d3x3B;
		mB.block<3, 3>(id0, id0) += d3x3B;
		mB.block<3, 3>(id0, id1) += -d3x3B;

		vC.segment<3>(id0) += d3C;
		vC.segment<3>(id1) -= d3C;
		

		//mK.add_triple(id1, id1, d3x3K);
		//mK.add_triple_without_check(id1, id0, -d3x3K);
		//mK.add_triple_without_check(id0, id1, -d3x3K);
		//mK.add_triple(id0, id0, d3x3K);

		//mB.add_triple(id1, id1, d3x3B);
		//mB.add_triple_without_check(id1, id0, -d3x3B);
		//mB.add_triple(id0, id0, d3x3B);
		//mB.add_triple_without_check(id0, id1, -d3x3B);

		//triple(vC, id0) += d3C;
		//triple(vC, id1) -= d3C;
	}

	void BiSpring::setSpring(int type, const Particle* p0, const Particle* p1, float *K)
	{
		type = stride;

		nodes[0] = p0;
		nodes[1] = p1;

		setCoef(K, (nodes[0]->get_ref() - nodes[1]->get_ref()).norm());
	}

	void BiSpring::setCoef(float *k, float l)
	{
		_K = k;
		L0 = l;
	}

}
