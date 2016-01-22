#include "ADFOctree.h"
#include "wrMacro.h"
#include "wrLogger.h"
#include "wrGeo.h"
#include "linmath.h"
#include <CGAL\bounding_box.h>

namespace
{
    const unsigned EIDX[12][3] = {
        { 0, 1, 2 }, { 1, 2, 2 }, { 2, 1, 2 }, { 1, 0, 2 },
        { 0, 0, 1 }, { 0, 2, 1 }, { 2, 2, 1 }, { 2, 0, 1 },
        { 0, 1, 0 }, { 1, 2, 0 }, { 2, 1, 0 }, { 1, 0, 0 }
    };

    const unsigned FIDX[6][3] = {
        { 1, 1, 2 }, { 0, 1, 1 }, { 1, 2, 1 },
        { 2, 1, 1 }, { 1, 0, 1 }, { 1, 1, 0 }
    };

    const unsigned BIDX[8][2] = {
        { 12, 20 }, { 21, 9 }, { 26, 2 }, { 24, 10 },
        { 4, 26 }, { 16, 22 }, { 25, 14 }, { 19, 23 }
    };

    const unsigned CIDX[8][8] = {
        { 0, 8, 20, 11, 12, 21, 26, 24 },
        { 8, 1, 9, 20, 21, 13, 22, 26 },
        { 20, 9, 2, 10, 26, 22, 14, 23 },
        { 11, 20, 10, 3, 24, 26, 23, 15 },
        { 12, 21, 26, 24, 4, 16, 25, 19 },
        { 21, 13, 22, 26, 16, 5, 17, 25 },
        { 26, 22, 14, 23, 25, 17, 6, 18 },
        { 24, 26, 23, 15, 19, 25, 18, 7 }
    };
}

namespace WR
{


    ADFOctree::ADFOctree()
    {
    }


    ADFOctree::~ADFOctree()
    {
        release();
    }

    ADFCollisionObject* ADFOctree::createCollisionObject()
    {
        return nullptr;
    }


    bool ADFOctree::construct(Polyhedron_3& geom, size_t maxLvl)
    {
        release();
        nMaxLevel = maxLvl;
        dt = new Dt;

        // contruct all the triangles
        nTriangles = geom.size_of_facets();
        triList = new Triangle_3[nTriangles];

        size_t count = 0;
        for (auto tItr = geom.facets_begin(); tItr != geom.facets_end(); tItr++, count++)
        {
            if (!tItr->is_triangle())
                throw std::exception("Not a triangle");

            auto ffItr = tItr->facet_begin();
            Point_3 vP[3];
            for (size_t i = 0; i < 3; i++, ffItr++)
                vP[i] = ffItr->vertex()->point();

            triList[count] = Triangle_3(vP[0], vP[1], vP[2]);
            triList[count].fh = tItr;
            tItr->idx = count;
            triList[count].initInfo();
        }

        pRoot = createRootNode(geom);

        constructChildren(pRoot);
        //computeGradient();

        return true;
    }

    void ADFOctree::constructChildren(Node* root)
    {

        //   /----0----/            •-----y
        //  3|        1|            |
        // /----2----/ |            |
        // | 4       | 5            x
        // | |       | |              1
        // 7 /----8--6-/          4  0(5)  2  
        // |11       |9               3
        // /----10---/

        // 顶点0-7，边中点8-19，面心20-25，体心26
        // 子节点方位标记和顶点方位标记同
        DtVh v[27];
        for (size_t i = 0; i < 8; i++)
            v[i] = root->vertices[i];

        vec3 coords[3];

        coords[0][0] = root->bbox.xmin();
        coords[0][1] = (root->bbox.xmin() + root->bbox.xmax()) / 2.0f;
        coords[0][2] = root->bbox.xmax();

        coords[1][0] = root->bbox.ymin();
        coords[1][1] = (root->bbox.ymin() + root->bbox.ymax()) / 2.0f;
        coords[1][2] = root->bbox.ymax();

        coords[2][0] = root->bbox.zmin();
        coords[2][1] = (root->bbox.zmin() + root->bbox.zmax()) / 2.0f;
        coords[2][2] = root->bbox.zmax();

        const unsigned OE = 8;
        for (size_t i = 0; i < 12; i++)
            v[i + OE] = dt->insert(Point_3(coords[0][EIDX[i][0]], coords[1][EIDX[i][1]], coords[2][EIDX[i][2]]), root->vertices[0]);

        const unsigned OF = 20;
        for (size_t i = 0; i < 6; i++)
            v[i + OF] = dt->insert(Point_3(coords[0][FIDX[i][0]], coords[1][FIDX[i][1]], coords[2][FIDX[i][2]]), root->vertices[0]);

        v[26] = dt->insert(Point_3(coords[0][1], coords[1][1], coords[2][1]), root->vertices[0]);

        // deploy chidren
        for (size_t i = 0; i < 8; i++)
        {
            Node* child = createNode();
            child->level = root->level + 1;
            child->pParent = root;

            for (size_t j = 0; j < 8; j++)
                child->vertices[j] = v[CIDX[i][j]];

            child->bbox = Cube_3(v[BIDX[i][0]]->point(), v[BIDX[i][1]]->point());
            computeTripleFromBbox(child->triple, child->bbox);

            for (auto eItr = root->eList.begin(); eItr != root->eList.end(); eItr++)
            {
                if (CGAL::do_intersect(triList[*eItr], child->triple.bbox()))
                    child->eList.push_back(*eItr);
            }

            // if has elements and not max level, split
            if (!child->eList.empty() && child->level < nMaxLevel)
                constructChildren(child);
            else computeMinDistance(child);

            root->children[i] = child;
        }

    }

