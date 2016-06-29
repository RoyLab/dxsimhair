#pragma once
#include <d3d11.h>
#include "XRwy_h.h"

namespace XRwy
{
    class IRenderer:
        public IUnknown
    {
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
        void SetRenderState();
        void SetConstantBuffer(const ConstantBuffer* buffer);
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
        typedef DirectX::XMFLOAT4X4 Matrix;
        typedef DirectX::XMFLOAT3   Float3;

    public:
        void SetRenderState();
        void SetConstantBuffer();
        bool Initialize();
        void Release();

    private:
    };
}