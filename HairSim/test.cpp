#include <cstring>
#include "wrMath.h"


//#define _TEST

#ifdef _TEST

//#include "wrLevelsetOctree.h"
//#include <CGAL/AABB_tree.h>
//#include <CGAL/AABB_traits.h>
//#include <CGAL/Polyhedron_3.h>
//#include <CGAL/AABB_face_graph_triangle_primitive.h>
//#include <CGAL/IO/Polyhedron_iostream.h>
//#include <CGAL/boost/graph/graph_traits_Polyhedron_3.h>
//#include <CGAL/point_generators_3.h>
//#include <CGAL/Side_of_triangle_mesh.h>
//#include <vector>
//#include <fstream>
//#include <limits>
//#include <boost/foreach.hpp>
//
//
//typedef K::FT FT;
//typedef K::Point_3 Point;
//typedef K::Segment_3 Segment;
//typedef CGAL::AABB_face_graph_triangle_primitive<Polyhedron_3> Primitive;
//typedef CGAL::AABB_traits<K, Primitive> Traits;
//typedef CGAL::AABB_tree<Traits> Tree;
//typedef Tree::Point_and_primitive_id Point_and_primitive_id;
//
//
//int sign(const Point_3& a)
//{
//    double sum = a[0] + a[1] + a[2];
//    double sum1 = -a[0] + a[1] + a[2];
//
//    if (sum1 < 2 && a[1] > 0 && a[2] > 0 && sum < 2)
//        return -1;
//    return 1;
//}
//
//
//
//template <class K>
//int sign(CGAL::Side_of_triangle_mesh<const CGAL::Polyhedron_3<K>, K>& inside, const CGAL::Point_3<K>& query)
//{
//    auto res = inside(query);
//    if (res == CGAL::ON_BOUNDED_SIDE) return 1;
//    if (res == CGAL::ON_BOUNDARY) return 0;
//    else return -1;
//}
//
//
//template <class Poly>
//Poly* readFile(const char* fileName)
//{
//    Poly *P = new Poly;
//    std::ifstream f(fileName);
//    f >> (*P);
//    f.close();
//    return P;
//}
//
//void testDistanceQuery()
//{
//    Polyhedron_3 *P = readFile<Polyhedron_3>("../../models/head.off");
//    std::cout << P->size_of_vertices() << std::endl;
//
//    wrLevelsetOctree* pTree = new wrLevelsetOctree;
//    pTree->construct(*P, 4);
//    std::cout << "result as follows:\n";
//
//    Point_3 a;
//    int n = 50;
//    while (n--)
//    {
//        //std::cin >> a;
//        //a = Point_3(4, 4, 4);
//        a = Point_3(2 * randSignedFloat(), 2 * randSignedFloat(), 2 * randSignedFloat());
//
//        double distestimate = pTree->queryDistance(a);
//        double dist = sign(a) * pTree->queryExactDistance(a);
//
//        std::cout
//            << std::setprecision(2)
//            << std::setiosflags(std::ios::fixed)
//            << std::setw(10) << fabs(distestimate) - fabs(dist)
//            << std::setw(10) << distestimate
//            << std::setw(10) << dist << '\t'
//            << a << std::endl;
//    }
//    delete P;
//    delete pTree;
//
//}
//
//void test2()
//{
//    Polyhedron_3 *P = readFile<Polyhedron_3>("../../models/head.off");
//    auto &polyhedron = *P;
//
//    Tree tree(polyhedron.facets_begin(), polyhedron.facets_end(), polyhedron);
//    tree.accelerate_distance_queries();
//    // query point
//    Point query(0.0, 0.0, 0.0);
//    // computes squared distance from query
//    CGAL::Side_of_triangle_mesh<Polyhedron_3, K> inside(polyhedron);
//    wrLevelsetOctree* pTree = new wrLevelsetOctree;
//    pTree->construct(*P, 8);
//    std::cout << "result as follows:\n";
//
//    int n = 50;
//    int size = 1;
//    while (n--)
//    {
//
//        int s;
//        auto res = inside(query);
//        std::cout << "haha: " << res << " fafasd " << query << std::endl;
//        if (res == CGAL::ON_BOUNDED_SIDE) s = -1;
//        else if (res == CGAL::ON_BOUNDARY) s = 0;
//        else s = 1;
//        query = Point(size * randSignedFloat(), size * randSignedFloat(), size * randSignedFloat());
//
//        FT sqd = tree.squared_distance(query);
//        std::cout << "haha: " << sqrt(sqd) << std::endl;
//        double distestimate = pTree->queryDistance(query);
//        double dist = s * pTree->queryExactDistance(query);
//        std::cout << "haha1: " << distestimate << std::endl;
//        std::cout << "haha2: " << dist << std::endl << std::endl;
//    }
//
//
//    delete P;
//    delete pTree;
//}

#include "ConfigReader.h"
#include <iostream>
#endif


void test()
{
#ifdef _TEST
    ConfigReader reader("config.ini");
    std::cout << reader.getValue("val");

    system("pause");
    exit(0);
#endif
}


