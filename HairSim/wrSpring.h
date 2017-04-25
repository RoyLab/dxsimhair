#pragma once

#include <Eigen\SparseCore>
#include "wrTripleMatrix.h"
#include <vector>

namespace WR
{
	class HairParticle;
	typedef HairParticle Particle;

	class ISpring
	{
	public:
		ISpring() {}
		virtual ~ISpring() {}

		virtual void applyForces(SparseMatAssemble& matK, SparseMatAssemble& matB, VecX& Const) const = 0;
		float K() const { return *_K; };

	protected:
		float *_K;
		float L0;

	};

	class BiSpring :
		public ISpring
	{
	public:
		void applyForces(SparseMatAssemble& matK, SparseMatAssemble& matB, VecX& Const) const;
		void setSpring(int type, const Particle* p0, const Particle* p1, float *K);
		void setCoef(float *k, float l);

		//protected:
		float KdivL0() const {
			return (*_K) / L0;
		}
		const Particle* nodes[2];  // id 0 > id 1
		int                stride;
	};


	class wrEdgeEdgeSpring :
		public ISpring
	{
	public:
		void applyForces(SparseMatAssemble& matK, SparseMatAssemble& matB, VecX& Const) const;

	protected:
		Particle* nodes[4];  // 0 big index, 1 small index
	};

	class wrPointFaceSpring :
		public ISpring
	{
	public:
		void applyForces(SparseMatAssemble& matK, SparseMatAssemble& matB, VecX& Const) const;

	protected:
		Particle* nodes[4];  // 0 big index, 1 small index
	};

}

typedef WR::ISpring wrISpring;
typedef WR::BiSpring wrNormalSpring;
