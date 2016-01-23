#include "ADFCollisionObject.h"
#include "wrMacro.h"
#include <fstream>
#include "wrLogger.h"
#include "linmath.h"
#include "ADFOctree.h"

namespace
{
#define ADF_SUFFIXW L".adf"

    template <class K, class T>
    T simplex3d_interpolation(CGAL::Point_3<K>* cell, T* vals, const CGAL::Point_3<K>& p)
    {
        float vol[4];
        vol[0] = CGAL::volume(cell[1], cell[3], cell[2], p);
        vol[1] = CGAL::volume(cell[2], cell[3], cell[0], p);
        vol[2] = CGAL::volume(cell[0], cell[3], cell[1], p);
        vol[3] = CGAL::volume(cell[0], cell[1], cell[2], p);

        float sum = 0.f;
        float numer = 0.0f;
        for (size_t i = 0; i < 4; i++)
        {
            sum += vol[i];
            numer += vol[i] * vals[i];
        }
        return numer / sum;
    }

    template <class K, class T>
    T simplex2d_interpolation(CGAL::Point_3<K>* v, T* vals, const CGAL::Point_3<K>& p)
    {
        CGAL::Vector_3<K> normal = CGAL::normal(v[0], v[1], v[2]);

        float ratio[3]; // not real area, but area ratio
        ratio[0] = CGAL::determinant(v[1] - p, v[2] - p, normal);
        ratio[1] = CGAL::determinant(v[2] - p, v[0] - p, normal);
        ratio[2] = CGAL::determinant(v[0] - p, v[1] - p, normal);

        float sum = 0.f;
        float numer = 0.0f;
        for (size_t i = 0; i < 4; i++)
        {
            sum += ratio[i];
            numer += ratio[i] * vals[i];
        }
        return numer / sum;
    }

    template <class K, class T>
    T simplex2d_interpolation(CGAL::Point_3<K>* v, T* vals, const CGAL::Point_3<K>& p, const CGAL::Vector_3<K>& normal)
    {
        float ratio[3]; // not real area, but area ratio
        ratio[0] = CGAL::determinant(v[1] - p, v[2] - p, normal);
        ratio[1] = CGAL::determinant(v[2] - p, v[0] - p, normal);
        ratio[2] = CGAL::determinant(v[0] - p, v[1] - p, normal);

        float sum = 0.f;
        float numer = 0.0f;
        for (size_t i = 0; i < 4; i++)
        {
            sum += ratio[i];
            numer += ratio[i] * vals[i];
        }
        return numer / sum;
    }
}


namespace WR
{
    ADFCollisionObject::ADFCollisionObject(const wchar_t* fileName)
    {
        load_model(fileName);
    }


    void ADFCollisionObject::release()
    {
        SAFE_DELETE(pDt);
    }

    float ADFCollisionObject::fake_extrapolate(const Point_3& p, const Point_3 v[], size_t infId, Dt::Cell_handle ch) const
    {
        int indices[3];
        float dists[3];
        Point_3 validv[3];
        for (size_t i = 0; i < 3; i++)
        {
            indices[i] = (infId + 1 + i) % 4;
            dists[i] = ch->vertex(indices[i])->info().minDist;
            validv[i] = v[indices[i]];
        }

        CGAL::Vector_3<K> normal = CGAL::normal(validv[0], validv[1], validv[2]);

        float squared_dist_from_plane = normal * (p - validv[0]);
        squared_dist_from_plane *= squared_dist_from_plane / normal.squared_length();

        //if (CGAL::coplanar(validv[0], validv[1], validv[2], p)) // too strict
        if (squared_dist_from_plane < 1e-15)
            return simplex2d_interpolation(validv, dists, p, normal);
        else 
            return std::numeric_limits<float>::max();
    }

    float ADFCollisionObject::extrapolate(const Point_3& p, const Point_3 v[], size_t infId, Dt::Cell_handle ch) const
    {
        int indices[3];
        float dists[3];
        Point_3 validv[3];
        for (size_t i = 0; i < 3; i++)
        {
            indices[i] = (infId + 1 + i) % 4;
            dists[i] = ch->vertex(indices[i])->info().minDist;
            validv[i] = v[indices[i]];
        }

        TriangleWithInfos<K> tri(validv[0], validv[1], validv[2]);
        tri.initInfo();
        tri.computeInfo(p);
        WRG::PointTriangleDistResult<K::FT> res;
        WRG::squaredDistance(p, tri, tri.infos, res);

        Point_3 touch = tri.vertex(0) + res.s * tri.E0 + res.t * tri.E1;
        float dist = simplex2d_interpolation(validv, dists, touch);

        dist += sqrt(res.dist);
        return dist;
    }

