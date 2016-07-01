#pragma once
#include <d3d11.h>
#include <Effects.h>

#include "IRenderer.h"
#include "CFBXRendererDX11.h"

namespace XRwy
{
    //D3D11_INPUT_ELEMENT_DESC layout[] =
    //{
    //    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    //    { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    //};

    class LineRenderer:
        public IRenderer
    {
        typedef DirectX::XMFLOAT4X4 Matrix;
        typedef DirectX::XMFLOAT3   Float3;

    public:
        struct ConstantBuffer
        {
            Matrix viewProjMatrix;
            Matrix worldMatrix;
        };

        struct VertexBuffer
        {
            Float3 position;
            Float3 color;
        };

    public:
        void SetConstantBuffer(const ConstantBuffer* buffer);

        int GetNumOfRenderPass() const { return 1; }
        void SetRenderState(int i = 0, void* = nullptr);
        void GetVertexShaderBytecode(void const** pShaderByteCode, size_t* pByteCodeLength, void* = nullptr);
        bool Initialize();
        void Release();

    private:
        ID3D11VertexShader* pVertexShader = nullptr;
        ID3D11PixelShader*  pPixelShader = nullptr;
        ID3D11Buffer*       pConstantBuffer = nullptr;
        ID3DBlob*           pVSBlob = nullptr;
    };

    class MeshRenderer :
        public IRenderer
    {
    public:
        void SetMaterial(DirectX::BasicEffect* pEffect, const FBX_LOADER::MATERIAL_DATA* material);

        int GetNumOfRenderPass() const { return 1; }
        void SetRenderState(int i = 0, void* = nullptr);
        void GetVertexShaderBytecode(void const** pShaderByteCode, size_t* pByteCodeLength, void* effect);

        bool Initialize();
        void Release();

    private:
        DirectX::BasicEffect* pEffect;
    };
}