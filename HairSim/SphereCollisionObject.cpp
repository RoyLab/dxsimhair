#include "DXUT.h"
#include "SphereCollisionObject.h"
#include <CGAL\bounding_box.h>

namespace WR
{
    void SphereCollisionObject::setupFromPolyhedron(const Polyhedron_3& poly)
    {
        auto box = CGAL::bounding_box(poly.points_begin(), poly.points_end());
        center = box.min() + (box.max() - box.min()) / 2.0f;
        radius = sqrt((box.max() - box.min()).squared_length());

        radius *= 0.3f;
    }

}