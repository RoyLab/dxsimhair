#pragma once
#include <Eigen\SparseCore>

class wrParticle;

class wrISpring
{
public:
	wrISpring();
	~wrISpring();

	virtual void applyForces(Eigen::SparseMatrix<float>& matK, Eigen::SparseMatrix<float>& matB, Eigen::SparseMatrix<float>& Const) const = 0;
	float K() const { return _K; };

protected:
	float		_K;
	float		L0;

};

class wrNormalSpring:
	public wrISpring
{
public:
	void applyForces(Eigen::SparseMatrix<float>& matK, Eigen::SparseMatrix<float>& matB, Eigen::SparseMatrix<float>& Const) const;
	void setSpring(int type, wrParticle* p0, wrParticle* p1, float K);
	void setCoef(float k, float l);

protected:
	float		KdivL0;
	wrParticle* nodes[2];  // id 0 > id 1
	int			stride;
};


class wrEdgeEdgeSpring :
	public wrISpring
{
public:
	void applyForces(Eigen::SparseMatrix<float>& matK, Eigen::SparseMatrix<float>& matB, Eigen::SparseMatrix<float>& Const) const;

protected:
	wrParticle* nodes[4];  // 0 big index, 1 small index
};

class wrPointFaceSpring :
	public wrISpring
{
public:
	void applyForces(Eigen::SparseMatrix<float>& matK, Eigen::SparseMatrix<float>& matB, Eigen::SparseMatrix<float>& Const) const;

protected:
	wrParticle* nodes[4];  // 0 big index, 1 small index
};
