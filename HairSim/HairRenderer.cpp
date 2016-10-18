#include "precompiled.h"
#include <DXUT.h>
#include <SDKmisc.h>
#include "HairRenderer.h"
#include "rendertextureclass.h"

using namespace DirectX;

namespace XRwy
{
    const D3D11_INPUT_ELEMENT_DESC HairRenderer::LayoutDesc[5] =
    {
        { "SEQ", 0, DXGI_FORMAT_R32_SINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "DIR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "REF", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    void HairRenderer::SetRenderState(int pass, void*)
    {
        if (pass == 0)
        {
            UINT nvp = 1;
            pd3dImmediateContext->OMGetRenderTargets(1, &pMainRenderTarget, &pMainDepthStencil);
            pd3dImmediateContext->RSGetViewports(&nvp, &mainViewport);

			ID3D11ShaderResourceView* pNull = nullptr;
			pd3dImmediateContext->PSSetShaderResources(0, 1, &pNull);
            pShadowMapRenderTarget->SetRenderTarget(pd3dImmediateContext);
            pShadowMapRenderTarget->ClearRenderTarget(pd3dImmediateContext, 1.0f, /*no use*/0.0f, 0.0f, 0.0f);

            pd3dImmediateContext->VSSetConstantBuffers(0, 1, &pConstBuffer);
            pd3dImmediateContext->VSSetShader(pShadowMapVS, nullptr, 0);
            pd3dImmediateContext->PSSetShader(pShadowMapPS, nullptr, 0);
            pd3dImmediateContext->GSSetShader(nullptr, nullptr, 0);

            return;
        }

        if (pass == 1)
        {
            pd3dImmediateContext->OMSetRenderTargets(1, &pMainRenderTarget, pMainDepthStencil);
            pd3dImmediateContext->RSSetViewports(1, &mainViewport);
            SAFE_RELEASE(pMainRenderTarget);
            SAFE_RELEASE(pMainDepthStencil);

            auto pTexture = pShadowMapRenderTarget->GetShaderResourceView();
            pd3dImmediateContext->PSSetShaderResources(0, 1, &pTexture);
            pd3dImmediateContext->PSSetSamplers(0, 1, &psampleStateClamp);

            pd3dImmediateContext->VSSetConstantBuffers(0, 1, &pConstBuffer);
            pd3dImmediateContext->GSSetConstantBuffers(0, 1, &pConstBuffer);

            pd3dImmediateContext->VSSetShader(pHairVS, nullptr, 0);
            pd3dImmediateContext->GSSetShader(pHairGS, nullptr, 0);
            pd3dImmediateContext->PSSetShader(pHairPS, nullptr, 0);

            return;
        }
    }

    void HairRenderer::GetVertexShaderBytecode(void const** pShaderByteCode, size_t* pByteCodeLength, void*)
    {
        *pShaderByteCode = pVSBlob->GetBufferPointer();
        *pByteCodeLength = pVSBlob->GetBufferSize();
    }

    bool HairRenderer::Initialize()
    {
        HRESULT hr;
        DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
        dwShaderFlags |= D3DCOMPILE_DEBUG;
        dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        pd3dDevice = DXUTGetD3D11Device();
        pd3dImmediateContext = DXUTGetD3D11DeviceContext();

        // Compile the shadow hair vertex shader
        ID3DBlob* pBlob = nullptr;
        V_RETURN(DXUTCompileFromFile(L"HairFx.hlsl", nullptr, "VS", "vs_4_0", dwShaderFlags, 0, &pBlob));

        hr = pd3dDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pHairVS);
        if (FAILED(hr))
        {
            SAFE_RELEASE(pBlob);
            return hr;
        }
        pVSBlob = pBlob;

        V_RETURN(DXUTCompileFromFile(L"HairFx.hlsl", nullptr, "PS", "ps_4_0", dwShaderFlags, 0, &pBlob));

        hr = pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pHairPS);
        if (FAILED(hr))
        {
            SAFE_RELEASE(pBlob);
            return hr;
        }
        SAFE_RELEASE(pBlob);

        // Compile the shadow map geometry shader
        V_RETURN(DXUTCompileFromFile(L"HairFx.hlsl", nullptr, "GS", "gs_4_0", dwShaderFlags, 0, &pBlob));

        hr = pd3dDevice->CreateGeometryShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pHairGS);
        if (FAILED(hr))
        {
            SAFE_RELEASE(pBlob);
            return hr;
        }
        SAFE_RELEASE(pBlob);

