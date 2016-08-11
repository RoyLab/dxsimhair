#pragma once
#include "ICollisionObject.h"

namespace XRwy
{
	class GridCollisionObject :
		public WR::ICollisionObject
	{
	public:
		GridCollisionObject(const char*);
		virtual ~GridCollisionObject();

		float query_distance(const Point_3& p) const;
		float query_squared_distance(const Point_3& p) const;
		bool exceed_threshhold(const Point_3& p, float thresh = 0.f) const;
		bool position_correlation(const Point_3& p, Point_3* pCorrect, float thresh = 0.f) const;

	private:
		int x, y, z;
	};
}

