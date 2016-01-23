#pragma once
#include "wrLogger.h"
#include <CGAL\Point_3.h>
#include <CGAL\Triangle_3.h>
#include <CGAL\Vector_3.h>
#include <CGAL\Iso_cuboid_3.h>
#include <CGAL\Aff_transformation_3.h>
#include <fstream>
#include <CGAL\IO\Polyhedron_iostream.h>

namespace WRG
{


    template <class Polyhedron_3>
    Polyhedron_3* readFile(const char* fileName)
    {
        Polyhedron_3 *P = new Polyhedron_3;
        std::ifstream f(fileName);
        f >> (*P);
        f.close();
        WR_LOG_INFO << "Read off file: " << fileName << " nVertices: " << P->size_of_vertices();
        return P;
    }

    template <class Polyhedron_3>
    Polyhedron_3* readFile(const wchar_t* fileName)
    {
        Polyhedron_3 *P = new Polyhedron_3;
        std::ifstream f(fileName);
        f >> (*P);
        f.close();
        WR_LOG_INFO << "Read off file: " << fileName << " nVertices: " << P->size_of_vertices();
        return P;
    }

    template <class R_, class FT_>
    static inline void enlarge(CGAL::Iso_cuboid_3<R_>& bbox, FT_ offset)
    {
        bbox = CGAL::Iso_cuboid_3<R_>(bbox.min() + CGAL::Vector_3<R_>(-offset, -offset, -offset), 
            bbox.max() + CGAL::Vector_3<R_>(offset, offset, offset));
    }

    template < class K >
    inline typename K::FT squaredArea(const CGAL::Point_3<K> &p, const CGAL::Point_3<K> &q, const CGAL::Point_3<K> &r)
    {
        auto e1 = q - p;
        auto e2 = r - p;

        return CGAL::cross_product(e1, e2).squared_length();
    }

    template <class T>
    struct PointTriangleDistInfo
    {
        T a, b, c, d, e, f;
    };

    template <class T>
    struct PointTriangleDistResult
    {
        T s, t, dist;
        int type;
    };


    template <class T>
    static inline T _calcDist(const T& s, const T& t, const PointTriangleDistInfo<T>& infos)
    {
        return infos.a * s * s + 2.0 * infos.b * s * t +
            infos.c * t * t + 2.0 * infos.d * s + 2.0 * infos.e * t + infos.f;
    }

    template <class T>
    static inline void _e1(PointTriangleDistResult<T>& res, const PointTriangleDistInfo<T>& infos)
    {
        T numer = infos.c + infos.e - infos.b - infos.d;
        if (numer <= 0)
        {
            res.s = 0;
            res.t = 1;
            res.type = 2;
        }
        else
        {
            T denom = infos.a - 2.0 * infos.b + infos.c;
            if (numer >= denom)
            {
                res.s = 1;
                res.t = 0;
                res.type = 6;
            }
            else
            {
                res.s = numer / denom;
                res.t = 1 - res.s;
                res.type = 1;
            }
        }
    }

    template <class T>
    static inline void _e3(PointTriangleDistResult<T>& res, const PointTriangleDistInfo<T>& infos)
    {
        res.s = 0;
        if (infos.e >= 0)
        {
            res.t = 0;
            res.type = 4;
        }
        else if (-infos.e >= infos.c)
        {
            res.t = 1;
            res.type = 2;
        }
        else
        {
            res.t = -infos.e / infos.c;
            res.type = 3;
        }
    }

    template <class T>
    static inline void _e5(PointTriangleDistResult<T>& res, const PointTriangleDistInfo<T>& infos)
    {
        res.t = 0;
        if (infos.d >= 0)
        {
            res.s = 0;
            res.type = 4;
        }
        else if (-infos.d >= infos.a)
        {
            res.s = 1;
            res.type = 6;
        }
        else
        {
            res.s = -infos.d / infos.a;
            res.type = 5;
        }
    }


    // 这里的返回结果不完全是图中标示的意思
    // 0表示在内部，135表示在边上，246表示在顶点
    template <class K, class T>
    static inline void squaredDistance(const CGAL::Point_3<K> & plane,
        const CGAL::Triangle_3<K> & seg, const PointTriangleDistInfo<T>& infos,
        PointTriangleDistResult<T>& res)
    {
        T det = fabs(infos.a * infos.c - infos.b * infos.b);
        res.s = infos.b * infos.e - infos.c * infos.d;
        res.t = infos.b *infos.d - infos.a *infos.e;

        if (res.s + res.t <= det)
        {
            if (res.s < 0)
            {
                if (res.t < 0)
                {
                    // region 4
                    if (infos.e < 0)
                        _e3(res, infos);
                    else
                        _e5(res, infos);
                }
                else
                {
                    // region 3
                    _e3(res, infos);
                }
            }
            else if (res.t < 0)
            {
                // region 5
                _e5(res, infos);
            }
            else
            {
                // region 0
                T invDet = 1 / det;
                res.s *= invDet;
                res.t *= invDet;
                res.type = 0;
            }
        }
        else
        {
            if (res.s < 0)
            {
                // region 2
                T tmp0 = infos.b + infos.d;
                T tmp1 = infos.c + infos.e;
                if (tmp1 > tmp0)
                    _e1(res, infos);
                else
                    _e3(res, infos);
            }
            else if (res.t < 0)
            {
                // region 6
                T tmp0 = infos.b + infos.e;
                T tmp1 = infos.a + infos.d;
                if (tmp1 > tmp0)
                    _e1(res, infos);
                else
                    _e5(res, infos);
            }
            else
            {
                // region 1
                _e1(res, infos);
            }
        }
        res.dist = _calcDist(res.s, res.t, infos);
    }
}