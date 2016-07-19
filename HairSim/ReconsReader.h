#pragma once
#include <fstream>
#include <d3d11.h>
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
		void ScanFile(SkinningStaticInfo* skinning);

		size_t				nFrame = 0;
		size_t				curFrame = -1;
		bool				bDynamicState = false;
	};

	class SkinningInfo
	{
	public:
		SkinningStaticInfo*	skinning = nullptr; // static part
		HairGeometry*		guidances = nullptr; // dynamic part
		ReconsReader*		reader = nullptr;

		~SkinningInfo();
		void LoadReconsFile(const wchar_t* fileName);
		void UpdateGuidance();
		void SetFrameID(size_t n);

		ID3D11Buffer* CreateGuidanceStructuredBuffer(ID3D11ShaderResourceView** ppRsv) const;
		ID3D11Buffer* CreateSkinningVertexBuffer() const;

	private:
		bool				sync = false;
		size_t				nFrame = 0;
		size_t				curFrame = -1;
	};


}