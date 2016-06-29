#include <DXUT.h>
#include <SDKmisc.h>
#include "BasicRenderer.h"
#include "HairDebugRenderer.h"
ID3D11Buffer *pIndexBuffer;
extern ID3D11Buffer* gIb;
extern ID3D11InputLayout* gLayout;

namespace XRwy
{
    bool LineRenderer::init()
    {
        HRESULT hr;
        auto pd3dDevice = DXUTGetD3D11Device();
        auto pd3dImmediateContext = DXUTGetD3D11DeviceContext();
        ID3DBlob* pVSBlob = nullptr;
        ID3DBlob* pPSBlob = nullptr;

        DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
        dwShaderFlags |= D3DCOMPILE_DEBUG;
        dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        V_RETURN(DXUTCompileFromFile(L"BasicLine.hlsl", nullptr, "VS", "vs_4_0", dwShaderFlags, 0, &pVSBlob));
        hr = pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &pVS);
        if (FAILED(hr))
        {
            SAFE_RELEASE(pVSBlob);
            return hr;
        }

        V_RETURN(DXUTCompileFromFile(L"BasicLine.hlsl", nullptr, "PS", "ps_4_0", dwShaderFlags, 0, &pPSBlob));
        hr = pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &pPS);
        if (FAILED(hr))
        {
            SAFE_RELEASE(pPSBlob);
            return hr;
        }

        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        UINT numElements = ARRAYSIZE(layout);
        //V_RETURN(pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &pLayout));

        SAFE_RELEASE(pVSBlob);
        SAFE_RELEASE(pPSBlob);


        DWORD *indices = new DWORD[10000];
        D3D11_SUBRESOURCE_DATA subRes;
        CD3D11_BUFFER_DESC bDesc;

        ZeroMemory(&bDesc, sizeof(CD3D11_BUFFER_DESC));
        ZeroMemory(&subRes, sizeof(D3D11_SUBRESOURCE_DATA));

        bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bDesc.ByteWidth = (10000) * sizeof(DWORD);
        bDesc.CPUAccessFlags = 0;
        bDesc.Usage = D3D11_USAGE_DEFAULT;

        for (int i = 0; i < 10000; i++)
            indices[i] = i;

        subRes.pSysMem = indices;

        V(DXUTGetD3D11Device()->CreateBuffer(&bDesc, &subRes, &pIndexBuffer));
        SAFE_DELETE_ARRAY(indices);

        return true;
    }

    void LineRenderer::release()
    {
        SAFE_RELEASE(pVS);
        SAFE_RELEASE(pPS);
        SAFE_RELEASE(pLayout);
    }

    void LineRenderer::setRenderState()
    {
        UINT strides[1] = { sizeof(HairDebugVertexInput) }, offsets[1] = { 0 };
        auto pd3dImmediateContext = DXUTGetD3D11DeviceContext();
        
        pd3dImmediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
        pd3dImmediateContext->VSSetShader(pVS, 0, 0);
        pd3dImmediateContext->PSSetShader(pPS, 0, 0);
        pd3dImmediateContext->GSSetShader(nullptr, 0, 0);
        // TO-DO
        pd3dImmediateContext->IASetInputLayout(gLayout);
        //pd3dImmediateContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
        //pd3dImmediateContext->DrawIndexed(10000, 0, 0);

    }
}