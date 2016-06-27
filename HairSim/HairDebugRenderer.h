#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include <vector>

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

    void activateMonoGroup(int idx);
    void toggleGDMode() { GDMode = (GDMode == 1) ? 0 : 1; }

    bool pointFlag = true;

protected:
    bool initWithShadow();
    bool initWithFollicle();
    void initColorSchemes();
    void drawCall(const WR::IHair* hair);

    void renderWithShadow(const WR::IHair* hair, ID3D11Buffer* vb,
        ID3D11Buffer* ib, const DirectX::XMFLOAT3* colors, float* offset);

    void render(const WR::IHair* hair, ID3D11Buffer* vb,
        ID3D11Buffer* ib, const DirectX::XMFLOAT3* colors, float* offset);

    const DirectX::XMFLOAT3* getColorBuffer() const;

    ID3D11Device*           pd3dDevice = nullptr;
    ID3D11DeviceContext*    pd3dImmediateContext = nullptr;

    ID3D11Buffer*           pVB = nullptr;
    ID3D11Buffer*           pIB = nullptr;
    ID3D11Buffer*           pIBPoint = nullptr;
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
    ID3D11VertexShader*     psmVS = nullptr, *psVS = nullptr, *pPVS = nullptr;
    ID3D11PixelShader*      psmPS = nullptr, *psPS = nullptr, *pPPS = nullptr;
    ID3D11GeometryShader*   psmGS = nullptr, *psGS = nullptr, *pPGS = nullptr;
    RenderTextureClass*     pShadowMap = nullptr;

    DirectX::XMFLOAT4X4     lightProjView;

    /* 配色方案 */
    enum COLOR_SCHEME { PSEUDO_COLOR, GUIDE_COLOR, GROUP_COLOR, ERROR_COLOR, DIR_COLOR, ERROR_GROUP_COLOR, NUM_COLOR_SCHEME };
    void setColorScheme(COLOR_SCHEME s);

    COLOR_SCHEME            colorScheme = PSEUDO_COLOR;
    DirectX::XMFLOAT3**     colorSet = nullptr;

    /* 单独显示某一个group的模块，GD，Group Display */
    bool isGDActive = false;
    int GDMode = 0;
    int GDId = -1;
    short* groupIndex = nullptr; // [0,0,0,1,1,1,2,2,2,2,2,.....]
    int* guideHairs = nullptr; // [1345,2345,2578,43,234,......]
    std::vector<int>* neighbourGroups = nullptr; // [0,1,2,3],[3,4,6,35],....

    size_t n_group = -1;
    size_t n_strand = -1;
};