        // Compile the shadow map vertex shader
        V_RETURN(DXUTCompileFromFile(L"HairFx.hlsl", nullptr, "SM_VS", "vs_4_0", dwShaderFlags, 0, &pBlob));

        hr = pd3dDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pShadowMapVS);
        if (FAILED(hr))
        {
            SAFE_RELEASE(pBlob);
            return hr;
        }
        SAFE_RELEASE(pBlob);

        // Compile the shadow map pixel shader
        V_RETURN(DXUTCompileFromFile(L"HairFx.hlsl", nullptr, "SM_PS", "ps_4_0", dwShaderFlags, 0, &pBlob));

        hr = pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pShadowMapPS);
        if (FAILED(hr))
        {
            SAFE_RELEASE(pBlob);
            return hr;
        }
        SAFE_RELEASE(pBlob);


        ConstBuffer smcbuffer;
        pShadowMapRenderTarget = new RenderTextureClass;
        bool result = pShadowMapRenderTarget->Initialize(pd3dDevice, 1024, 704, 100.0f, 0.1f);
        if (!result)
        {
            assert(0);
            return false;
        }

        Matrix proj;  pShadowMapRenderTarget->GetOrthoMatrix(proj);
        Float3 lightPos = Float3(10.0f, 10.0f, -10.0f);
        Float3 lightTarget = Float3(0.0f, 0.0f, 0.0f);
        Float3 lightUp = Float3(0.0f, 1.0f, 0.0f);

        XMStoreFloat4x4(&mlightProjViewWorld, XMMatrixTranspose(XMMatrixLookAtLH(XMLoadFloat3(&lightPos),
            XMLoadFloat3(&lightTarget), XMLoadFloat3(&lightUp))*XMLoadFloat4x4(&proj)));

        CD3D11_BUFFER_DESC bDesc;
        ZeroMemory(&bDesc, sizeof(CD3D11_BUFFER_DESC));
        bDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bDesc.ByteWidth = sizeof(ConstBuffer);
        bDesc.Usage = D3D11_USAGE_DYNAMIC;
        bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        V_RETURN(pd3dDevice->CreateBuffer(&bDesc, nullptr, &pConstBuffer));

        CD3D11_SAMPLER_DESC samplerDesc;
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.MipLODBias = 0.0f;
        samplerDesc.MaxAnisotropy = 1;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        samplerDesc.BorderColor[0] = 0;
        samplerDesc.BorderColor[1] = 0;
        samplerDesc.BorderColor[2] = 0;
        samplerDesc.BorderColor[3] = 0;
        samplerDesc.MinLOD = 0;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

        // Create the texture sampler state.
        V_RETURN(pd3dDevice->CreateSamplerState(&samplerDesc, &psampleStateClamp));

        return true;
    }

    void HairRenderer::Release()
    {        
        SAFE_RELEASE(pVSBlob);

        SAFE_RELEASE(pShadowMapVS);
        SAFE_RELEASE(pHairVS);
        SAFE_RELEASE(pHairGS);
        SAFE_RELEASE(pShadowMapPS);
        SAFE_RELEASE(pHairPS);

        SAFE_RELEASE(pShadowMapRenderTarget);

        SAFE_RELEASE(pConstBuffer);

        SAFE_RELEASE(psampleStateClamp);
        SAFE_RELEASE(pMainRenderTarget);
        SAFE_RELEASE(pMainDepthStencil);

        delete this;
    }

    void HairRenderer::SetConstantBuffer(const ConstBuffer* buffer)
    {
        HRESULT hr;
        D3D11_MAPPED_SUBRESOURCE MappedResource;

        V(pd3dImmediateContext->Map(pConstBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
        auto data = reinterpret_cast<ConstBuffer*>(MappedResource.pData);
        CopyMemory(data, buffer, sizeof(ConstBuffer));
        pd3dImmediateContext->Unmap(pConstBuffer, 0);
    }

    void HairRenderer::GetShadowMapProjMatrix(Matrix& proj)
    {
        pShadowMapRenderTarget->GetOrthoMatrix(proj);
    }
        

}