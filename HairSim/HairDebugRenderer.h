#pragma once
#include <DirectXMath.h>
#include <d3d11.h>

#include "IHair.h"
#include "wrSceneManager.h"

class RenderTextureClass;

struct HairDebugVertexInput
{
    int                 seq;
    DirectX::XMFLOAT3   pos;
    DirectX::XMFLOAT3   color;
    float               na1;
    DirectX::XMFLOAT3   direction;
    float               na2;
    DirectX::XMFLOAT3   ref;
    float               na3;
};


class HairBiDebugRenderer :
    public wrRendererInterface
{
public:
    HairBiDebugRenderer(const WR::IHair* hair1, const WR::IHair* hair0) :
        pHair(hair1), pHair0(hair0){}
    virtual ~HairBiDebugRenderer();

    virtual bool init();
    virtual void release();
    virtual void onFrame(double, float){}
    virtual void render(double, float);
    void nextColorScheme();
    void prevColorScheme();

protected:
    bool initWithShadow();
    void initColorSchemes();

    void renderWithShadow(const WR::IHair* hair, ID3D11Buffer* vb,
        ID3D11Buffer* ib, const DirectX::XMFLOAT3* colors, float* offset);

    void render(const WR::IHair* hair, ID3D11Buffer* vb,
        ID3D11Buffer* ib, const DirectX::XMFLOAT3* colors, float* offset);

    const DirectX::XMFLOAT3* getColorBuffer() const;

    ID3D11Device*           pd3dDevice = nullptr;
    ID3D11DeviceContext*    pd3dImmediateContext = nullptr;

    ID3D11Buffer*           pVB = nullptr;
    ID3D11Buffer*           pIB = nullptr;
    const WR::IHair*        pHair;

    ID3D11VertexShader*     pVS = nullptr;
    ID3D11PixelShader*      pPS = nullptr;
    ID3D11InputLayout*      pLayout = nullptr;


    ID3D11Buffer*           pVB0 = nullptr;
    ID3D11Buffer*           pIB0 = nullptr;
    const WR::IHair*        pHair0;

    /* for simple dsm*/
    bool                    bNeedShadow = false;
    DirectX::XMMATRIX       lightProjViewMatrix;
    ID3D11Buffer*           pCBShadow = nullptr;
    ID3D11SamplerState*     psampleStateClamp = nullptr;
    ID3D11VertexShader*     psmVS = nullptr, *psVS = nullptr;
    ID3D11PixelShader*      psmPS = nullptr, *psPS = nullptr;
    RenderTextureClass*     pShadowMap = nullptr;

    DirectX::XMFLOAT4X4     lightProjView;

    /* ÅäÉ«·½°¸ */
    enum COLOR_SCHEME { PSEUDO_COLOR, GUIDE_COLOR, GROUP_COLOR, ERROR_COLOR, DIR_COLOR, ERROR_GROUP_COLOR, NUM_COLOR_SCHEME };
    void setColorScheme(COLOR_SCHEME s);

    COLOR_SCHEME            colorScheme = PSEUDO_COLOR;
    DirectX::XMFLOAT3**     colorSet = nullptr;
};
