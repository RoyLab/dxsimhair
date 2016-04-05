#pragma once
#include "IHair.h"
#include "wrSceneManager.h"


struct wrHairVertexInput
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT3 color;
};


class wrHairRenderer:
    public wrRendererInterface
{
public:
    wrHairRenderer(const WR::IHair* hair);
    ~wrHairRenderer();

    bool init(DirectX::XMFLOAT3* colors);
    void release();
    void onFrame(double, float){}
    void render(double, float);

private:
    ID3D11Device*           pd3dDevice = nullptr;
    ID3D11DeviceContext*    pd3dImmediateContext = nullptr;

    ID3D11Buffer*           pVB = nullptr;
    ID3D11Buffer*           pIB = nullptr;
    ID3D11VertexShader*     pVS = nullptr;
    ID3D11PixelShader*      pPS = nullptr;
    ID3D11InputLayout*      pLayout = nullptr;

    DirectX::XMFLOAT3*      vInputs = nullptr;

    const WR::IHair*        pHair;
};