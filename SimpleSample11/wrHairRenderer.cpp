#include "DXUT.h"
#include "wrHairRenderer.h"
#include <DirectXMath.h>
#include "wrColorGenerator.h"
#include "wrLogger.h"
#include "SDKmisc.h"

using namespace DirectX;

wrHairRenderer::wrHairRenderer(){}

wrHairRenderer::~wrHairRenderer(){}

bool wrHairRenderer::init(const wrHair& hair)
{
    HRESULT hr; 

    pd3dDevice = DXUTGetD3D11Device();
    pd3dImmediateContext = DXUTGetD3D11DeviceContext();

    int n_particles = hair.n_strands() *  N_PARTICLES_PER_STRAND;

    // assign random color
    vInputs = new XMFLOAT3[n_particles];
    for (int i = 0; i < hair.n_strands(); i++)
    {
        vec3 color;
        wrColorGenerator::genRandSaturatedColor(color);
        for (int j = 0; j < N_PARTICLES_PER_STRAND; j++)
           memcpy(&vInputs[N_PARTICLES_PER_STRAND*i + j], color, sizeof(vec3));
    }


    D3D11_SUBRESOURCE_DATA subRes;
    //ZeroMemory(&subRes, sizeof(D3D11_SUBRESOURCE_DATA));
    //subRes.pSysMem = vInputs;

    CD3D11_BUFFER_DESC bDesc;
    ZeroMemory(&bDesc, sizeof(CD3D11_BUFFER_DESC));
    bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bDesc.ByteWidth = n_particles * sizeof(wrHairVertexInput);
    bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bDesc.Usage = D3D11_USAGE_DYNAMIC;

    V_RETURN(pd3dDevice->CreateBuffer(&bDesc, nullptr, &pVB));

    ZeroMemory(&bDesc, sizeof(CD3D11_BUFFER_DESC));
    bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bDesc.ByteWidth = n_particles * sizeof(DWORD);
    bDesc.Usage = D3D11_USAGE_DEFAULT;

    ZeroMemory(&subRes, sizeof(D3D11_SUBRESOURCE_DATA));
    DWORD *indices = new DWORD[n_particles];
    for (int i = 0; i < n_particles; i++)
        indices[i] = i;
    subRes.pSysMem = indices;

    V_RETURN(pd3dDevice->CreateBuffer(&bDesc, &subRes, &pIB));
    SAFE_DELETE_ARRAY(indices);

    // create vs, ps, layout
    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;

    // Disable optimizations to further improve shader debugging
    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    V_RETURN(DXUTCompileFromFile(L"Line.hlsl", nullptr, "VS", "vs_4_0", dwShaderFlags, 0, &pVSBlob));

    // Create the vertex shader
    hr = pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &pVS);
    if (FAILED(hr))
    {
        SAFE_RELEASE(pVSBlob);
        return hr;
    }

    // Compile the pixel shader
    ID3DBlob* pPSBlob = nullptr;
    V_RETURN(DXUTCompileFromFile(L"Line.hlsl", nullptr, "PS", "ps_4_0", dwShaderFlags, 0, &pPSBlob));

    // Create the pixel shader
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
    V_RETURN(pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &pLayout));

    SAFE_RELEASE(pVSBlob);
    SAFE_RELEASE(pPSBlob);

    return true;
}

void wrHairRenderer::release()
{
    SAFE_RELEASE(pVB);
    SAFE_RELEASE(pIB);
    SAFE_RELEASE(pVS);
    SAFE_RELEASE(pPS);
    SAFE_RELEASE(pLayout);

    SAFE_DELETE_ARRAY(vInputs);
}

void wrHairRenderer::render(const wrHair& hair)
{
    if (!pVB) WR_LOG_ERROR << "No pVB available.\n";

    HRESULT hr;
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    V(pd3dImmediateContext->Map(pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));

    auto pData = reinterpret_cast<wrHairVertexInput*>(MappedResource.pData);
    int n_strands = hair.n_strands();
    for (int i = 0; i < n_strands; i++)
        for (int j = 0; j < N_PARTICLES_PER_STRAND; j++)
        {
            memcpy(&pData[25 * i + j].pos, hair.getStrand(i).getParticles()[j].position, sizeof(vec3));
            memcpy(&pData[25 * i + j].color, &vInputs[25 * i + j], sizeof(vec3));
        }

    pd3dImmediateContext->Unmap(pVB, 0);

    UINT strides[1] = { sizeof(wrHairVertexInput) }, offsets[1] = { 0 };

    pd3dImmediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP);
    pd3dImmediateContext->IASetVertexBuffers(0, 1, &pVB, strides, offsets);
    pd3dImmediateContext->IASetIndexBuffer(pIB, DXGI_FORMAT_R32_UINT, 0);
    pd3dImmediateContext->IASetInputLayout(pLayout);
    pd3dImmediateContext->VSSetShader(pVS, nullptr, 0);
    pd3dImmediateContext->PSSetShader(pPS, nullptr, 0);

    int start = 0;
    for (int i = 0; i < n_strands; i++, start += N_PARTICLES_PER_STRAND)
        pd3dImmediateContext->DrawIndexed(N_PARTICLES_PER_STRAND, start, 0);
}