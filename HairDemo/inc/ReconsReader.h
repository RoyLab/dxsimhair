#pragma once

#include <fstream>
#include <map>

#include <macros.h>
#include <cy\cyPoint.h>

#include "xhair.h"
#include "HairStructs.h"

namespace xhair
{
	struct BlendHairGeometry:
		public HairGeometry
	{
		std::vector<Point3> trans;

		void allocMemory()
		{
			HairGeometry::allocMemory();
			trans.resize(nParticle);
		}
	};

	struct WeightItem
	{
		static const int MAX_GUIDANCE = 10;
	public:
		int		guideID[MAX_GUIDANCE]; // local ID !!
		float	weights[MAX_GUIDANCE];
		int		n;
	};

	struct SkinningInfo
	{
		BlendHairGeometry*			guidances = nullptr;
		HairGeometry*				restState = nullptr;

		std::vector<int>						groupId;
		std::vector<std::vector<int>*>			neighbor;
		char									anim2[128];


		std::vector<WeightItem>		weights;
		std::map<int, int>			global2local;
		size_t						nFrame = 0;
		size_t						curFrame = -1;
	};

	class ReconsReader
	{
		COMMON_PROPERTY(size_t, nFrame);
		COMMON_PROPERTY(size_t, curFrame);
	public:
		ReconsReader();
		~ReconsReader();

		// not a hairloader since it load other things like weights
		bool loadFile(const char* fileName, SkinningInfo* skinHair);

		// do not interpolate here!
		void rewind();
		void nextFrame();
		void jumpTo(int frameNo);

		HairColorsPerStrand* GetGroupColor() const;
		HairColorsPerStrand* GetGuidanceColor() const;

	private:
		static const int NUM_SECTION = 6;

		void readHead(HairGeometry* geom);
		void readHead2(BlendHairGeometry* guidance);
		void readShortcut();
		void setupGuideHair();
		void setupRestState();
		void setupWeights();
		void setupGroupSection();
		void setupNeighbSection();
		void setupInplSection();

		ExternPtr SkinningInfo*	hair = nullptr;

		std::ifstream		file;
		size_t				fileShortcut[NUM_SECTION];
	};

}