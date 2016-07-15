#pragma once
#include <fstream>
#include "HairStructs.h"

namespace XRwy
{
	class ReconsReader
	{
	public:
		~ReconsReader();
		void LoadFile(const wchar_t* fileName);
		void PrepareForGuidanceDynamic(HairGeometry* guidance);
		void SetFrameID(size_t n);
		HairColorsPerStrand* GetGroupColor() const;
		HairColorsPerStrand* GetGuidanceColor() const;

	private:
		void ScanFile(SkinningScheme* skinning);

		size_t				nFrame = 0;
		size_t				curFrame = -1;
		bool				bDynamicState = false;
	};

	class SkinningEngine
	{
	public:
		SkinningScheme*		skinning = nullptr; // static part
		HairGeometry*		guidances = nullptr; // dynamic part
		ReconsReader*		reader = nullptr;

		~SkinningEngine();
		void LoadReconsFile(const wchar_t* fileName);
		void UpdateGuidance();
		void SetFrameID(size_t n);

	private:
		bool				sync = false;
		size_t				nFrame = 0;
		size_t				curFrame = -1;
	};

}