#pragma once
#include <CGAL\Simple_cartesian.h>
#include <CGAL\Filtered_kernel.h>

namespace CGAL
{
    class FloatKernel
        : public CGAL::Filtered_kernel_adaptor<
        Type_equality_wrapper< Simple_cartesian<float>::Base<FloatKernel>::Type, FloatKernel >,
#ifdef CGAL_NO_STATIC_FILTERS
        false >
#else
        true >
#endif
    {};
}

namespace WR
{


    typedef CGAL::FloatKernel     K;

    // since most are geometry computation, using CGAL Point_3
    class ICollisionObject
    {
    public:
        typedef K::Point_3                                              Point_3;
        typedef K::Vector_3                                             Vector_3;

    public:
        ICollisionObject(){}
        virtual ~ICollisionObject(){}

        virtual float query_distance(const Point_3& p) const = 0;
        virtual float query_squared_distance(const Point_3& p) const = 0;
        virtual bool exceed_threshhold(const Point_3& p, float thresh = 0.f) const = 0;
        virtual bool position_correlation(const Point_3& p, Point_3* pCorrect, float thresh = 0.f) const = 0;
    };
}