    float ADFCollisionObject::query_distance_template(const Point_3& p, ExtrapolateFunc func) const
    {
        assert(pDt);
        assert(pDt->number_of_cells());

        Dt::Cell_handle ch = pDt->locate(p);

        Point_3 v[4];
        bool isInf = false;
        int infId = -1;
        for (size_t i = 0; i < 4; i++)
        {
            if (ch->vertex(i) == pDt->infinite_vertex())
            {
                assert(infId == -1);
                isInf = true;
                infId = i;
            }
            else v[i] = ch->vertex(i)->point();
        }

        if (isInf)
        {
            return (this->*func)(p, v, infId, ch);
        }
        else
        {
            assert(CGAL::volume(v[0], v[1], v[2], v[3]) > 0);
            float vals[4];
            for (size_t i = 0; i < 4; i++)
                vals[i] = ch->vertex(i)->info().minDist;
            return simplex3d_interpolation(v, vals, p);
        }
    }


    float ADFCollisionObject::query_squared_distance(const Point_3& p) const
    {
        assert(0);
        return 0.f;
    }


    bool ADFCollisionObject::exceed_threshhold(const Point_3& p, float thresh) const
    {
        return false;

    }


    bool ADFCollisionObject::position_correlation(const Point_3& p, Point_3* pCorrect, float thresh) const
    {
        return true;
    }


    bool ADFCollisionObject::save_model(const wchar_t* fileName) const
    {
        assert(pDt);

        std::wstring fullName(fileName);
        size_t sz;
        const wchar_t *pch;
        ADD_SUFFIX_IF_NECESSARYW(fileName, ADF_SUFFIXW, fullName);

        std::ofstream file(fullName);
        assert(file);

        file.precision(10);
        file << m_bbox << std::endl;

        Dt::Finite_vertices_iterator v_end = pDt->finite_vertices_end();
        for (auto itr = pDt->finite_vertices_begin(); itr != v_end; itr++)
        {
            file << itr->point() << '\t';
            file << itr->info().minDist << '\t';
            file << itr->info().gradient << std::endl;
        }
        return true;
    }

    bool ADFCollisionObject::load_model(const wchar_t* fileName)
    {
        release();

        std::wstring fullName;
        size_t sz;
        const wchar_t *pch;
        ADD_SUFFIX_IF_NECESSARYW(fileName, ADF_SUFFIXW, fullName);

        WR_LOG_INFO << "load Model: " << fullName;

        std::ifstream file(fullName);
        assert(file);

        file >> m_bbox;

        Dt::Vertex_handle vh;
        std::vector<VInfo> infos;

        Point_3 p;
        VInfo info;
        pDt = new Dt;
        while (!file.eof())
        {
            file >> p;
            file >> info.minDist;
            file >> info.gradient;
            vh = pDt->insert(p);
            vh->info() = info;
        }

        WR_LOG_INFO << "load succeded! " << fullName;
        return true;
    }


    void ADFCollisionObject::compute_gradient()
    {
        const float k = 0.4f;

        Vector_3 step[3];
        float coef = pow(0.5f, m_max_level) * k;
        step[0] = Vector_3(coef * (m_bbox.xmax() - m_bbox.xmin()), 0, 0);
        step[1] = Vector_3(0, coef * (m_bbox.ymax() - m_bbox.ymin()), 0);
        step[2] = Vector_3(0, 0, coef * (m_bbox.zmax() - m_bbox.zmin()));

        vec3 v;
        for (auto vItr = pDt->finite_vertices_begin(); vItr != pDt->finite_vertices_end(); vItr++)
        {
            auto &pos = vItr->point();
            for (size_t i = 0; i < 3; i++)
            {
                auto dist1 = query_distance_with_fake_extrapolation(pos + step[i]);
                auto dist2 = query_distance_with_fake_extrapolation(pos - step[i]);

                assert(dist1 < 1e6 || dist2 < 1e6);

                if (dist1 > 1e6)
                    v[i] = (vItr->info().minDist - dist2) / step[i][i];
                else if (dist2 > 1e6)
                    v[i] = (dist1 - vItr->info().minDist) / step[i][i];
                else
                    v[i] = (dist1 - dist2) / (2 * step[i][i]);

                assert(!std::isinf(v[i]));
            }
            vItr->info().gradient = Vector_3(v[0], v[1], v[2]);
        }
    }


}