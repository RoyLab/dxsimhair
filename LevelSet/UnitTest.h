#pragma once
#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/boost/graph/graph_traits_Polyhedron_3.h>
#include <CGAL/AABB_face_graph_triangle_primitive.h>

#include <CGAL/Polyhedron_3.h>
#include <CGAL/IO/Polyhedron_iostream.h>
#include <CGAL/point_generators_3.h>
#include <CGAL/Side_of_triangle_mesh.h>
#include "macros.h"

namespace CGAL
{
namespace Ext
{
    template<class Polyhedron, class Kernel>
    class DistanceTester
    {
        typedef typename Kernel::FT FT;
        typedef typename Kernel::Point_3 Point;
        typedef CGAL::AABB_face_graph_triangle_primitive<Polyhedron> Primitive;
        typedef CGAL::AABB_traits<Kernel, Primitive> Traits;
        typedef CGAL::AABB_tree<Traits> Tree;
        typedef CGAL::Side_of_triangle_mesh<Polyhedron, Kernel> WhichSide;

    public:
        DistanceTester(Polyhedron& poly)
        {
            m_tree = new Tree(poly.facets_begin(), poly.facets_end(), poly);
            m_side = new WhichSide(poly);
            m_tree->accelerate_distance_queries();
        }
        
        ~DistanceTester()
        {
            SAFE_DELETE(m_side);
            SAFE_DELETE(m_tree);
        }

        FT query_signed_distance(const Point& p) const
        {
            FT sqd = m_tree->squared_distance(p);
            int s = determine_sign(*m_side, p);
            return sqrt(sqd) * s;
        }

    private:

        int determine_sign(const WhichSide& inside, const Point& query) const
        {
            auto res = inside(query);
            if (res == CGAL::ON_BOUNDED_SIDE) return -1;
            if (res == CGAL::ON_BOUNDARY) return 0;
            else return 1;
        }

        WhichSide*  m_side;
        Tree*       m_tree;
    };


    template<class Polyhedron, class Kernel>
    DistanceTester<Polyhedron, Kernel>* createDistanceTester(Polyhedron& poly)
    {
        return new DistanceTester<Polyhedron, Kernel>(poly);
    }

}
}

