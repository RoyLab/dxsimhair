#include <DXUT.h>
#include <SDKmisc.h>
#include <VertexTypes.h>
#include "BasicRenderer.h"


namespace XRwy
{
    using namespace DirectX;

    const D3D11_INPUT_ELEMENT_DESC LineRenderer::LayoutDesc[2] = 
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    bool LineRenderer::Initialize()
    {
        auto pd3dDevice = DXUTGetD3D11Device();

        HRESULT hr;
        ID3DBlob* pPSBlob = nullptr;

        // create shaders
        DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
        dwShaderFlags |= D3DCOMPILE_DEBUG;
        dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        V_RETURN(DXUTCompileFromFile(L"BasicLine.hlsl", nullptr, "VS", "vs_4_0", dwShaderFlags, 0, &pVSBlob));
        hr = pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &pVertexShader);
        if (FAILED(hr))
        {
            SAFE_RELEASE(pVSBlob);
            return hr;
        }

        V_RETURN(DXUTCompileFromFile(L"BasicLine.hlsl", nullptr, "PS", "ps_4_0", dwShaderFlags, 0, &pPSBlob));
        hr = pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &pPixelShader);
        if (FAILED(hr))
        {
            SAFE_RELEASE(pPSBlob);
            return hr;
        }

        SAFE_RELEASE(pPSBlob);

        // create constant buffer
        CD3D11_BUFFER_DESC bDesc;
        ZeroMemory(&bDesc, sizeof(CD3D11_BUFFER_DESC));
        bDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bDesc.ByteWidth = sizeof(ConstantBuffer);
        bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bDesc.Usage = D3D11_USAGE_DYNAMIC;

        pd3dDevice->CreateBuffer(&bDesc, nullptr, &pConstantBuffer);

        return true;
    }

    void LineRenderer::Release()
    {
        SAFE_RELEASE(pVertexShader);
        SAFE_RELEASE(pPixelShader);
        SAFE_RELEASE(pConstantBuffer);
        SAFE_RELEASE(pVSBlob);

        delete this;
    }

    void LineRenderer::SetRenderState(int i, void*)
    {
        auto pd3dImmediateContext = DXUTGetD3D11DeviceContext();

        pd3dImmediateContext->VSSetShader(pVertexShader, nullptr, 0);
        pd3dImmediateContext->PSSetShader(pPixelShader, nullptr, 0);
        pd3dImmediateContext->GSSetShader(nullptr, nullptr, 0);
        pd3dImmediateContext->VSSetConstantBuffers(0, 1, &pConstantBuffer);
    }

    void LineRenderer::SetConstantBuffer(const ConstantBuffer* buffer)
    {
        auto pd3dImmediateContext = DXUTGetD3D11DeviceContext();

        HRESULT hr;
        D3D11_MAPPED_SUBRESOURCE MappedResource;

        V(pd3dImmediateContext->Map(pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
        auto data = reinterpret_cast<ConstantBuffer*>(MappedResource.pData);
        memcpy(data, buffer, sizeof(ConstantBuffer));
        pd3dImmediateContext->Unmap(pConstantBuffer, 0);
    }

    void LineRenderer::GetVertexShaderBytecode(void const** pShaderByteCode, size_t* pByteCodeLength, void*)
    {
        *pShaderByteCode = pVSBlob->GetBufferPointer();
        *pByteCodeLength = pVSBlob->GetBufferSize();
    }

    void MeshRenderer::SetMaterial(const FBX_LOADER::MATERIAL_DATA* material)
    {
        assert(pEffect);
        if (!material) return;

        pEffect->SetTexture(material->pSRV);
        pEffect->SetAmbientLightColor(XMLoadFloat4(&material->ambient));
        pEffect->SetDiffuseColor(XMLoadFloat4(&material->diffuse));
        pEffect->SetSpecularColor(XMLoadFloat4(&material->specular));
    }

    void MeshRenderer::SetMatrices(const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& proj)
    {
        pEffect->SetWorld(world);
        pEffect->SetView(view);
        pEffect->SetProjection(proj);
    }

    void MeshRenderer::SetRenderState(int i, void*)
    {
        pEffect->EnableDefaultLighting();
        pEffect->SetPerPixelLighting(true);
        pEffect->SetTextureEnabled(false);
        pEffect->Apply(DXUTGetD3D11DeviceContext());
    }

    bool MeshRenderer::Initialize()
    {
        return true;
    }

    void MeshRenderer::Release()
    {
        delete this;
    }

    void MeshRenderer::GetVertexShaderBytecode(void const** pShaderByteCode, size_t* pByteCodeLength, void* effect)
    {
        pEffect = reinterpret_cast<BasicEffect*>(effect);
        SetRenderState();
        pEffect->GetVertexShaderBytecode(pShaderByteCode, pByteCodeLength);
    }
}