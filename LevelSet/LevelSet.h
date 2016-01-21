#pragma once
#include "LSOctree.h"
#include "ICollisionObject.h"
#include <CGAL\Polyhedron_3.h>

namespace WR
{
    template <class _K>
    ICollisionObject* createCollisionObject(const CGAL::Polyhedron_3<_K>& poly)
    {
        return createLSOctree(poly);
    }


    template <class _K>
    ISOctree* createLSOctree(const CGAL::Polyhedron_3<_K>& poly)
    {
        ISOctree* pTree = new ISOctree;
        return pTree;
    }

    template <class _K>
    ISOctree* loadLSOCollisionObject(const wchar_t fileName[])
    {
        ISOctree* pTree = new ISOctree;
        return pTree;
    }

    template <class _K>
    ISOctree* saveLSOctree(const wchar_t fileName[])
    {
        ISOctree* pTree = new ISOctree;
        return pTree;
    }
}