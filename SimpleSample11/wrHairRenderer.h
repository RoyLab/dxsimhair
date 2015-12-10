#pragma once
#include "wrHair.h"
#include <d3d11.h>
#include <DirectXMath.h>


struct wrHairVertexInput
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT3 color;
};


class wrHairRenderer
{
public:
	wrHairRenderer();
	~wrHairRenderer();

    bool init(const wrHair&);
	void release();
	void render(const wrHair&);

private:
    ID3D11Device*           pd3dDevice = nullptr;
    ID3D11DeviceContext*    pd3dImmediateContext = nullptr;

    ID3D11Buffer*           pVB = nullptr;
    ID3D11Buffer*           pIB = nullptr;
    ID3D11VertexShader*     pVS = nullptr;
    ID3D11PixelShader*      pPS = nullptr;
    ID3D11InputLayout*      pLayout = nullptr;

    wrHairVertexInput*      vInputs = nullptr;
};