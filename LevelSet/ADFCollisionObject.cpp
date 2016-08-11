#include "UnitTest.h"// must be the first

#include "ADFCollisionObject.h"
#include "macros.h"
#include <fstream>
#include "wrLogger.h"
#include "wrMath.h"
#include "ADFOctree.h"
#include "LevelSet.h"
#include "ConfigReader.h"

namespace
{
#define ADF_SUFFIXW L".adf"
#define MAX_INTERATION 20
#define CORRECTION_TOL 3e-4f

    template <class K, class T>
    void simplex3d_interpolation(CGAL::Point_3<K>* cell, T* vals, const CGAL::Point_3<K>& p, T& numer)
    {
        float vol[4];
        vol[0] = CGAL::volume(cell[1], cell[3], cell[2], p);
        vol[1] = CGAL::volume(cell[2], cell[3], cell[0], p);
        vol[2] = CGAL::volume(cell[0], cell[3], cell[1], p);
        vol[3] = CGAL::volume(cell[0], cell[1], cell[2], p);

        float sum = 0.f;
        for (size_t i = 0; i < 4; i++)
        {
            sum += vol[i];
            numer = numer + vol[i] * vals[i];
        }

        if (std::isnan(sum))
        {
            WR_LOG_DEBUG << std::endl << vals[0] << ", " << vals[1]
                << ", " << vals[2] << ", " << vals[3]
                << ", " << numer << ", " << sum;
        }

        numer = numer / sum;
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

    size_t g_count = 0;
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

        float result = 0.0;
        if (squared_dist_from_plane < 1e-10)
            result = simplex2d_interpolation(validv, dists, p, normal);
        else 
            result = std::numeric_limits<float>::max();
        return result;
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

            float result = 0.f;
            simplex3d_interpolation(v, vals, p, result);
            return result;
        }
    }


    float ADFCollisionObject::query_squared_distance(const Point_3& p) const
    {
        UNIMPLEMENTED_DECLARATION;
        return 0.f;
    }


    bool ADFCollisionObject::exceed_threshhold(const Point_3& p, float thresh) const
    {
        UNIMPLEMENTED_DECLARATION;
        return false;
    }


    bool ADFCollisionObject::position_correlation(const Point_3& p, Point_3* pCorrect, float thresh) const
    {
        assert(pDt);
        assert(pDt->number_of_cells());

        if (CGAL::ON_UNBOUNDED_SIDE == m_bbox.bounded_side(p))
            return false;

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
            return false;
        }
        else
        {
            assert(CGAL::volume(v[0], v[1], v[2], v[3]) > 0);
            std::vector<float> vals(4);
            for (size_t i = 0; i < 4; i++)
                vals[i] = ch->vertex(i)->info().minDist;

            if (*std::min_element(vals.cbegin(), vals.cend()) > thresh)
                return false;

            float cur_value = 0.f;
            simplex3d_interpolation(v, vals.data(), p, cur_value);

            if (cur_value > thresh)
            {
                return false;
            }
            else
            {
                if (!pCorrect) return true;

                std::vector<Vector_3> grads(4);
                for (size_t i = 0; i < 4; i++)
                    grads[i] = ch->vertex(i)->info().gradient;

                Point_3 curPos, newPos;
                g_count = 0;
                correct_position_by_gradient(p, curPos, v, grads.data(), cur_value, thresh);

                Dt::Cell_handle ch_hint = ch, ch_new;
                while (position_correlation_iteration(curPos, newPos, ch_new, ch_hint, thresh) && g_count < MAX_INTERATION)
                {
                    ch_hint = ch_new;
                    curPos = newPos;
                }
                *pCorrect = newPos;
                return true;
            }
        }
    }

    bool ADFCollisionObject::position_correlation_iteration(const Point_3& p, Point_3& newPos, Dt::Cell_handle chnew, Dt::Cell_handle chhint, float thresh) const
    {
       chnew = pDt->locate(p, chhint);

        std::vector<float> dist(4);
        std::vector<Point_3> pts(4);
        for (size_t i = 0; i < 4; i++)
        {
            pts[i] = chnew->vertex(i)->point();
            dist[i] = chnew->vertex(i)->info().minDist;
        }

        float cur_value = 0.f;
        simplex3d_interpolation(pts.data(), dist.data(), p, cur_value);

        if (cur_value > thresh && cur_value - thresh < CORRECTION_TOL) return false;

        std::vector<Vector_3> grads(4);
        for (size_t i = 0; i < 4; i++)
            grads[i] = chnew->vertex(i)->info().gradient;

        correct_position_by_gradient(p, newPos, pts.data(), grads.data(), cur_value, thresh + CORRECTION_TOL / 2.0f);

        return true;
    }

    void ADFCollisionObject::correct_position_by_gradient(const Point_3& p, Point_3& newPos, Point_3* pts, Vector_3* grads, float cur_value, float thresh) const
    {
        Vector_3 g(0, 0, 0);
        simplex3d_interpolation(pts, grads, p, g);

        float rawStep = cur_value - thresh;
        float step = sgn(rawStep) * std::min(m_max_step, std::abs(rawStep));
        newPos = p - g * step * 0.8f;

        //WR_LOG_DEBUG << "correlation iteration: " << g_count <<
        //    " dist: " << query_distance(newPos) << " step: " << step;
        g_count++;
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
        file << m_max_step << std::endl;
        file << m_bbox << std::endl;

        Dt::Finite_vertices_iterator v_end = pDt->finite_vertices_end();
        for (auto itr = pDt->finite_vertices_begin(); itr != v_end; itr++)
        {
            file << itr->point() << '\t';
            file << itr->info().minDist << '\t';
            file << itr->info().gradient << std::endl;
        }
        file.close();
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

        file >> m_max_step;
        file >> m_bbox;

        Dt::Vertex_handle vh;
        std::vector<VInfo> infos;

        Point_3 p;
        VInfo info;
        pDt = new Dt;

        size_t count = 0;
        while (!file.eof())
        {
            if (!file.good())
            {
                WR_LOG_ERROR << "fatal error! point " << count << ", " << p;
                return false;
            }

            file >> p;
            file >> info.minDist;
            file >> info.gradient;
            vh = pDt->insert(p);
            vh->info() = info;

            count++;
        }
        file.close();

        WR_LOG_INFO << "load succeded! " << fullName;
        return true;
    }


    void ADFCollisionObject::compute_gradient()
    {
        const float k = 0.005f;

        Vector_3 step[3];
        float coef = k;
        step[0] = Vector_3(coef * (m_bbox.xmax() - m_bbox.xmin()), 0, 0);
        step[1] = Vector_3(0, coef * (m_bbox.ymax() - m_bbox.ymin()), 0);
        step[2] = Vector_3(0, 0, coef * (m_bbox.zmax() - m_bbox.zmin()));

		ConfigReader reader("..\\config.ini");

		assert(pModel);
		auto* tester = CGAL::Ext::createDistanceTester<Polyhedron_3_FaceWithId, K>(*pModel);

        vec3 v;
        for (auto vItr = pDt->finite_vertices_begin(); vItr != pDt->finite_vertices_end(); vItr++)
        {
            auto &pos = vItr->point();
            for (size_t i = 0; i < 3; i++)
            {
                auto dist1 = tester->query_signed_distance(pos + step[i]);
                auto dist2 = tester->query_signed_distance(pos - step[i]);

                if (dist1 > 1e6 && dist2 > 1e6)
                {
                    WR_LOG_ERROR << "\ndual large distance, cannot interpolate. "
                        << pos << " direction: " << i;
                }

                if (dist1 > 1e6)
                    v[i] = (vItr->info().minDist - dist2) / step[i][i];
                else if (dist2 > 1e6)
                    v[i] = (dist1 - vItr->info().minDist) / step[i][i];
                else
                    v[i] = (dist1 - dist2) / (2 * step[i][i]);

                if (std::isnan(v[i]))
                {
                    WR_LOG_ERROR << "\ninvalid gradient. "
                        << pos << " direction: " << i 
                        << ", " << dist1 << ", " << dist2;
                }
            }
            Vector_3 grad(v[0], v[1], v[2]);
            grad = grad / sqrt(grad.squared_length());
            vItr->info().gradient = grad;
        }

		delete tester;
    }


}