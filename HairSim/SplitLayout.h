#pragma once
#include <d3d11.h>
#include <DXUTcamera.h>
#include "XRwy_h.h"


namespace XRwy
{
	class SplitLayout
	{
		static const int MAX_SPLIT_LAYOUT_NUMBER = 4;
		typedef float Rectangle[4];
	public:
		SplitLayout(int n);
		~SplitLayout();

		int GetFrameNumber() const { return nFrame; }
		void BeginLayout();
		void EndLayout();
		void SetupFrame(int i, CModelViewerCamera* pCamera);

	private:
		int nFrame = 0;
		D3D11_VIEWPORT mainViewport;

	private:
		static const Rectangle RECTS[4][MAX_SPLIT_LAYOUT_NUMBER];
	};
}
