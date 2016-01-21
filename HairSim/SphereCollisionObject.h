#pragma once
#include "ICollisionObject.h"
#include <CGAL\Polyhedron_3.h>

namespace WR
{
    typedef CGAL::Polyhedron_3<K> Polyhedron_3;

    class SphereCollisionObject :
        public ICollisionObject
    {
    public:
        SphereCollisionObject() {}
        ~SphereCollisionObject() {}

        void setupFromPolyhedron(const Polyhedron_3&);

        void setParam(const Point_3& p, float r){ center = p; radius = r; }

        virtual float query_distance(const Point_3& p) const { return sqrt((p - center).squared_length()) - radius; }

        virtual float query_squared_distance(const Point_3& p) const { return 0.0f; };

        virtual bool exceed_threshhold(const Point_3& p, float thresh = 0.f) const
        {
            float sl = (p - center).squared_length();
            float l0 = thresh + radius;
            float sl0 = l0 * l0;

            if (sl < sl0) return true;
            else return false;
        }

        virtual bool position_correlation(const Point_3& p, Point_3* pCorrect, float thresh = 0.f) const
        {
            if (exceed_threshhold(p, thresh))
            {
                auto d = p - center;
                auto dn = d / sqrt(d.squared_length()) * (radius + thresh);
                *pCorrect = center + dn;
                return true;
            }
            else return false;
        }

        //CGAL::Bbox_3 bbox() const { return box; };
        //Point_3 center() const { return Point_3((box.xmax() + box.xmin()) / 2, (box.ymax() + box.ymin()) / 2, (box.zmax() + box.zmin()) / 2); }
        //float radius() const { return sqrt(Vector_3((box.xmax() - box.xmin()) / 2, (box.ymax() - box.ymin()) / 2, (box.zmax() - box.zmin()) / 2).squared_length()) / 1.4; }
    private:

    private:
        Point_3     center;
        float       radius;
    };
}