    void ADFOctree::release()
    {
        releaseExceptDt();
        if (dt)
        {
            dt->clear();
            SAFE_DELETE(dt);
        }
    }

    void ADFOctree::releaseExceptDt()
    {
        size_t n = cellList.size();
        for (size_t i = 0; i < n; i++)
            delete cellList[i];

        SAFE_DELETE_ARRAY(triList);
        nTriangles = 0;

        pRoot = nullptr;
        nMaxLevel = 0;
        cellList.clear();
    }

    //void ADFOctree::computeGradient()
    //{
    //    Vector_3 step[3];
    //    auto& bbox = pRoot->bbox;
    //    float coef = pow(0.5f, nMaxLevel);
    //    step[0] = Vector_3(coef * (bbox.xmax() - bbox.xmin()), 0, 0);
    //    step[1] = Vector_3(0, coef * (bbox.ymax() - bbox.ymin()), 0);
    //    step[2] = Vector_3(0, 0, coef * (bbox.zmax() - bbox.zmin()));

    //    vec3 v;
    //    for (auto vItr = dt->finite_vertices_begin(); vItr != dt->finite_vertices_end(); vItr++)
    //    {
    //        auto &pos = vItr->point();
    //        for (size_t i = 0; i < 3; i++)
    //        {
    //            auto dist1 = queryDistance(pos + step[i]);
    //            auto dist2 = queryDistance(pos - step[i]);
    //            v[i] = (dist2 - dist1) / (2 * step[i][i]);
    //        }
    //        vItr->info().gradient = Vector_3(v[0], v[1], v[2]);
    //    }
    //}
    double ADFOctree::query_distance(const Point_3& p) const
    {
        int type = -1;
        size_t triIdx = 0;
        Vector_3 diff;

        double sd = minSquaredDist(p, pRoot->eList.cbegin(), pRoot->eList.cend(), &diff, &triIdx, &type);
        int sign = determineSign(type, p, diff, triIdx);

        return sign * sqrt(sd);
    }


    void ADFOctree::computeMinDistance(Node* node)
    {
        static int count = 0;
        for (size_t i = 0; i < 8; i++)
        {
            auto& vh = node->vertices[i];
            auto& dist = vh->info().minDist;
            if (dist < 1.0e8) continue;

            int type = -1;
            size_t triIdx = 0;
            Node* curNode = node;
            Vector_3 diff;
            double tmpSquaredDist, distLimit;

            while (true)
            {
                if (!curNode->eList.empty())
                {
                    tmpSquaredDist = minSquaredDist(vh->point(), curNode->eList.cbegin(), curNode->eList.cend(), &diff, &triIdx, &type);
                    distLimit = minDist(curNode->triple, vh->point());

                    if (tmpSquaredDist < distLimit * distLimit || !curNode->pParent)
                    {
                        dist = sqrt(tmpSquaredDist);
                        break;
                    }
                }
                curNode = curNode->pParent;
            }

            int sign = determineSign(type, vh->point(), diff, triIdx);
            dist *= sign;
            //assert(sign == testSign(vh->point()));

            //vh->info().idx = count++;
            ////******************************************************
            //std::cout << count++ << ": " << vh->point() << '\t';
            //std::cout << ": " << dist << std::endl;
        }
    }

