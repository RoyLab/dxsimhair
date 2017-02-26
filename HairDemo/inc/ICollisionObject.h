#pragma once
#include "xhair.h"
//#include <CGAL\Simple_cartesian.h>
//#include <CGAL\Filtered_kernel.h>
//#include <limits>
//#include <CGAL\Delaunay_Triangulation_3.h>
//#include <CGAL\Triangulation_vertex_base_with_info_3.h>
//
//#include "CGALKernel.h"

namespace xhair
{
    // since most are geometry computation, using CGAL Point_3
    class ICollisionObject
    {
//    public:
//        typedef CGAL::Epickf    K;
//        typedef K::Point_3      Point_3;
//        typedef K::Vector_3     Vector_3;
//        typedef K::Direction_3  Direction_3;
//
//		struct VInfo
//		{
//#ifdef max
//#undef max
//#endif
//			float           minDist = std::numeric_limits<float>::max();
//			Vector_3        gradient;
//		};
//
//		typedef CGAL::Triangulation_vertex_base_with_info_3<VInfo, K>                       Vb;
//		typedef CGAL::Triangulation_data_structure_3<Vb>                                    Tds;
//		typedef CGAL::Delaunay_triangulation_3<K, Tds, CGAL::Location_policy<CGAL::Fast>>   Dt;
//
    public:
        typedef Point3 Point_3;

        ICollisionObject(){}
        virtual ~ICollisionObject(){}

        virtual float query_distance(const Point_3& p) const = 0;
        virtual float query_squared_distance(const Point_3& p) const = 0;
        virtual bool exceed_threshhold(const Point_3& p, float thresh = 0.f) const = 0;
        virtual bool position_correlation(const Point_3& p, Point_3* pCorrect, float thresh = 0.f) const = 0;
    };
}