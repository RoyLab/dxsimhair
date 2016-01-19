#pragma once
#include "ICollisionObject.h"

namespace WR
{
    class SphereCollisionObject :
        public ICollisionObject
    {
    public:
        SphereCollisionObject();
        ~SphereCollisionObject();

        float queryDistance(const Point_3& p) const { return sqrt((p - center()).squared_length()) - radius(); }

        CGAL::Bbox_3 bbox() const { return box; };
        Point_3 center() const { return Point_3((box.xmax() + box.xmin()) / 2, (box.ymax() + box.ymin()) / 2, (box.zmax() + box.zmin()) / 2); }
        float radius() const { return sqrt(Vector_3((box.xmax() - box.xmin()) / 2, (box.ymax() - box.ymin()) / 2, (box.zmax() - box.zmin()) / 2).squared_length()) / 1.4; }

    private:
        Point_3 center;
        float    radius;
    };
}


