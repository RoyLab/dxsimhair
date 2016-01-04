#pragma once
#include "linmath.h"
#include <CGAL\Exact_predicates_inexact_constructions_kernel.h>
#include <list>
#include <CGAL\Polyhedron_3.h>
#include <CGAL\Delaunay_Triangulation_3.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Polyhedron_3<K> Polyhedron_3;
typedef K::Triangle_3 Triangle_3;
typedef K::Point_3 Point_3;
typedef K::Iso_cuboid_3 Box_3;
typedef CGAL::Delaunay_Triangulation_3<K> DT_3;

//typedef Triangulation_3 TdsVHandle;

struct wrLevelsetOctreeNode
{
	size_t					level;
	vec3						corners[2];

    //TdsVHandle              vertices[8];
    //std::list<Triangle_3>

	wrLevelsetOctreeNode*	pParent;
	wrLevelsetOctreeNode*	children[8];
};

class wrLevelsetOctree
{
public:
    struct VInfo
    {
        Point_3 pos;
        float minDist;
    };
public:
	wrLevelsetOctree();
	~wrLevelsetOctree();

    void construct();

private:

	wrLevelsetOctreeNode*	pRoot = nullptr;
	size_t					nMaxLevel = 0;
	
    std::list<wrLevelsetOctreeNode*>    cellList;
    
};