    int ADFOctree::determineSign(int type, const Point_3& p, const Vector_3& diff, size_t triIdx) const
    {
        switch (type)
        {
        case 0:
            return detSignOnFace(p, diff, triIdx);
        case 1:
            return detSignOnEdge(p, diff, triIdx, 2);
        case 3:
            return detSignOnEdge(p, diff, triIdx, 0);
        case 5:
            return detSignOnEdge(p, diff, triIdx, 1);
        case 2:
            return detSignOnVertex(p, diff, triIdx, 2);
        case 4:
            return detSignOnVertex(p, diff, triIdx, 0);
        case 6:
            return detSignOnVertex(p, diff, triIdx, 1);
        default:
            throw std::exception("unexpected type.");
        }
    }

    int ADFOctree::detSignOnFace(const Point_3& p, const Vector_3& diff, size_t triIdx) const
    {
        if (diff * triList[triIdx].normal < 0.0) return 1;
        else return -1;
    }

    int ADFOctree::detSignOnEdge(const Point_3& p, const Vector_3& diff, size_t triIdx, int seq) const
    {
        auto edge = triList[triIdx].fh->facet_begin();
        for (size_t i = 0; i < seq; i++, edge++);

        Vector_3 normal = (triList[edge->opposite()->facet()->idx].normal + triList[triIdx].normal) / 2.0;
        if (diff * normal < 0.0) return 1;
        else return -1;
    }

    int ADFOctree::detSignOnVertex(const Point_3& p, const Vector_3& diff, size_t triIdx, int seq) const
    {
        auto edge = triList[triIdx].fh->facet_begin();
        for (size_t i = 0; i < seq; i++, edge++);

        auto vh = edge->vertex();
        const size_t dim = vh->degree();
        auto v_circle = vh->vertex_begin();
        Vector_3 dir(0, 0, 0);
        for (size_t i = 0; i < dim; i++, v_circle++)
        {
            dir = dir + (v_circle->opposite()->vertex()->point() - vh->point());
        }
        if (diff * dir < 0.0) return 1;
        else return -1;
    }


    // begin != end !!!
    template <class Iterator>
    double ADFOctree::minSquaredDist(const Point_3& p, Iterator begin, Iterator end, Vector_3* diff, size_t* pTriIdx, int* pType) const
    {
        assert(begin != end);

        double dist = 1.e8;
        size_t triIdx = 0;
        int type = 0;
        double s;
        double t;

        for (auto eItr = begin; eItr != end; eItr++)
        {
            WRG::PointTriangleDistResult<K::FT> res;
            auto &tri = triList[*eItr];
            tri.computeInfo(p);
            WRG::squaredDistance(p, tri, tri.infos, res);
            if (dist > res.dist)
            {
                dist = res.dist;
                type = res.type;
                s = res.s;
                t = res.t;
                triIdx = *eItr;
            }
        }
        if (diff)
            *diff = p - (triList[triIdx].vertex(0) + s * triList[triIdx].E0 + t * triList[triIdx].E1);

        if (pTriIdx) *pTriIdx = triIdx;
        if (pType) *pType = type;
        return dist;
    }

    double ADFOctree::minDist(const Cube_3& bbox, const Point_3& p)
    {
        double dist = 1.e9;

#define MIN_TEST(d, d0)\
                    {if ((d) < (d0))\
        (d0) = (d);}

        MIN_TEST(fabs(bbox.xmax() - p.x()), dist);
        MIN_TEST(fabs(bbox.xmin() - p.x()), dist);
        MIN_TEST(fabs(bbox.ymax() - p.y()), dist);
        MIN_TEST(fabs(bbox.ymin() - p.y()), dist);
        MIN_TEST(fabs(bbox.zmax() - p.z()), dist);
        MIN_TEST(fabs(bbox.zmin() - p.z()), dist);

        return dist;
    }

    ADFOctree::Node* ADFOctree::createRootNode(const Polyhedron_3& geom)
    {
        Node* node = createNode();

        Cube_3 bbox = CGAL::bounding_box(geom.points_begin(), geom.points_end());
        box = bbox.bbox();
        WR_LOG_INFO << "Bbox size: " << bbox;

        WRG::enlarge(bbox, 1.13); // 不希望贴的太紧
        node->vertices[0] = dt->insert(Point_3(bbox.xmin(), bbox.ymin(), bbox.zmax()));
        node->vertices[1] = dt->insert(Point_3(bbox.xmin(), bbox.ymax(), bbox.zmax()));
        node->vertices[2] = dt->insert(Point_3(bbox.xmax(), bbox.ymax(), bbox.zmax()));
        node->vertices[3] = dt->insert(Point_3(bbox.xmax(), bbox.ymin(), bbox.zmax()));

        node->vertices[4] = dt->insert(Point_3(bbox.xmin(), bbox.ymin(), bbox.zmin()));
        node->vertices[5] = dt->insert(Point_3(bbox.xmin(), bbox.ymax(), bbox.zmin()));
        node->vertices[6] = dt->insert(Point_3(bbox.xmax(), bbox.ymax(), bbox.zmin()));
        node->vertices[7] = dt->insert(Point_3(bbox.xmax(), bbox.ymin(), bbox.zmin()));

        node->level = 0;
        node->bbox = bbox;

        for (size_t i = 0; i < nTriangles; i++)
            node->eList.push_back(i);

        return node;
    }

