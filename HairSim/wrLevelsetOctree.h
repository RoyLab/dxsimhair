#pragma once
#include "linmath.h"
#include <list>
#include <vector>
#include <CGAL\Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL\Polyhedron_3.h>
#include <CGAL\Point_3.h>
#include <CGAL\Bbox_3.h>
#include <CGAL\Delaunay_Triangulation_3.h>
#include <CGAL\Triangulation_vertex_base_with_info_3.h>
#include <CGAL\Triangle_3.h>
#include "wrGeo.h"

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Polyhedron_3<K> Polyhedron_3;
typedef K::Point_3 Point_3;
typedef K::Iso_cuboid_3 Cube;

using CGAL::Bbox_3;

template <class T>
struct PointTriangleDistInfo
{
	T a, b, c, d, e, f;
};

template <class R_, class T_>
class wrTriangle : public CGAL::Triangle_3 < R_ >
{
	typedef typename R_::Point_3					Point_3;
	typedef typename R_::Vector_3				Vector_3;
	typedef typename CGAL::Triangle_3 < R_ >		Parent;

public:
	wrTriangle() {}

	wrTriangle(const Parent& t)
		: Parent(t) {}

	wrTriangle(const Point_3& p, const Point_3& q, const Point_3& r) :
		Parent(p, q, r){}

	virtual void initInfo() = 0;
	virtual void computeInfo(const Point_3& p) = 0;

	WRG::PointTriangleDistInfo<T_>	infos;
	Vector_3							E0, E1;
};

typedef wrTriangle<K, K::FT> Triangle_3;

class wrLevelsetOctree
{
public:
	struct VInfo
	{
		float	minDist = 1.0e9f;
		vec3		gradient;
	};

	typedef CGAL::Triangulation_vertex_base_with_info_3<VInfo, K>		Vb;
	typedef CGAL::Triangulation_data_structure_3<Vb>                    Tds;
	typedef CGAL::Delaunay_triangulation_3<K, Tds>                      DT_3;

	struct Node
	{
		size_t					level;
		Cube						bbox, triple;

		//   0---------1
		//  /|        /|
		// 3---------2 |
		// | |       | |
		// | |       | |
		// | 4-------|-5
		// |/        |/
		// 7---------6
		//   
		//   z
		//   |
		//   •----y
		//  /
		// x

		DT_3::Vertex_handle		vertices[8];
		std::vector<unsigned>	eList;

		Node*	pParent = nullptr;
		Node*	children[8];

		bool hasChild() const { return children[0] == nullptr; }
	};

public:

	wrLevelsetOctree();
	~wrLevelsetOctree();

	bool construct(const Polyhedron_3& geom, size_t);

private:

	void constructChildren(Node*);
	Node* createRootNode(const Polyhedron_3& geom);
	void computeMinDistance(Node*);
	virtual void computeGradient() = 0;

	void release();
	Node* createNode();
	void computeTripleFromBbox(Cube&, const Cube&);

	Node*						pRoot = nullptr;
	size_t						nMaxLevel = 0;
	DT_3							dt;
	
	std::vector<Node*>			cellList;
	Triangle_3*					triList = nullptr;
	size_t						nTriangles = 0;
};

