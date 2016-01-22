#include "LevelSet.h"
#include "ConfigReader.h"
#include "wrMath.h"

#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/AABB_face_graph_triangle_primitive.h>
#include <CGAL/IO/Polyhedron_iostream.h>
#include <CGAL/boost/graph/graph_traits_Polyhedron_3.h>
#include <CGAL/point_generators_3.h>
#include <CGAL/Side_of_triangle_mesh.h>
#include <vector>
#include <fstream>
#include <limits>
#include <boost/foreach.hpp>

using std::cout;
using std::endl;

namespace WR
{ 

    typedef K::FT FT;
    typedef K::Point_3 Point;
    typedef K::Segment_3 Segment;
    typedef CGAL::AABB_face_graph_triangle_primitive<Polyhedron_3_FaceWithId> Primitive;
    typedef CGAL::AABB_traits<K, Primitive> Traits;
    typedef CGAL::AABB_tree<Traits> Tree;



    template <class K, class PolygonSide>
    int determine_sign(const PolygonSide& inside, const CGAL::Point_3<K>& query)
    {
        auto res = inside(query);
        if (res == CGAL::ON_BOUNDED_SIDE) return 1;
        if (res == CGAL::ON_BOUNDARY) return 0;
        else return -1;
    }


    ICollisionObject* createCollisionObject(Polyhedron_3_FaceWithId& poly)
    {
        ADFOctree* pTree = new ADFOctree;
        pTree->construct(poly, 2);
        ICollisionObject* pCO = pTree->createCollisionObject();
        delete pTree;
        return pCO;
    }

    ICollisionObject* createCollisionObject(const wchar_t* fileName)
    {
        Polyhedron_3_FaceWithId* pModel = WRG::readFile<Polyhedron_3_FaceWithId>(fileName);
        assert(pModel);
        ICollisionObject* pCO = createCollisionObject(*pModel);
        assert(pCO);
        delete pModel;
        return pCO;
    }

    template <class Poly>
    double max_coordinate(const Poly& poly)
    {
        double max_coord = (std::numeric_limits<double>::min)();
        BOOST_FOREACH(Poly::Vertex_handle v, vertices(poly))
        {
            Point p = v->point();
            max_coord = (std::max)(max_coord, p.x());
            max_coord = (std::max)(max_coord, p.y());
            max_coord = (std::max)(max_coord, p.z());
        }
        return max_coord;
    }

    void runLevelSetBenchMark(const wchar_t* fileName)
    {
        ConfigReader reader("..\\HairSim\\config.ini");
        int level = std::stoi(reader.getValue("maxlevel"));
        int number = std::stoi(reader.getValue("testnumber"));

        Polyhedron_3_FaceWithId* pModel = WRG::readFile<Polyhedron_3_FaceWithId>(fileName);
        assert(pModel);

        Tree tree(pModel->facets_begin(), pModel->facets_end(), *pModel);
        tree.accelerate_distance_queries();
        CGAL::Side_of_triangle_mesh<Polyhedron_3_FaceWithId, K> inside(*pModel);

        ADFOctree* pTree = new ADFOctree;
        pTree->construct(*pModel, level);
        
        double size = max_coordinate(*pModel);
        std::vector<Point> points;
        points.reserve(number);
        CGAL::Random_points_in_cube_3<Point> gen(size);
        for (unsigned int i = 0; i < number; ++i)
            points.push_back(*gen++);

        while (number--)
        {
            auto & p = points[number-1];
            double dist = pTree->query_distance(p);

            FT sqd = tree.squared_distance(p);
            int s = determine_sign(inside, p);

            cout << "Point: " << p << endl;
            cout << " Dist1: " << dist << " Dist2: " << sqrt(sqd) * s <<
                "\t\t diff: " << dist - sqrt(sqd) * s << endl;
        }

        delete pTree;
        delete pModel;
    }
}