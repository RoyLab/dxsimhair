#pragma once
#include "EigenTypes.h"

namespace XRwy
{
namespace Hair
{

typedef WR::SparseMat SparseMatrix;

template <class Container>
class MatrixFactory
{
	struct GroupCache
	{
		SparseMatrix L;
		//Eigen::SimplicialLLT<WR::SparseMat, Eigen::Upper> solver;
	};
public:
	MatrixFactory(const char*);
	~MatrixFactory();

	void update(Container& id0, Container& id1);

private:
	bool isInit() const { return bInit; }

	bool bInit = false;
	Container id0_, id1_;
};




} /// Hair
} /// XRwy
