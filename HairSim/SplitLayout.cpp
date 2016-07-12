#include <DXUT.h>
#include "SplitLayout.h"

namespace XRwy
{
	const SplitLayout::Rectangle SplitLayout::RECTS[4][MAX_SPLIT_LAYOUT_NUMBER] =
	{
		{{0, 0, 1, 1}, },
		{{ 0, 0, 0.5, 1 },{ 0.5, 0, 0.5, 1 },},
		{{ 0, 0, 0.5, 0.5 },{ 0.5, 0, 0.5, 0.5 }, {0, 0.5, 1, 0.5}, },
		{{ 0, 0, 0.5, 0.5 },{ 0.5, 0, 0.5, 0.5 }, {0, 0.5, 0.5, 0.5}, { 0.5, 0.5, 0.5, 0.5 } },
	};

	SplitLayout::SplitLayout(int n)
	{
		nFrame = n;
	}

	SplitLayout::~SplitLayout()
	{
	}

	void SplitLayout::BeginLayout()
	{
		UINT nVP = 1;
		D3D11_VIEWPORT pVP, pVPl;
		DXUTGetD3D11DeviceContext()->RSGetViewports(&nVP, &mainViewport);
	}

	void SplitLayout::EndLayout()
	{
		DXUTGetD3D11DeviceContext()->RSSetViewports(1, &mainViewport);
	}

	void SplitLayout::SetupFrame(int i, CModelViewerCamera* pCamera)
	{
		int select = nFrame - 1;

		D3D11_VIEWPORT vp;
		CopyMemory(&vp, &mainViewport, sizeof(D3D11_VIEWPORT));
		vp.TopLeftX = static_cast<int>(mainViewport.Width * RECTS[select][i][0]);
		vp.TopLeftY = static_cast<int>(mainViewport.Height * RECTS[select][i][1]);
		vp.Width = static_cast<int>(mainViewport.Width * RECTS[select][i][2]);
		vp.Height = static_cast<int>(mainViewport.Height * RECTS[select][i][3]);

		DXUTGetD3D11DeviceContext()->RSSetViewports(1, &vp);

		float ratio = mainViewport.Width / (float)mainViewport.Height;
		pCamera->SetProjParams(DirectX::XM_PI / 4, RECTS[select][i][2] / RECTS[select][i][3] * ratio, 0.1f, 1000.0f);
	}
}