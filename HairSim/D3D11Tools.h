#pragma once
#include <d3d11.h>
#include <iostream>

namespace XRwy
{
    void DumpLiveObjects(ID3D11Device* pd3dDevice, const char* name, bool detail = false);

    struct DebugBuffers
    {
        ID3D11Buffer* pVB;
        ID3D11Buffer* pIB;

        ID3D11Device* pDevice;
        ID3D11DeviceContext* pDCT;

        void drawCall(D3D11_PRIMITIVE_TOPOLOGY topology);
    };

    void SetupDebugBuffers(DebugBuffers*);
    void ReleaseDebugBuffers(DebugBuffers*);
}

