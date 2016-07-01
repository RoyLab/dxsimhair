#include <DXUT.h>
#include <SDKmisc.h>
#include "HairRenderer.h"
#include "rendertextureclass.h"

using namespace DirectX;

namespace XRwy
{
    void HairRenderer::SetRenderState(int i, void*)
    {

    }

    void HairRenderer::GetVertexShaderBytecode(void const** pShaderByteCode, size_t* pByteCodeLength, void*)
    {

    }

    bool HairRenderer::Initialize()
    {
        HRESULT hr;
        DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
        dwShaderFlags |= D3DCOMPILE_DEBUG;
        dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        auto pd3dDevice = DXUTGetD3D11Device();

        // Compile the shadow hair vertex shader
        ID3DBlob* pBlob = nullptr;
        V_RETURN(DXUTCompileFromFile(L"HairShadow.hlsl", nullptr, "VS", "vs_4_0", dwShaderFlags, 0, &pVSBlob));

        hr = pd3dDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pHairVS);
        if (FAILED(hr))
        {
            SAFE_RELEASE(pBlob);
            return hr;
        }
        pVSBlob = pBlob;

        V_RETURN(DXUTCompileFromFile(L"HairShadow.hlsl", nullptr, "PS", "ps_4_0", dwShaderFlags, 0, &pBlob));

        hr = pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pHairPS);
        if (FAILED(hr))
        {
            SAFE_RELEASE(pBlob);
            return hr;
        }
        SAFE_RELEASE(pBlob);

        // Compile the shadow map geometry shader
        V_RETURN(DXUTCompileFromFile(L"HairShadow.hlsl", nullptr, "GS", "gs_4_0", dwShaderFlags, 0, &pBlob));

        hr = pd3dDevice->CreateGeometryShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pHairGS);
        if (FAILED(hr))
        {
            SAFE_RELEASE(pBlob);
            return hr;
        }
        SAFE_RELEASE(pBlob);

        // Compile the shadow map vertex shader
        V_RETURN(DXUTCompileFromFile(L"HairShadow.hlsl", nullptr, "SM_VS", "vs_4_0", dwShaderFlags, 0, &pBlob));

        hr = pd3dDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pShadowMapVS);
        if (FAILED(hr))
        {
            SAFE_RELEASE(pBlob);
            return hr;
        }
        SAFE_RELEASE(pBlob);

        // Compile the shadow map pixel shader
        V_RETURN(DXUTCompileFromFile(L"HairShadow.hlsl", nullptr, "SM_PS", "ps_4_0", dwShaderFlags, 0, &pBlob));

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

        XMStoreFloat4x4(&smcbuffer.lightProjViewMatrix, XMMatrixTranspose(XMMatrixLookAtLH(XMLoadFloat3(&lightPos),
            XMLoadFloat3(&lightTarget), XMLoadFloat3(&lightUp))*XMLoadFloat4x4(&proj)));
        smcbuffer.colorScheme = 0;

        D3D11_SUBRESOURCE_DATA subRes;
        ZeroMemory(&subRes, sizeof(D3D11_SUBRESOURCE_DATA));
        subRes.pSysMem = &smcbuffer;

        CD3D11_BUFFER_DESC bDesc;
        ZeroMemory(&bDesc, sizeof(CD3D11_BUFFER_DESC));
        bDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bDesc.ByteWidth = sizeof(ConstBuffer);
        bDesc.Usage = D3D11_USAGE_DEFAULT;
        V_RETURN(pd3dDevice->CreateBuffer(&bDesc, &subRes, &pShadowMapConstBuffer));

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
        return true;
    }

    void HairRenderer::Release()
    {
        delete this;
    }

    void HairRenderer::SetConstantBuffer(const ConstBuffer* buffer)
    {

    }

}