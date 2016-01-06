#include "precompiled.h"
#include <iostream>
#include "wrLevelsetOctree.h"
#include "wrMath.h"

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


void test()
{
#ifdef _TEST
	Polyhedron_3 *P = new Polyhedron_3;
	auto p0 = Point_3(-2, 0, 0);
	auto p1 = Point_3(2, 0, 0);
	auto p2 = Point_3(0, 2, 0);
	auto p3 = Point_3(0, 0, 2);
	P->make_tetrahedron(p0, p1, p2, p3);

	wrLevelsetOctree* pTree = new wrLevelsetOctree;
	pTree->construct(*P, 1);
	std::cout << "result as follows:\n";

	Point_3 a = Point_3(9,9,9);
	pTree->queryDistance(a);

	system("pause");
	//Sleep(5000);
	exit(0);
#endif
}

#ifdef _TEST
void testDistanceQuery()
{
	Polyhedron_3 *P = new Polyhedron_3;
	auto p0 = Point_3(-2, 0, 0);
	auto p1 = Point_3(2, 0, 0);
	auto p2 = Point_3(0, 2, 0);
	auto p3 = Point_3(0, 0, 2);
	P->make_tetrahedron(p0, p1, p2, p3);


	wrLevelsetOctree* pTree = new wrLevelsetOctree;
	pTree->construct(*P, 4);
	std::cout << "result as follows:\n";

	Point_3 a;
	int n = 50;
	while (n--)
	{
		//std::cin >> a;
		a = Point_3(0.7425, 0.2275, 1);
		a = Point_3(randf(), randf(), randf());
		double sum = a[0] + a[1] + a[2];
		double sum1 = -a[0] + a[1] + a[2];
		int s = 1;
		if (sum1 < 2 && a[1] > 0 && a[2] > 0 && sum < 2)
			s = -1;
		std::cout << std::setprecision(2) << s * pTree->queryExactDistance(a) + pTree->queryDistance(a) << "\t " << a << " sum: " << sum << "*****************************\n";

	}
	delete P;
	delete pTree;
}
#endif