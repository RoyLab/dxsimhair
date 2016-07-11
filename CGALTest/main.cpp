#define _CRT_SECURE_NO_WARNINGS
#define CGAL_EIGEN3_ENABLED

#include <CGAL/Epick_d.h>
#include <CGAL/point_generators_d.h>
#include <CGAL/Kd_tree.h>
#include <CGAL/Fuzzy_sphere.h>
#include <CGAL/Fuzzy_iso_box.h>
#include <CGAL/Search_traits_d.h>

#include <list>
#include <boost/timer/timer.hpp>
#include <windows.h>


const int D = 4;
typedef CGAL::Epick_d<CGAL::Dimension_tag<D> > K;
typedef K::Point_d Point_d;
typedef CGAL::Search_traits_d<K, CGAL::Dimension_tag<D> >  Traits;

typedef CGAL::Random_points_in_cube_d<Point_d>       Random_points_iterator;
typedef CGAL::Counting_iterator<Random_points_iterator> N_Random_points_iterator;

typedef CGAL::Kd_tree<Traits> Tree;
typedef CGAL::Fuzzy_sphere<Traits> Fuzzy_sphere;
typedef CGAL::Fuzzy_iso_box<Traits> Fuzzy_iso_box;
int main() {

	const int N = 30000;
	// generator for random data points in the square ( (-1000,-1000), (1000,1000) )
	Random_points_iterator rpit(4, 5000.0);
	// Insert N points in the tree

	Tree *tree;
	{
		boost::timer::auto_cpu_timer t;

		tree = new Tree(N_Random_points_iterator(rpit, 0),
			N_Random_points_iterator(rpit, N));
	}

	// define range query objects
	double  pcoord[D] = { 300, 300, 300, 300 };
	double  qcoord[D] = { 900.0, 900.0, 900.0, 900.0 };
	Point_d p(D, pcoord + 0, pcoord + D);
	Point_d q(D, qcoord + 0, qcoord + D);
	Fuzzy_sphere fs(p, 700.0, 100.0);
	Fuzzy_iso_box fib(p, q, 100.0);


	std::cout << "points approximately in fuzzy spherical range query" << std::endl;
	std::cout << "with center (300, 300, 300, 300)" << std::endl;
	std::cout << "and fuzzy radius [600, 800] are:" << std::endl;

	std::list<Point_d> output;
	{
		boost::timer::auto_cpu_timer t;
		tree->search(std::back_inserter(output), fib);
	}

	std::cout << output.size() << std::endl;

	std::cout << "points approximately in fuzzy rectangular range query ";
	std::cout << "[[200, 400], [800,1000]]^4 are:" << std::endl;

	{
		for (int k = 0; k < 10; k++)
		{
			boost::timer::auto_cpu_timer t;
			for (int i = 0; i < 1000000; i++)
			{
				fs = Fuzzy_sphere(*rpit, 700.0, 200.0);

				tree->search(std::back_inserter(output), fs);
				output.clear();
			}
		}
	}

	std::cout << output.size() << std::endl;

	system("pause");

	return 0;
}