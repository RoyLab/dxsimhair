#pragma once
#include "wrHair.h"
#include <d3d11.h>

class wrHairRenderer
{
public:
	wrHairRenderer();
	~wrHairRenderer();

	bool init();
	void release();
	void render(const wrHair&);

private:
	ID3D11Device* pd3dDevice;
	ID3D11DeviceContext* pd3dImmediateContext;

};