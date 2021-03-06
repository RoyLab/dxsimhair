#pragma once
#include "ICollisionEngine.h"

namespace xhair
{
    typedef Point3 Point_3;

	class GridCollisionObject
	{
    public:
        struct LevelSetVData
        {
            float value;
            float grad[3];
        };

	public:
		GridCollisionObject(const ParamDict& param);
		virtual ~GridCollisionObject();

		float query_distance(const Point_3& p) const;
		float query_squared_distance(const Point_3& p) const;
		bool exceed_threshhold(const Point_3& p, float thresh = 0.f) const;
		bool position_correlation(const Point_3& p, Point_3* pCorrect, float thresh = 0.f) const;

	private:
		LevelSetVData* query(int i, int j, int k) const { return data + i*yz + j*z + k; }
		void computeIdx(const Point_3& p, float& i, float& j, float& k) const;

		int x, y, z, yz;
		float bbox[6];
		float diag[3];

		float tolerance, step, correctionRate, maxstep;
		LevelSetVData* data = nullptr;
	};

    class CGridCollisionEngine :
        public ICollisionEngine
    {
    public:
        CGridCollisionEngine(const ParamDict& param);
        virtual ~CGridCollisionEngine();

        void filter(HairGeometry* hair);
    private:
        GridCollisionObject* collision_obj = nullptr;
    };
}

