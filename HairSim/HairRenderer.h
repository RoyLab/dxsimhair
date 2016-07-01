#pragma once
#include <d3d11.h>
#include "XRwy_h.h"
#include "IRenderer.h"

namespace XRwy
{
    class HairRenderer:
        public IRenderer
    {
    public:
        struct ShadowMapCB
        {
            Matrix  lightProjViewMatrix;
            int     colorScheme;
            int     padding[3];
        };

    public:
        void SetConstantBuffer(const ConstBuffer* buffer);
        void SetRenderState(int i = 0, void* = nullptr);
        int GetNumOfRenderPass() const { return 2; }
        void GetVertexShaderBytecode(void const** pShaderByteCode, size_t* pByteCodeLength, void*);

        bool Initialize();
        void Release();

    private:
        ID3D11Device*           pd3dDevice = nullptr;
        ID3D11DeviceContext*    pd3dImmediateContext = nullptr;

        ID3DBlob* pVSBlob = nullptr;
        ID3D11VertexShader*     pShadowMapVS = nullptr, *pHairVS = nullptr;
        ID3D11GeometryShader*   pHairGS = nullptr;
        ID3D11PixelShader*      pShadowMapPS = nullptr, *pHairPS = nullptr;
        RenderTextureClass*     pShadowMapRenderTarget = nullptr;
        ID3D11Buffer*           pShadowMapConstBuffer = nullptr, *pHairConstBuffer = nullptr;
        ID3D11SamplerState*     psampleStateClamp = nullptr;

        ID3D11RenderTargetView* pMainRenderTarget = nullptr;
        ID3D11DepthStencilView* pMainDepthStencil = nullptr;
        D3D11_VIEWPORT          mainViewport;
    };
}