#include <cstring>
#include "wrMath.h"


#define _TEST

#ifdef _TEST

#include "wrLevelsetOctree.h"
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/AABB_face_graph_triangle_primitive.h>
typedef K::FT FT;
typedef K::Point_3 Point;
typedef K::Segment_3 Segment;
typedef CGAL::AABB_face_graph_triangle_primitive<Polyhedron_3> Primitive;
typedef CGAL::AABB_traits<K, Primitive> Traits;
typedef CGAL::AABB_tree<Traits> Tree;
typedef Tree::Point_and_primitive_id Point_and_primitive_id;


int sign(const Point_3& a)
{
	double sum = a[0] + a[1] + a[2];
	double sum1 = -a[0] + a[1] + a[2];

	if (sum1 < 2 && a[1] > 0 && a[2] > 0 && sum < 2)
		return -1;
	return 1;
}


template <class Poly>
Poly* readFile(const char* fileName)
{
	Poly *P = new Poly;
	std::ifstream f(fileName);
	f >> (*P);
	f.close();
	return P;
}

void testDistanceQuery()
{
	Polyhedron_3 *P = readFile<Polyhedron_3>("../../models/head.off");
	std::cout << P->size_of_vertices() << std::endl;

	wrLevelsetOctree* pTree = new wrLevelsetOctree;
	pTree->testSign = &sign;
	pTree->construct(*P, 4);
	std::cout << "result as follows:\n";

	Point_3 a;
	int n = 50;
	while (n--)
	{
		//std::cin >> a;
		//a = Point_3(4, 4, 4);
		a = Point_3(2 * randSignedFloat(), 2 * randSignedFloat(), 2 * randSignedFloat());

		double distestimate = pTree->queryDistance(a);
		double dist = sign(a) * pTree->queryExactDistance(a);

		std::cout
			<< std::setprecision(2)
			<< std::setiosflags(std::ios::fixed)
			<< std::setw(10) << fabs(distestimate) - fabs(dist)
			<< std::setw(10) << distestimate
			<< std::setw(10) << dist << '\t'
			<< a << std::endl;
	}
	delete P;
	delete pTree;

}


#endif


void test()
{
#ifdef _TEST
	Polyhedron_3 *P = readFile<Polyhedron_3>("../../models/head.off");
	auto &polyhedron = *P;

	Tree tree(polyhedron.facets_begin(), polyhedron.facets_end(), polyhedron);
	tree.accelerate_distance_queries();
	// query point
	Point query(0.0, 0.0, 0.0);
	// computes squared distance from query

	wrLevelsetOctree* pTree = new wrLevelsetOctree;
	pTree->construct(*P, 4);
	std::cout << "result as follows:\n";

	int n = 20;
	while (n--)
	{
		//query = Point(1 * randSignedFloat(), 1 * randSignedFloat(), 1 * randSignedFloat());
		FT sqd = tree.squared_distance(query);
		std::cout << "haha: " << sqrt(sqd) << std::endl;
		double distestimate = pTree->queryDistance(query);
		double dist = pTree->queryExactDistance(query);
		std::cout << "haha1: " << distestimate << std::endl;
		std::cout << "haha2: " << dist << std::endl << std::endl;
	}


	delete P;
	delete pTree;


	system("pause");
	exit(0);
#endif
}


