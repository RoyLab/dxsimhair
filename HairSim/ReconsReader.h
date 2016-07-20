#pragma once
#include <fstream>
#include <d3d11.h>
#include "HairStructs.h"

namespace XRwy
{
	struct BlendHairGeometry:
		public HairGeometry
	{
		std::vector<XMFLOAT3> trans;
	};

	class ReconsReader:
		public HairLoader
	{
	public:
		~ReconsReader();

		bool loadFile(const char* fileName, HairGeometry * geom); // different meaning, geom is first state.
		void rewind();
		void nextFrame();
		void jumpTo(int frameNo);

		HairColorsPerStrand* GetGroupColor() const;
		HairColorsPerStrand* GetGuidanceColor() const;

	private:
		BlendHairGeometry*	hair = nullptr;
		size_t				nFrame = 0;
		size_t				curFrame = -1;
		bool				bDynamicState = false;
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


}