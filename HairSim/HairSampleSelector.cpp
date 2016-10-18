#include "precompiled.h"
#include <list>

#define CGAL_EIGEN3_ENABLED
#ifdef V
#undef V
#endif

#include <CGAL/Kd_tree.h>
#include <CGAL/Fuzzy_sphere.h>
#include <CGAL/Search_traits_3.h>
#include "CGALKernel.h"
#include "xlogger.h"
#include "HairSampleSelector.h"

namespace XRwy
{
	namespace
	{
		typedef CGAL::FloatEpick K;

		class KDSearchPoint :
			public K::Point_3
		{
		public:
			KDSearchPoint() :K::Point_3(), id(-1) {}
			KDSearchPoint(float x, float y, float z, int i) :
				K::Point_3(x, y, z), id(i) {}

			int id;
		};

		typedef KDSearchPoint Point_3;

		class KDSearchTraits :
			public CGAL::Search_traits_3<K>
		{
		public:
			typedef KDSearchPoint Point_d;
		};

		typedef KDSearchTraits Traits;
		typedef CGAL::Kd_tree<Traits> Tree;
		typedef CGAL::Fuzzy_sphere<Traits> Fuzzy_sphere;

	}

	HairSampleSelector::HairSampleSelector(HairGeometry* hair, size_t sampleRate, size_t groupScale, size_t groupSeed)
	{
		nSampleRate = sampleRate;
		pHair = hair;
		
		Rechoosing(groupSeed, groupScale);
	}


	HairSampleSelector::~HairSampleSelector()
	{
	}

	void HairSampleSelector::FillInHairStructs(HairGeometry* geom)
	{
		geom->nStrand = idList.size();
		geom->particlePerStrand = pHair->particlePerStrand;
		geom->nParticle = geom->nStrand * geom->particlePerStrand;
	}

	void HairSampleSelector::Rechoosing(size_t groupSeed, size_t groupScale)
	{
		std::vector<int> initList;
		nGroupScale = groupScale;
		nGroupSeed = groupSeed;
		if (nGroupScale && nGroupScale < pHair->nStrand)
		{
			int factor = pHair->particlePerStrand;
			std::vector<Point_3> pts;
			pts.reserve(pHair->nStrand);
			for (int i = 0; i < pHair->nStrand; i++)
			{
				auto ptr = pHair->position + i*factor;
				pts.emplace_back(ptr->x, ptr->y, ptr->z, i);
			}

			Tree tree(pts.begin(), pts.end());

			std::list<Point_3> output;
			const int low = nGroupScale * 0.9f, high = nGroupScale * 1.1f;
			float lr = 0, hr = 4.0f, r;

			do
			{
				r = (lr + hr) / 2.0f;
				output.clear();
				Fuzzy_sphere fs(pts[nGroupSeed], r, 0);
				tree.search(std::back_inserter(output), fs);

				if (output.size() > high)
					hr = r;
				else if (output.size() < low)
					lr = r;
				else break;
			} while (true);

			XLOG_INFO << "Select " << output.size() << " strands with radius " << r;
			for (auto& itr : output)
				initList.push_back(itr.id);
		}
		else
		{
			for (int i = 0; i < pHair->nStrand; i++)
				initList.push_back(i);
		}

		for (auto id : initList)
		{
			if (id % nSampleRate == 0)
				idList.push_back(id);
		}
	}

	void HairSampleSelector::ResetIterator()
	{
		iteratorPtr = 0;
	}

	int HairSampleSelector::GetNextId()
	{
		if (iteratorPtr == idList.size())
			return -1;
		return idList[iteratorPtr++];
	}
}