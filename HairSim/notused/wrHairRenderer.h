#pragma once
#include "linmath.h"
#include "IHair.h"
#include "wrSceneManager.h"


struct wrHairVertexInput
{
    //int                 seq;
    DirectX::XMFLOAT3   pos;
    //DirectX::XMFLOAT3   direction;
    DirectX::XMFLOAT3   color;
};


class wrHairRenderer:
    public wrRendererInterface
{
public:
    wrHairRenderer(const WR::IHair* hair);
    virtual ~wrHairRenderer();

    virtual bool init(DirectX::XMFLOAT3* colors);
    virtual void release();
    virtual void onFrame(double, float){}
    virtual void render(double, float);

protected:
    void render(const WR::IHair* hair, ID3D11Buffer* vb, 
        ID3D11Buffer* ib, DirectX::XMFLOAT3* colors, float* offset);

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


class wrBiHairRenderer :
    public wrHairRenderer
{
public:
    wrBiHairRenderer(const WR::IHair* hair1, const WR::IHair* hair0):
        wrHairRenderer(hair1), pHair0(hair0){}
    bool init(DirectX::XMFLOAT3* colors);
    void release();
    void render(double, float);

protected:
    ID3D11Buffer*           pVB0 = nullptr;
    ID3D11Buffer*           pIB0 = nullptr;
    const WR::IHair*        pHair0;
};