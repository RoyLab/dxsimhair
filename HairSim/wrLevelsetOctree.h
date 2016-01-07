#pragma once
#include "linmath.h"
#include "wrGeo.h"
#include <list>
#include <vector>
#include <CGAL\Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL\Delaunay_Triangulation_3.h>
#include <CGAL\Triangulation_vertex_base_with_info_3.h>
#include <CGAL\Polyhedron_3.h>
#include <CGAL\Point_3.h>
#include <CGAL\Triangle_3.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_3 Point_3;
typedef K::Vector_3 Vector_3;
typedef K::Iso_cuboid_3 Cube;


// A face type with a color member variable.

template < class Refs, class T = Tag_true, class Pln = Tag_false>
struct wrFace : public CGAL::HalfedgeDS_face_base<Refs, T, Pln> {
	int idx;
	wrFace() {}
};

// An items type using my face.
struct wrItems : public CGAL::Polyhedron_items_3 {
	template <class Refs, class Traits>
	struct Face_wrapper {
		typedef typename Traits::Plane_3 Plane;
		typedef wrFace<Refs, CGAL::Tag_true, Plane> Face;
	};
};
typedef CGAL::Polyhedron_3<K, wrItems>		Polyhedron_3;
typedef Polyhedron_3::Halfedge_handle			Halfedge_handle;


template <class R_, class T_, class FH>
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

	void initInfo()
	{
		E0 = vertex(1) - vertex(0);
		E1 = vertex(2) - vertex(0);

		infos.a = E0 * E0;
		infos.b = E0 * E1;
		infos.c = E1 * E1;

		normal = CGAL::normal(vertex(0), vertex(1), vertex(2));
		normal = normal / sqrt(normal.squared_length());
	}

	void computeInfo(const Point_3& p)
	{
		Vector_3 D = vertex(0) - p;
		infos.d = E0 * D;
		infos.e = E1 * D;
		infos.f = D * D;
	}

	WRG::PointTriangleDistInfo<T_>	infos;
	Vector_3							E0, E1;
	Vector_3							normal;
	FH								fh;
};

typedef wrTriangle<K, K::FT, Polyhedron_3::Face_handle> Triangle_3;

class wrLevelsetOctree
{
public:

	struct VInfo
	{
		float		minDist = 1.0e9f;
		Vector_3		gradient;

		// for dubug
		int		idx;
	};

	typedef CGAL::Triangulation_vertex_base_with_info_3<VInfo, K>						Vb;
	typedef CGAL::Triangulation_data_structure_3<Vb>										Tds;
	typedef CGAL::Delaunay_triangulation_3<K, Tds, CGAL::Location_policy<CGAL::Fast>>   DT_3;

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
	void releaseExceptDt();

	bool construct(Polyhedron_3& geom, size_t maxLvl);
	float queryDistance(const Point_3& p) const;
	bool queryGradient(const Point_3& p, Vector_3& grad) const;
	float queryExactDistance(const Point_3& p) const;

public: // for debug
	int(*testSign)(const Point_3&);

private:

	void constructChildren(Node*);
	Node* createRootNode(const Polyhedron_3&);
	void computeMinDistance(Node*);
	int determineSign(int type, const Point_3& p, const Vector_3& diff, size_t triIdx) const;

	template <class Iterator>
	double minSquaredDist(const Point_3& p, Iterator begin, Iterator end, Vector_3* diff = nullptr, size_t* tri = nullptr, int* type = nullptr) const;

	double minDist(const Cube& bbox, const Point_3& p);
	void computeGradient();

	int detSignOnFace(const Point_3& p, const Vector_3& diff, size_t triIdx) const;
	int detSignOnEdge(const Point_3& p, const Vector_3& diff, size_t triIdx, int seq) const;
	int detSignOnVertex(const Point_3& p, const Vector_3& diff, size_t triIdx, int seq) const;

	void release();
	Node* createNode();
	void computeTripleFromBbox(Cube&, const Cube&) const;

	Node*						pRoot = nullptr;
	size_t						nMaxLevel = 0;
	DT_3							dt;
	
	std::vector<Node*>			cellList;
	Triangle_3*					triList = nullptr;
	size_t						nTriangles = 0;
};

