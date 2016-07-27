#pragma once
#include <fstream>
#include <map>
#include <d3d11.h>
#include "macros.h"
#include "HairStructs.h"

namespace XRwy
{
	struct BlendHairGeometry:
		public HairGeometry
	{
		std::vector<XMFLOAT3> trans;

		void allocMemory()
		{
			HairGeometry::allocMemory();
			trans.resize(nParticle);
		}

		//~BlendHairGeometry()
		//{
		//	for (auto& ptr : trans)
		//		delete ptr;
		//}
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

		HairColorsPerStrand* GetGroupColor() const { return nullptr; }
		HairColorsPerStrand* GetGuidanceColor() const { return nullptr; }

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