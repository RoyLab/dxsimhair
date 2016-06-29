#pragma once
#include <d3d11.h>
#include <Effects.h>

#include "XRwy_h.h"
#include "CFBXRendererDX11.h"

namespace XRwy
{
    class IRenderer:
        public IUnknown
    {
    public:
        typedef DirectX::XMFLOAT4X4 Matrix;
        typedef DirectX::XMFLOAT3   Float3;

    public:
        virtual ~IRenderer(){}
        virtual void SetRenderState() = 0;
    };

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

        void SetRenderState();
        bool Initialize();
        void Release();

    private:
        ID3D11VertexShader* pVertexShader = nullptr;
        ID3D11PixelShader*  pPixelShader = nullptr;
        ID3D11InputLayout*  pInputLayout = nullptr;
        ID3D11Buffer*       pConstantBuffer = nullptr;
    };

    class MeshRenderer :
        public IRenderer
    {
        DirectX::BasicEffect* pEffect;
    public:
        void SetMaterial(DirectX::BasicEffect* pEffect, const FBX_LOADER::MATERIAL_DATA* material);

        void SetRenderState();
        bool Initialize();
        void Release();

    private:
    };
}