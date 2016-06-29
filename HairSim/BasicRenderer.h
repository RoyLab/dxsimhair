#pragma once
#include <d3d11.h>
#include "XRwy_h.h"

namespace XRwy
{
    class LineRenderer:
        public IUnknown
    {
    public:
        ~LineRenderer(){ release(); }

        void setRenderState();
        bool init();
        void release();

    private:
        ID3D11VertexShader* pVS = nullptr;
        ID3D11PixelShader* pPS = nullptr;
        ID3D11InputLayout* pLayout = nullptr;
    };
}