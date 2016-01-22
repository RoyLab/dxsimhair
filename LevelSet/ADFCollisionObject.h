#pragma once
#include "ICollisionObject.h"
#include <CGAL\Delaunay_Triangulation_3.h>
#include <CGAL\Triangulation_vertex_base_with_info_3.h>

namespace WR
{


    class ADFCollisionObject :
        public ICollisionObject
    {
        friend class ADFOctree;

        struct VInfo
        {
            float           minDist = 1.0e9f;
            Vector_3        gradient;
            //int             idx; // for dubug
        };

        typedef CGAL::Triangulation_vertex_base_with_info_3<VInfo, K>                       Vb;
        typedef CGAL::Triangulation_data_structure_3<Vb>                                    Tds;
        typedef CGAL::Delaunay_triangulation_3<K, Tds, CGAL::Location_policy<CGAL::Fast>>   Dt;

    public:
        ADFCollisionObject();
        ~ADFCollisionObject();

        virtual float query_distance(const Point_3& p) const = 0;
        virtual float query_squared_distance(const Point_3& p) const = 0;
        virtual bool exceed_threshhold(const Point_3& p, float thresh = 0.f) const = 0;
        virtual bool position_correlation(const Point_3& p, Point_3* pCorrect, float thresh = 0.f) const = 0;

        bool save_model(const wchar_t*);
        bool load_model(const wchar_t*);

    private:

        Dt*     pDt;

    };
}
