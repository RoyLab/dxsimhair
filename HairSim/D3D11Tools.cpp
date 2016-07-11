#include <cassert>
#include <DXUT.h>
#include "D3D11Tools.h"

namespace XRwy
{
    using namespace DirectX;

    void DumpLiveObjects(ID3D11Device* pd3dDevice, const char* name, bool detail)
    {
        auto para = D3D11_RLDO_SUMMARY;
        if (detail)
            para |= D3D11_RLDO_DETAIL;

        char info[200];

        strcpy(info, name);
        strcat_s(info, ": begin........................................\n");
        OutputDebugStringA(info);

        ID3D11Debug * d3dDebug = nullptr;
        if (SUCCEEDED(pd3dDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&d3dDebug))))
        {
            d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);
            d3dDebug->Release();
        }

        strcpy(info, name);
        strcat_s(info, ": end........................................\n");
        OutputDebugStringA(info);
    }

    void SetupDebugBuffers(DebugBuffers* buffers)
    {
        assert(buffers);
        buffers->pDevice = DXUTGetD3D11Device();
        buffers->pDCT = DXUTGetD3D11DeviceContext();
        HRESULT hr;

        // create vertex buffer
        D3D11_BUFFER_DESC bDesc;
        ZeroMemory(&bDesc, sizeof(D3D11_BUFFER_DESC));
        bDesc.ByteWidth = 3 * sizeof(XMFLOAT3);
        bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bDesc.Usage = D3D11_USAGE_DEFAULT;

        D3D11_SUBRESOURCE_DATA data;
        XMFLOAT3 vertices[] = 
        {
            {2.0f, 2.0f, 10.0f},
            {-2.0f, -2.0f, 10.0f},
            {-2.0f, 2.0f, 10.0f}
        };
        ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
        data.pSysMem = vertices;

        V(buffers->pDevice->CreateBuffer(&bDesc, &data, &buffers->pVB));

        // create index buffer
        bDesc.ByteWidth = 3 * sizeof(UINT);
        bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bDesc.Usage = D3D11_USAGE_DEFAULT;

        UINT indices[] = { 0, 1, 2 };
        ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
        data.pSysMem = indices;

        V(buffers->pDevice->CreateBuffer(&bDesc, &data, &buffers->pIB));
    }

    void ReleaseDebugBuffers(DebugBuffers* buffers)
    {
        SAFE_RELEASE(buffers->pIB);
        SAFE_RELEASE(buffers->pVB);
    }


    void DebugBuffers::drawCall(D3D11_PRIMITIVE_TOPOLOGY topology)
    {
        UINT strides[] = { sizeof(XMFLOAT3) };
        UINT offsets[] = { 0 };
        pDCT->IASetVertexBuffers(0, 1, &pVB, strides, offsets);
        //pDCT->IASetIndexBuffer(pIB, DXGI_FORMAT_R32_UINT, 0);
        pDCT->IASetPrimitiveTopology(topology);

        pDCT->DrawIndexed(3, 0, 0);
    }

}