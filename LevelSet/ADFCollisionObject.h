#pragma once
#include <CGAL\Delaunay_Triangulation_3.h>
#include <CGAL\Triangulation_vertex_base_with_info_3.h>
#include <CGAL\Iso_cuboid_3.h>

#include "ICollisionObject.h"
#include "macros.h"
#include "ADFOctree.h"
#include <string.h>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace WR
{
	class ADFCollisionObject :
        public ICollisionObject
    {
        typedef CGAL::Iso_cuboid_3<K> BoundingBox;

        COMMON_PROPERTY(BoundingBox, bbox);
        COMMON_PROPERTY(size_t, max_level);
        COMMON_PROPERTY(float, max_step);

        friend class ADFOctree;
        typedef float(ADFCollisionObject::*ExtrapolateFunc)(const Point_3& p, const Point_3 v[], size_t infId, Dt::Cell_handle ch) const;

    public:
        ADFCollisionObject(Dt* stt, const BoundingBox& box, size_t lvl, float sz, Polyhedron_3_FaceWithId* m) :
            pDt(stt), m_bbox(box), m_max_level(lvl), m_max_step(sz * 0.95f), pModel(m){
            compute_gradient();
        }
        ADFCollisionObject(const wchar_t*);
        ~ADFCollisionObject() { release(); }

        virtual float query_distance(const Point_3& p) const { return query_distance_template(p, &ADFCollisionObject::no_extrapolate); }
        virtual float query_squared_distance(const Point_3& p) const;
        virtual bool exceed_threshhold(const Point_3& p, float thresh = 0.f) const;
        virtual bool position_correlation(const Point_3& p, Point_3* pCorrect, float thresh = 0.f) const;

        bool save_model(const wchar_t*) const;
        bool load_model(const wchar_t*);

        void compute_gradient();

    private:
        // must be in finite cell, near hint
        bool position_correlation_iteration(const Point_3& p, Point_3& newPos, Dt::Cell_handle chnew, Dt::Cell_handle chhint, float thresh) const;
        void correct_position_by_gradient(const Point_3& p, Point_3& newPos, Point_3* pts, Vector_3* grads, float cur_value, float thresh) const;
        float query_distance_with_extrapolation(const Point_3& p) const { return query_distance_template(p, &ADFCollisionObject::extrapolate); }
		float query_distance_with_fake_extrapolation(const Point_3& p) const { return query_distance_template(p, &ADFCollisionObject::fake_extrapolate); }
		float query_distance_cgal(const Point_3& p) const { return query_distance_template(p, &ADFCollisionObject::cgal_distance); }
        float query_distance_template(const Point_3& p, ExtrapolateFunc func) const;

        float no_extrapolate(const Point_3& p, const Point_3 v[], size_t infId, Dt::Cell_handle ch) const { return std::numeric_limits<float>::max(); }
        float extrapolate(const Point_3& p, const Point_3 v[], size_t infId, Dt::Cell_handle ch) const;
		float fake_extrapolate(const Point_3& p, const Point_3 v[], size_t infId, Dt::Cell_handle ch) const;
		float cgal_distance(const Point_3& p, const Point_3 v[], size_t infId, Dt::Cell_handle ch) const;

        void release();

        Dt* pDt = nullptr;
		Polyhedron_3_FaceWithId *pModel = nullptr;
    };
}
