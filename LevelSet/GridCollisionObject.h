#pragma once
#include "ICollisionObject.h"
#include "UnitTest.h"

namespace XRwy
{
	class GridCollisionObject :
		public WR::ICollisionObject
	{
	public:
		//GridCollisionObject(const char*);
		GridCollisionObject(CGAL::Polyhedron_3<CGAL::FloatKernel> &iMesh, int slice = 64);
		GridCollisionObject(const char* file_path);
		virtual ~GridCollisionObject();

		float query_distance(const Point_3& p) const;
		float query_squared_distance(const Point_3& p) const;
		bool exceed_threshhold(const Point_3& p, float thresh = 0.f) const;
		bool position_correlation(const Point_3& p, Point_3* pCorrect, float thresh = 0.f) const;

		void write_to_file(const char* file_path);

		static GridCollisionObject* load_file(const char *file_path);

	private:
		WR::LevelSetVData* query(int i, int j, int k) const { return data + i*yz + j*z + k; }
		void computeIdx(const Point_3& p, float& i, float& j, float& k) const;

		int x, y, z, yz;
		float bbox[6];
		float diag[3];

		float tolerance , correctionRate, maxstep;
		Vector_3 step;

		WR::LevelSetVData* data = nullptr;
	};
}