    ADFOctree::Node* ADFOctree::createNode()
    {
        Node* node = new Node;
        memset(node->children, 0, sizeof(node->children));
        cellList.push_back(node);
        return node;
    }

    void ADFOctree::computeTripleFromBbox(Cube_3& triple, const Cube_3& bbox) const
    {
        Vector_3 radius = (bbox.max() - bbox.min()) / 2.0f;
        Point_3 center = bbox.min() + radius / 2.0f;
        triple = Cube_3(center - 3.0f * radius, center + 3.0f * radius);
    }

    //float ADFOctree::queryExactDistance(const Point_3& p) const
    //{
    //    return sqrt(minSquaredDist(p, pRoot->eList.begin(), pRoot->eList.end()));
    //}

    //float ADFOctree::queryDistance(const Point_3& p) const
    //{
    //    if (!dt.number_of_cells()) return -1.0f;

    //    DT_3::Cell_handle ch = dt.locate(p);

    //    Point_3 v[4];
    //    bool isInfinite = false;
    //    int infiniteId = -1;
    //    for (size_t i = 0; i < 4; i++)
    //    {
    //        if (ch->vertex(i) == dt.infinite_vertex())
    //        {
    //            assert(infiniteId == -1);
    //            isInfinite = true;
    //            infiniteId = i;
    //        }
    //        else v[i] = ch->vertex(i)->point();
    //    }

    //    if (isInfinite)
    //    {
    //        //std::cout << "this is out." << std::endl;
    //        // do nothing
    //        Triangle_3 tri(v[(infiniteId + 1) % 4], v[(infiniteId + 2) % 4], v[(infiniteId + 3) % 4]);
    //        tri.initInfo();
    //        tri.computeInfo(p);
    //        WRG::PointTriangleDistResult<K::FT> res;
    //        WRG::squaredDistance(p, tri, tri.infos, res);

    //        Point_3 touch = tri.vertex(0) + res.s * tri.E0 + res.t * tri.E1;
    //        double a[3];
    //        a[2] = WRG::squaredArea(tri.vertex(0), tri.vertex(1), touch);
    //        a[0] = WRG::squaredArea(tri.vertex(1), tri.vertex(2), touch);
    //        a[1] = WRG::squaredArea(tri.vertex(2), tri.vertex(0), touch);

    //        double dist = (a[0] * ch->vertex((infiniteId + 1) % 4)->info().minDist +
    //            (a[1] * ch->vertex((infiniteId + 2) % 4)->info().minDist) +
    //            (a[2] * ch->vertex((infiniteId + 3) % 4)->info().minDist)) /
    //            (a[0] + a[1] + a[2]);

    //        dist += sqrt(res.dist);
    //        return dist;
    //    }
    //    else
    //    {
    //        assert(CGAL::volume(v[0], v[1], v[2], v[3]) > 0);
    //        float vol[4];
    //        vol[0] = CGAL::volume(v[1], v[3], v[2], p);
    //        vol[1] = CGAL::volume(v[2], v[3], v[0], p);
    //        vol[2] = CGAL::volume(v[0], v[3], v[1], p);
    //        vol[3] = CGAL::volume(v[0], v[1], v[2], p);

    //        float sum = 0.f;
    //        float numer = 0.0f;
    //        for (size_t i = 0; i < 4; i++)
    //        {
    //            sum += vol[i];
    //            //WR_LOG_DEBUG << ch->vertex(i)->info().minDist << " idx: " << ch->vertex(i)->info().idx << " point: " << ch->vertex(i)->point();
    //            numer += vol[i] * ch->vertex(i)->info().minDist;
    //        }

    //        return numer / sum;
    //    }
    //}


    //bool ADFOctree::queryGradient(const Point_3& p, Vector_3& grad) const
    //{
    //    if (!dt.number_of_cells()) return false;

    //    grad = Vector_3(0, 0, 0);

    //    DT_3::Cell_handle ch = dt.locate(p);

    //    Point_3 v[4];
    //    bool isInfinite = false;
    //    int infiniteId = -1;
    //    for (size_t i = 0; i < 4; i++)
    //    {
    //        if (ch->vertex(i) == dt.infinite_vertex())
    //        {
    //            assert(infiniteId == -1);
    //            isInfinite = true;
    //            infiniteId = i;
    //        }
    //        else v[i] = ch->vertex(i)->point();
    //    }

