#include <fstream>
#include "GridCollisionObject.h"
#include "macros.h"
#include "wrLogger.h"

namespace XRwy
{
	GridCollisionObject::GridCollisionObject(const char* fileName)
	{
		std::ifstream gridFile(fileName, std::ios::binary);
		Read4Bytes(gridFile, x);
		Read4Bytes(gridFile, y);
		Read4Bytes(gridFile, z);
		gridFile.close();
	}


	GridCollisionObject::~GridCollisionObject()
	{
	}


	float GridCollisionObject::query_distance(const Point_3& p) const
	{
		return 0.0;
	}

	float GridCollisionObject::query_squared_distance(const Point_3& p) const
	{
		return 0.0;
	}

	bool GridCollisionObject::exceed_threshhold(const Point_3& p, float thresh) const
	{
		UNIMPLEMENTED_DECLARATION;
		return false;
	}

	bool GridCollisionObject::position_correlation(const Point_3& p, Point_3* pCorrect, float thresh) const
	{
		return false;
	}
}


