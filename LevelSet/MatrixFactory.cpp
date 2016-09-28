#include <fstream>

#include "macros.h"
#include "MatrixFactory.h"

namespace XRwy
{
namespace Hair
{

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


			return 0;
		}
		

	}


}
}