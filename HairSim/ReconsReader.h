#pragma once
#include <fstream>
#include <d3d11.h>
#include "macros.h"
#include "HairStructs.h"

namespace XRwy
{
	struct BlendHairGeometry:
		public HairGeometry
	{
		std::vector<XMFLOAT3> trans;
	};

	struct WeightItem
	{
		static const int MAX_GUIDANCE = 10;
	public:
		int		guideID[MAX_GUIDANCE]; // local ID !!
		float	weights[MAX_GUIDANCE];
	};

	struct SkinningInfo
	{
		BlendHairGeometry*			guidances = nullptr;
		HairGeometry*				restState = nullptr;

		std::vector<WeightItem>		weights;

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
		void rewind();
		void nextFrame();
		void jumpTo(int frameNo);

		HairColorsPerStrand* GetGroupColor() const;
		HairColorsPerStrand* GetGuidanceColor() const;

	private:
		void readHead(HairGeometry* geom);
		void readHead2(BlendHairGeometry* guidance);

		ExternPtr SkinningInfo*	hair = nullptr;

		std::ifstream		file;
		size_t				fileShortcut[6];
	};

}