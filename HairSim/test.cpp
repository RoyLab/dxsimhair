#include "precompiled.h"
#include <iostream>
#include "wrLevelsetOctree.h"
#include "wrMath.h"
#include <fstream>
#include <CGAL/IO/Polyhedron_iostream.h>
#define _TEST

//template <class Poly>
//typename Poly::Halfedge_handle make_cube_3(Poly& P) {
//	// appends a cube of size [0,1]^3 to the polyhedron P.
//	CGAL_precondition(P.is_valid());
//	typedef typename Poly::Point_3         Point;
//	typedef typename Poly::Halfedge_handle Halfedge_handle;
//	Halfedge_handle h = P.make_tetrahedron(Point(1, 0, 0),
//		Point(0, 0, 1),
//		Point(0, 0, 0),
//		Point(0, 1, 0));
//	Halfedge_handle g = h->next()->opposite()->next();             // Fig. (a)
//	P.split_edge(h->next());
//	P.split_edge(g->next());
//	P.split_edge(g);                                              // Fig. (b)
//	h->next()->vertex()->point() = Point(1, 0, 1);
//	g->next()->vertex()->point() = Point(0, 1, 1);
//	g->opposite()->vertex()->point() = Point(1, 1, 0);            // Fig. (c)
//	Halfedge_handle f = P.split_facet(g->next(),
//		g->next()->next()->next()); // Fig. (d)
//	Halfedge_handle e = P.split_edge(f);
//	e->vertex()->point() = Point(1, 1, 1);                        // Fig. (e)
//	P.split_facet(e, f->next()->next());                          // Fig. (f)
//	CGAL_postcondition(P.is_valid());
//	return h;
//}
//typedef Polyhedron_3::Halfedge_handle        Halfedge_handle;
void testDistanceQuery();


void test()
{
#ifdef _TEST
	testDistanceQuery();

	system("pause");
	//Sleep(5000);
	exit(0);
#endif
}

int sign(const Point_3& a)
{
	double sum = a[0] + a[1] + a[2];
	double sum1 = -a[0] + a[1] + a[2];

	if (sum1 < 2 && a[1] > 0 && a[2] > 0 && sum < 2)
		return -1;
	return 1;
}

#ifdef _TEST

Polyhedron_3* createSimpleTetrahedron()
{
	Polyhedron_3 *P = new Polyhedron_3;
	auto p0 = Point_3(-2, 0, 0);
	auto p1 = Point_3(2, 0, 0);
	auto p2 = Point_3(0, 2, 0);
	auto p3 = Point_3(0, 0, 2);
	P->make_tetrahedron(p0, p1, p2, p3);
	return P;
}

Polyhedron_3* readFile(const char* fileName)
{
	Polyhedron_3 *P = new Polyhedron_3;
	std::ifstream f(fileName);
	f >> (*P);
	f.close();
	return P;
}

void testDistanceQuery()
{
	Polyhedron_3 *P = readFile("../../models/head.off");
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