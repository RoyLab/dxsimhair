#pragma once
#include <CGAL\Polyhedron_3.h>
#include "ADFOctree.h"
#include "wrGeo.h"

namespace WR
{
    ICollisionObject* createCollisionObject(Polyhedron_3_FaceWithId& poly);
    ICollisionObject* createCollisionObject(const wchar_t* fileName);
    ICollisionObject* loadCollisionObject(const wchar_t* fileName);
    void runLevelSetBenchMark(const wchar_t* fileName);

}