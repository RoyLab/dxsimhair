#pragma once
#include <DirectXMath.h>
#include "XRwy_h.h"

namespace XRwy
{
    class IRenderer :
        public IUnknown_
    {
    public:
        typedef DirectX::XMFLOAT4X4 Matrix;
        typedef DirectX::XMFLOAT3   Float3;

    public:
        virtual ~IRenderer(){}
        virtual void SetRenderState(int i = 0, void* = nullptr) = 0;
        virtual int GetNumOfRenderPass() const { return 0; }
        virtual void GetVertexShaderBytecode(void const** pShaderByteCode, size_t* pByteCodeLength, void* = nullptr) = 0;

    };
}