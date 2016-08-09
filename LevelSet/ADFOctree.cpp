#include "UnitTest.h"
#include "ADFOctree.h"
#include "macros.h"
#include "wrLogger.h"
#include "wrGeo.h"
#include "linmath.h"
#include "ADFCollisionObject.h"
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
    float ADFOctree::m_box_enlarge_size = 0.1f;

    ADFOctree::ADFOctree()
    {
    }


    ADFOctree::~ADFOctree()
    {
        release();
    }

    ADFCollisionObject* ADFOctree::releaseAndCreateCollisionObject()
    {
        ADFCollisionObject* pCO = new ADFCollisionObject(dt, box, nMaxLevel, m_box_enlarge_size, pModel);
        releaseExceptDt();
        dt = nullptr;
        return pCO;
    }


    bool ADFOctree::construct(Polyhedron_3& geom, size_t maxLvl)
    {
		pModel = &geom;

        release();
        nMaxLevel = maxLvl;
        dt = new Dt;

        WR_LOG_INFO << "constructing...";

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

		auto* tester = CGAL::Ext::createDistanceTester2<Polyhedron_3_FaceWithId, K>(*pModel);
		constructChildren(pRoot, tester);
		delete tester;
        WR_LOG_INFO << "constructed: depth, " << maxLvl;
        return true;
    }

    void ADFOctree::constructChildren(Node* root, CGAL::Ext::DistanceTester2<Polyhedron_3_FaceWithId, K>* tester)
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
                constructChildren(child, tester);
            else computeMinDistance(child, tester);

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

    float ADFOctree::query_distance(const Point_3& p) const
    {
        int type = -1;
        size_t triIdx = 0;
        Vector_3 diff;

        float sd = minSquaredDist(p, pRoot->eList.cbegin(), pRoot->eList.cend(), &diff, &triIdx, &type);
        int sign = determineSign(type, p, diff, triIdx);

        return sign * sqrt(sd);
    }


    void ADFOctree::computeMinDistance(Node* node, CGAL::Ext::DistanceTester2<Polyhedron_3_FaceWithId, K>* tester)
    {
        static int count = 0;
        for (size_t i = 0; i < 8; i++)
        {
            auto& vh = node->vertices[i];
            auto& dist = vh->info().minDist;
            if (dist < 1.0e8) continue;

            //int type = -1;
            //size_t triIdx = 0;
            //Node* curNode = node;
            //Vector_3 diff;
            //float tmpSquaredDist, distLimit;

            //while (true)
            //{
            //    if (!curNode->eList.empty())
            //    {
            //        tmpSquaredDist = minSquaredDist(vh->point(), curNode->eList.cbegin(), curNode->eList.cend(), &diff, &triIdx, &type);
            //        distLimit = minDist(curNode->triple, vh->point());

            //        if (tmpSquaredDist < distLimit * distLimit || !curNode->pParent)
            //        {
            //            dist = sqrt(tmpSquaredDist);
            //            break;
            //        }
            //    }
            //    curNode = curNode->pParent;
            //}

            //int sign = determineSign(type, vh->point(), diff, triIdx);
            //dist *= sign;
			dist = tester->query_signed_distance(vh->point());
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
        if (diff * triList[triIdx].normal > 0.0) return 1;
        else return -1;
    }

    int ADFOctree::detSignOnEdge(const Point_3& p, const Vector_3& diff, size_t triIdx, int seq) const
    {
        auto edge = triList[triIdx].fh->facet_begin();
        for (size_t i = 0; i < seq; i++, edge++);

        Vector_3 normal = (triList[edge->opposite()->facet()->idx].normal + triList[triIdx].normal) / 2.0;
        if (diff * normal > 0.0) return 1;
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
            auto newDir = v_circle->opposite()->vertex()->point() - vh->point();
            dir = dir + newDir;
        }
        if (diff * dir < 0.0) return 1;
        else return -1;
    }


    // begin != end !!!
    template <class Iterator>
    float ADFOctree::minSquaredDist(const Point_3& p, Iterator begin, Iterator end, Vector_3* diff, size_t* pTriIdx, int* pType) const
    {
        assert(begin != end);

        float dist = std::numeric_limits<float>::max();
        size_t triIdx = 0;
        int type = 0;
        float s;
        float t;

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

    float ADFOctree::minDist(const Cube_3& bbox, const Point_3& p)
    {
        float dist = std::numeric_limits<float>::max();

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

        WRG::enlarge(bbox, m_box_enlarge_size); // 不希望贴的太紧
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

    /////////////////////////////////////////外推法/////////////////////////////////////////////
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
}