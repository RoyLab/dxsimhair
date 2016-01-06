#include "precompiled.h"
#include <iostream>
#include <CGAL\Triangle_3.h>
#include <CGAL\Exact_predicates_inexact_constructions_kernel.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Triangle_3 Tri;

void test()
{
	Tri tri;
	tri.vertex(1);
	K::Vector_3 v;
}