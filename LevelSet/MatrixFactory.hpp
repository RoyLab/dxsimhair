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

	template<class Container>
	MatrixFactory<Container>::MatrixFactory(const char* fgroup)
	{
		std::ifstream file(fgroup, std::ios::binary);
		int ngi;
		Read4Bytes(file, ngi);

		int *gid = new int[ngi];
		ReadNBytes(file, gid, ngi * sizeof(4));
		file.close();
	}

	template<class Container>
	MatrixFactory<Container>::~MatrixFactory()
	{
	}

	template<class Container>
	void MatrixFactory<Container>::update(Container & id0, Container & id1)
	{
		if (!isInit())
		{
			id0_.swap(id0);
			id1_.swap(id1);
			bInit = true;

		}


	}



} /// Hair
} /// XRwy
