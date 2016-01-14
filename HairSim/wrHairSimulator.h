#pragma once
#include <Eigen\SparseCore>

class wrLevelsetOctree;
class wrHair;
class wrStrand;

class wrHairSimulator
{
public:
	typedef Eigen::SparseMatrix<float> SM;
public:
    wrHairSimulator();
    ~wrHairSimulator();
    
    bool init(wrHair* hair);
    void onFrame(wrHair* hair, const DirectX::XMMATRIX& mWorld, float fTime, float fTimeElapsed);

protected:
	void applyMatrixToStrand(const wrStrand* strand, SM& matK, SM& matB, SM& vecC) const;
    void step(wrHair* hair, DirectX::XMMATRIX& mWorld, float fTime, float fTimeElapsed);

    wrLevelsetOctree*			pLSTree = nullptr;
};

