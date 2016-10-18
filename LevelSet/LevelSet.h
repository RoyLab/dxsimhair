#pragma once
#ifdef V
#undef V
#endif
#include <CGAL\Polyhedron_3.h>
#include "ADFOctree.h"
#include "wrGeo.h"
#include "GroupPBD.h"

#ifdef WR_EXPORTS
#define WR_API __declspec(dllexport)
#else
#define WR_API __declspec(dllimport)
#endif

namespace WR
{
    extern "C" WR_API ICollisionObject* createCollisionObject(Polyhedron_3_FaceWithId& poly);
	extern "C" WR_API ICollisionObject* createCollisionObject2(const wchar_t* fileName);
	extern "C" WR_API ICollisionObject* loadCollisionObject(const wchar_t* fileName);
	extern "C" WR_API void runLevelSetBenchMark(const wchar_t* fileName);

	extern "C" WR_API ICollisionObject* CreateGridCollisionObject(const char* fileName);
}

namespace XRwy
{
	extern "C" WR_API IHairCorrection* CreateHairCorrectionObject();

}