#pragma once
#include <DirectXMath.h>
#include <vector>
#include "macros.h"

namespace XRwy
{
	using DirectX::XMFLOAT3;
	using DirectX::XMFLOAT4X4;

	class HairAnimationLoader;
	class HairManager;
	struct FrameContent;
	class SkinningInfo;
	class ReconsReader;
	class GroupPBD;

	struct HairGeometry
	{
		XMFLOAT4X4			worldMatrix;
		float               rigidTrans[16];
		XMFLOAT3*           position = nullptr;
		XMFLOAT3*           direction = nullptr;

		size_t              nParticle;
		size_t              nStrand;
		size_t              particlePerStrand;


		~HairGeometry()
		{
			SAFE_DELETE_ARRAY(position);
			SAFE_DELETE_ARRAY(direction);
		}
	};

	struct InterpItem
	{
		static const int MAX_GUIDANCE = 10;
	public:
		int		guideID[MAX_GUIDANCE]; // local ID !!
		float	weights[MAX_GUIDANCE];
	};

	struct SkinningStaticInfo
	{
		HairGeometry* restState = nullptr;
		std::vector<InterpItem> items;

		~SkinningStaticInfo() { SAFE_DELETE_ARRAY(restState); }
	};

	// all interfaces are sync method
	class HairLoader
	{
	public:
		virtual bool loadFile(const char* fileName, HairGeometry * geom) = 0;
		virtual void rewind() {}
		virtual void nextFrame() {}
		virtual void jumpTo(int frameNo) {}

		virtual ~HairLoader() {}
	};


	///////////////////////////////////////////////////////
	///////////////color interface/////////////////////////
	///////////////////////////////////////////////////////

	struct HairColorsPerStrand
	{
		size_t      nStrand;
		XMFLOAT3*   color = nullptr;

		~HairColorsPerStrand()
		{
			SAFE_DELETE_ARRAY(color);
		}
	};

	// Responsible for the storage management of color array
	class IHairColorGenerator
	{
	public:
		virtual ~IHairColorGenerator() {}
		virtual const XMFLOAT3* GetColorArray() const = 0;
	};

	class AssignedColorHair :
		public IHairColorGenerator
	{
	public:
		AssignedColorHair(size_t n, HairColorsPerStrand* hair)
		{
			number = n;
			colorBuffer = new XMFLOAT3[n];
			size_t factor = number / hair->nStrand;
			for (size_t i = 0; i < hair->nStrand; i++)
			{
				for (size_t j = 0; j < factor; j++)
					colorBuffer[i * factor + j] = hair->color[i];
			}
		}

		~AssignedColorHair() { SAFE_DELETE_ARRAY(colorBuffer); }
		const XMFLOAT3* GetColorArray() const { return colorBuffer; }

	private:
		XMFLOAT3*   colorBuffer = nullptr;
		size_t      number = -1;
	};
}