    //    if (isInfinite)
    //    {
    //        //std::cout << "this is out." << std::endl;
    //        // do nothing
    //        Triangle_3 tri(v[(infiniteId + 1) % 4], v[(infiniteId + 2) % 4], v[(infiniteId + 3) % 4]);
    //        tri.initInfo();
    //        tri.computeInfo(p);
    //        WRG::PointTriangleDistResult<K::FT> res;
    //        WRG::squaredDistance(p, tri, tri.infos, res);

    //        Point_3 touch = tri.vertex(0) + res.s * tri.E0 + res.t * tri.E1;
    //        double a[3];
    //        a[2] = WRG::squaredArea(tri.vertex(0), tri.vertex(1), touch);
    //        a[0] = WRG::squaredArea(tri.vertex(1), tri.vertex(2), touch);
    //        a[1] = WRG::squaredArea(tri.vertex(2), tri.vertex(0), touch);

    //        // 不进行外推
    //        grad = (a[0] * ch->vertex((infiniteId + 1) % 4)->info().gradient +
    //            (a[1] * ch->vertex((infiniteId + 2) % 4)->info().gradient) +
    //            (a[2] * ch->vertex((infiniteId + 3) % 4)->info().gradient)) /
    //            (a[0] + a[1] + a[2]);

    //        return true;
    //    }
    //    else
    //    {
    //        assert(CGAL::volume(v[0], v[1], v[2], v[3]) > 0);
    //        float vol[4];
    //        vol[0] = CGAL::volume(v[1], v[3], v[2], p);
    //        vol[1] = CGAL::volume(v[2], v[3], v[0], p);
    //        vol[2] = CGAL::volume(v[0], v[3], v[1], p);
    //        vol[3] = CGAL::volume(v[0], v[1], v[2], p);

    //        float sum = 0.f;
    //        for (size_t i = 0; i < 4; i++)
    //        {
    //            sum += vol[i];
    //            //WR_LOG_DEBUG << ch->vertex(i)->info().minDist << " idx: " << ch->vertex(i)->info().idx << " point: " << ch->vertex(i)->point();
    //            grad = grad + vol[i] * ch->vertex(i)->info().gradient;
    //        }
    //        grad = grad / sum;
    //        return true;
    //    }
    //}
}
//#include <CGAL/Polyhedron_3.h>
//#include <CGAL/boost/graph/graph_traits_Polyhedron_3.h>
//#include <CGAL\AABB_tree.h>
//#include <CGAL\AABB_traits.h>
//#include <CGAL/boost/graph/graph_traits_Polyhedron_3.h>
//#include <CGAL/AABB_face_graph_triangle_primitive.h>

//void main()
//{
//    Point p(1.0, 0.0, 0.0);
//    Point q(0.0, 1.0, 0.0);
//    Point r(0.0, 0.0, 1.0);
//    Point s(0.0, 0.0, 0.0);
//    Polyhedron polyhedron;
//    polyhedron.make_tetrahedron(p, q, r, s);
//    // constructs AABB tree and computes internal KD-tree 
//    // data structure to accelerate distance queries
//    Tree tree(polyhedron.facets_begin(), polyhedron.facets_end(), polyhedron);
//    tree.accelerate_distance_queries();
//    // query point
//    Point query(0.0, 0.0, 3.0);
//    // computes squared distance from query
//    FT sqd = tree.squared_distance(query);
//    std::cout << "squared distance: " << sqd << std::endl;
//    // computes closest point
//    Point closest = tree.closest_point(query);
//    std::cout << "closest point: " << closest << std::endl;
//    // computes closest point and primitive id
//    Point_and_primitive_id pp = tree.closest_point_and_primitive(query);
//    Point closest_point = pp.first;
//    Polyhedron::Face_handle f = pp.second; // closest primitive id
//    std::cout << "closest point: " << closest_point << std::endl;
//    std::cout << "closest triangle: ( "
//        << f->halfedge()->vertex()->point() << " , "
//        << f->halfedge()->next()->vertex()->point() << " , "
//        << f->halfedge()->next()->next()->vertex()->point()
//        << " )" << std::endl;
//}

//template <class K>
//inline typename K::FT distance(const CGAL::AABB_tree<CGAL::AABB_traits<K, CGAL::AABB_face_graph_triangle_primitive<CGAL::Polyhedron_3<K>>>>& tree, const CGAL::Point_3<K>& query)
//{
//    return tree.squared_distance(query);
//}