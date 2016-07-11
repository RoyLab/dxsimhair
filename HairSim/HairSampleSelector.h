#pragma once
#include <vector>
#include "macros.h"
#include "XRwy_h.h"
#include "HairStructs.h"

namespace XRwy
{
	class HairSampleSelector
	{
	public:
		HairSampleSelector(HairGeometry* hair, size_t sampleRate, size_t groupScale = 0, size_t groupSeed = 0);
		~HairSampleSelector();

		void FillInHairStructs(HairGeometry* geom);
		void Rechoosing(size_t groupSeed, size_t groupScale = 0);
		void ResetIterator();
		int GetNextId();
		int GetNumberOfStrand() const { return idList.size(); }

	private:
		size_t nSampleRate, nGroupScale, nGroupSeed;
		std::vector<int> idList;

		int iteratorPtr = 0;

		ExternPtr HairGeometry* pHair = nullptr;
	};
}
