#include "precompiled.h"

#include <fstream>
#include <iostream>

#include "HairDebugRenderer.h"
#include <DirectXMath.h>
#include "wrLogger.h"
#include "SDKmisc.h"
#include "Parameter.h"
#include "linmath.h"
#include "rendertextureclass.h"
#include "wrColorGenerator.h"

using namespace DirectX;

extern bool hasShadow;
extern std::string GUIDE_FILE;
extern std::string GROUP_FILE;

struct ShadowMapConstBuffer
{
    XMFLOAT4X4 lightProjViewMatrix;
    int colorScheme;
    int padding[3];
};

HairBiDebugRenderer::~HairBiDebugRenderer()
{
    release();
}

bool HairBiDebugRenderer::init()
{
    bNeedShadow = hasShadow;

    HRESULT hr;

    pd3dDevice = DXUTGetD3D11Device();
    pd3dImmediateContext = DXUTGetD3D11DeviceContext();

    int n_particles = pHair->n_strands() *  N_PARTICLES_PER_STRAND;

    D3D11_SUBRESOURCE_DATA subRes;
    //ZeroMemory(&subRes, sizeof(D3D11_SUBRESOURCE_DATA));
    //subRes.pSysMem = vInputs;

    CD3D11_BUFFER_DESC bDesc;
    ZeroMemory(&bDesc, sizeof(CD3D11_BUFFER_DESC));
    bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bDesc.ByteWidth = n_particles * sizeof(HairDebugVertexInput);
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
    V_RETURN(DXUTCompileFromFile(L"Hair.hlsl", nullptr, "VS", "vs_4_0", dwShaderFlags, 0, &pVSBlob));

    // Create the vertex shader
    hr = pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &pVS);
    if (FAILED(hr))
    {
        SAFE_RELEASE(pVSBlob);
        return hr;
    }

    // Compile the pixel shader
    ID3DBlob* pPSBlob = nullptr;
    V_RETURN(DXUTCompileFromFile(L"Hair.hlsl", nullptr, "PS", "ps_4_0", dwShaderFlags, 0, &pPSBlob));

    // Create the pixel shader
    hr = pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &pPS);
    if (FAILED(hr))
    {
        SAFE_RELEASE(pPSBlob);
        return hr;
    }

    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "SEQ", 0, DXGI_FORMAT_R32_SINT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "DIRECTION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    UINT numElements = ARRAYSIZE(layout);
    V_RETURN(pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &pLayout));
    
    SAFE_RELEASE(pVSBlob);
    SAFE_RELEASE(pPSBlob);

    ZeroMemory(&bDesc, sizeof(CD3D11_BUFFER_DESC));
    bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bDesc.ByteWidth = n_particles * sizeof(HairDebugVertexInput);
    bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bDesc.Usage = D3D11_USAGE_DYNAMIC;

    V_RETURN(pd3dDevice->CreateBuffer(&bDesc, nullptr, &pVB0));

    ZeroMemory(&bDesc, sizeof(CD3D11_BUFFER_DESC));
    bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bDesc.ByteWidth = n_particles * sizeof(DWORD);
    bDesc.Usage = D3D11_USAGE_DEFAULT;

    ZeroMemory(&subRes, sizeof(D3D11_SUBRESOURCE_DATA));
    indices = new DWORD[n_particles];
    for (int i = 0; i < n_particles; i++)
        indices[i] = i;
    subRes.pSysMem = indices;

    V_RETURN(pd3dDevice->CreateBuffer(&bDesc, &subRes, &pIB0));
    SAFE_DELETE_ARRAY(indices);

    if (bNeedShadow)
        initWithShadow();

    initColorSchemes();
    return true;
}

void HairBiDebugRenderer::release()
{
    SAFE_RELEASE(pVB);
    SAFE_RELEASE(pIB);
    SAFE_RELEASE(pVS);
    SAFE_RELEASE(pPS);
    SAFE_RELEASE(pLayout);
    SAFE_RELEASE(pVB0);
    SAFE_RELEASE(pIB0);

    SAFE_RELEASE(pCBShadow);
    SAFE_RELEASE(psampleStateClamp);
    SAFE_RELEASE(psmVS);
    SAFE_RELEASE(psmPS);
    SAFE_RELEASE(psVS);
    SAFE_RELEASE(psPS);

    if (pShadowMap)
        pShadowMap->Shutdown();
    SAFE_DELETE(pShadowMap);

    if (colorSet)
    {
        for (size_t i = 0; i < NUM_COLOR_SCHEME; i++)
            SAFE_DELETE_ARRAY(colorSet[i]);
    }

    SAFE_DELETE_ARRAY(colorSet);
}

void HairBiDebugRenderer::render(double fTime, float fTimeElapsed)
{
    vec3 offset0{ -2.f, 0, 0 };
    HairBiDebugRenderer::render(pHair0, pVB0, pIB0, getColorBuffer(), offset0);

    vec3 offset{ 2.f, 0, 0 };
    HairBiDebugRenderer::render(pHair, pVB, pIB, getColorBuffer(), offset);
}

void HairBiDebugRenderer::render(const WR::IHair* hair, ID3D11Buffer* vb,
    ID3D11Buffer* ib, const DirectX::XMFLOAT3* colors, float* offset)
{
    if (!vb) WR_LOG_ERROR << "No pVB available.\n";

    HRESULT hr;
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    V(pd3dImmediateContext->Map(vb, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));

    auto pData = reinterpret_cast<HairDebugVertexInput*>(MappedResource.pData);
    int n_strands = hair->n_strands();
    for (int i = 0; i < n_strands; i++)
    {
        for (int j = 0; j < N_PARTICLES_PER_STRAND; j++)
        {
            vec3 pos, dir;
            memcpy(pos, hair->get_visible_particle_position(i, j), sizeof(vec3));
            memcpy(dir, hair->get_visible_particle_direction(i, j), sizeof(vec3));
            vec3_add(pos, offset, pos);
            pData[N_PARTICLES_PER_STRAND * i + j].seq = j;
            memcpy(&pData[N_PARTICLES_PER_STRAND * i + j].pos, pos, sizeof(vec3));
            memcpy(&pData[N_PARTICLES_PER_STRAND * i + j].direction, dir, sizeof(vec3));
            if (colorScheme != DIR_COLOR)
            {
                if (colorScheme != ERROR_COLOR || hair == pHair0)
                    memcpy(&pData[N_PARTICLES_PER_STRAND * i + j].color, &colors[N_PARTICLES_PER_STRAND * i + j], sizeof(vec3));
                else
                {
                    memcpy(pos, pHair0->get_visible_particle_position(i, j), sizeof(vec3));
                    vec3_add(pos, offset, pos);
                    memcpy(&pData[N_PARTICLES_PER_STRAND * i + j].color, pos, sizeof(vec3));
                }
            }
        }
    }
    pd3dImmediateContext->Unmap(vb, 0);


    UINT strides[1] = { sizeof(HairDebugVertexInput) }, offsets[1] = { 0 };
    pd3dImmediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP);
    pd3dImmediateContext->IASetVertexBuffers(0, 1, &vb, strides, offsets);
    pd3dImmediateContext->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
    pd3dImmediateContext->IASetInputLayout(pLayout);

    if (bNeedShadow)
    {
        renderWithShadow(hair, vb, ib, colors, offset);
    }
    else
    {
        pd3dImmediateContext->VSSetShader(pVS, nullptr, 0);
        pd3dImmediateContext->PSSetShader(pPS, nullptr, 0);
    }

    int start = 0;
    for (int i = 0; i < n_strands; i++, start += N_PARTICLES_PER_STRAND)
        pd3dImmediateContext->DrawIndexed(N_PARTICLES_PER_STRAND, start, 0);
}

void HairBiDebugRenderer::renderWithShadow(const WR::IHair* hair, ID3D11Buffer* vb,
    ID3D11Buffer* ib, const DirectX::XMFLOAT3* colors, float* offset)
{
    /* save the old render target for restoration */
    ID3D11RenderTargetView* pRTV = nullptr;
    ID3D11DepthStencilView* pDSV = nullptr;
    ID3D11ShaderResourceView* pNull = nullptr;
    D3D11_VIEWPORT vp;
    UINT nvp = 1;
    pd3dImmediateContext->OMGetRenderTargets(1, &pRTV, &pDSV);
    pd3dImmediateContext->RSGetViewports(&nvp, &vp);

    pd3dImmediateContext->PSSetShaderResources(0, 1, &pNull);
    pShadowMap->SetRenderTarget(pd3dImmediateContext);
    pShadowMap->ClearRenderTarget(pd3dImmediateContext, 1.0f, /*no use*/0.0f, 0.0f, 0.0f);

    if (hair == pHair0 && colorScheme == ERROR_COLOR)
        setColorScheme(GUIDE_COLOR);
    else
        setColorScheme(colorScheme);

    pd3dImmediateContext->VSSetConstantBuffers(1, 1, &pCBShadow);
    pd3dImmediateContext->VSSetShader(psmVS, nullptr, 0);
    pd3dImmediateContext->PSSetShader(psmPS, nullptr, 0);

    int start = 0;
    for (int i = 0; i < hair->n_strands(); i++, start += N_PARTICLES_PER_STRAND)
        pd3dImmediateContext->DrawIndexed(N_PARTICLES_PER_STRAND, start, 0);

    pd3dImmediateContext->OMSetRenderTargets(1, &pRTV, pDSV);
    pd3dImmediateContext->RSSetViewports(1, &vp);
    SAFE_RELEASE(pRTV);
    SAFE_RELEASE(pDSV);

    auto pTexture = pShadowMap->GetShaderResourceView();
    pd3dImmediateContext->PSSetShaderResources(0, 1, &pTexture);
    pd3dImmediateContext->PSSetSamplers(0, 1, &psampleStateClamp);
    pd3dImmediateContext->VSSetShader(psVS, nullptr, 0);
    pd3dImmediateContext->PSSetShader(psPS, nullptr, 0);

    //start = 0;
    //for (int i = 0; i < hair->n_strands(); i++, start += N_PARTICLES_PER_STRAND)
    //    pd3dImmediateContext->DrawIndexed(N_PARTICLES_PER_STRAND, start, 0);
}

bool HairBiDebugRenderer::initWithShadow()
{
    HRESULT hr;
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

    // Compile the shadow hair vertex shader
    ID3DBlob* pVSBlob = nullptr;
    V_RETURN(DXUTCompileFromFile(L"HairShadow.hlsl", nullptr, "VS", "vs_5_0", dwShaderFlags, 0, &pVSBlob));

    hr = pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &psVS);
    if (FAILED(hr))
    {
        SAFE_RELEASE(pVSBlob);
        return hr;
    }
    SAFE_RELEASE(pVSBlob);

    // Compile the shadow hair pixel shader
    ID3DBlob* pPSBlob = nullptr;
    V_RETURN(DXUTCompileFromFile(L"HairShadow.hlsl", nullptr, "PS", "ps_5_0", dwShaderFlags, 0, &pPSBlob));

    hr = pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &psPS);
    if (FAILED(hr))
    {
        SAFE_RELEASE(pPSBlob);
        return hr;
    }
    SAFE_RELEASE(pPSBlob);

    // Compile the shadow map vertex shader
    V_RETURN(DXUTCompileFromFile(L"ShadowMap.hlsl", nullptr, "VS", "vs_5_0", dwShaderFlags, 0, &pVSBlob));

    hr = pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &psmVS);
    if (FAILED(hr))
    {
        SAFE_RELEASE(pVSBlob);
        return hr;
    }
    SAFE_RELEASE(pVSBlob);

    // Compile the shadow map pixel shader
    V_RETURN(DXUTCompileFromFile(L"ShadowMap.hlsl", nullptr, "PS", "ps_5_0", dwShaderFlags, 0, &pPSBlob));

    hr = pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &psmPS);
    if (FAILED(hr))
    {
        SAFE_RELEASE(pPSBlob);
        return hr;
    }
    SAFE_RELEASE(pPSBlob);

    ShadowMapConstBuffer smcbuffer;

    pShadowMap = new RenderTextureClass();
    bool result = pShadowMap->Initialize(pd3dDevice, 1024, 704, 100.0f, 0.1f);
    if (!result)
    {
        assert(0);
        return false;
    }

    XMFLOAT4X4 proj;  pShadowMap->GetOrthoMatrix(proj);
    XMFLOAT3 lightPos = XMFLOAT3(10.0f, 10.0f, 10.0f);
    XMFLOAT3 lightTarget = XMFLOAT3(0.0f, 0.0f, 0.0f);
    XMFLOAT3 lightUp = XMFLOAT3(0.0f, 1.0f, 0.0f);

    XMStoreFloat4x4(&smcbuffer.lightProjViewMatrix, XMMatrixTranspose(XMMatrixLookAtLH(XMLoadFloat3(&lightPos),
        XMLoadFloat3(&lightTarget), XMLoadFloat3(&lightUp))*XMLoadFloat4x4(&proj)));
    lightProjView = smcbuffer.lightProjViewMatrix;
    smcbuffer.colorScheme = colorScheme;

    D3D11_SUBRESOURCE_DATA subRes;
    ZeroMemory(&subRes, sizeof(D3D11_SUBRESOURCE_DATA));
    subRes.pSysMem = &smcbuffer;

    CD3D11_BUFFER_DESC bDesc;
    ZeroMemory(&bDesc, sizeof(CD3D11_BUFFER_DESC));
    bDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bDesc.ByteWidth = sizeof(ShadowMapConstBuffer);
    bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bDesc.Usage = D3D11_USAGE_DYNAMIC;
    V_RETURN(pd3dDevice->CreateBuffer(&bDesc, &subRes, &pCBShadow));

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

const DirectX::XMFLOAT3* HairBiDebugRenderer::getColorBuffer() const
{
    switch (colorScheme)
    {
    case HairBiDebugRenderer::PSEUDO_COLOR:
        return colorSet[PSEUDO_COLOR];
    case HairBiDebugRenderer::GUIDE_COLOR:
        return colorSet[GUIDE_COLOR];
    case HairBiDebugRenderer::GROUP_COLOR:
        return colorSet[GROUP_COLOR];
    case HairBiDebugRenderer::ERROR_COLOR:
        return colorSet[GUIDE_COLOR];
    case HairBiDebugRenderer::DIR_COLOR:
        return nullptr;
    default:
        return nullptr;
    }
}

void HairBiDebugRenderer::initColorSchemes()
{
    colorSet = new XMFLOAT3*[NUM_COLOR_SCHEME];

    size_t nParticle = pHair->n_strands() * N_PARTICLES_PER_STRAND;
    for (size_t i = 0; i < NUM_COLOR_SCHEME; i++)
        colorSet[i] = new DirectX::XMFLOAT3[nParticle];

    for (int i = 0; i < pHair->n_strands(); i++)
    {
        vec3 color;
        wrColorGenerator::genRandSaturatedColor(color);
        for (int j = 0; j < N_PARTICLES_PER_STRAND; j++)
            memcpy(colorSet[PSEUDO_COLOR] + N_PARTICLES_PER_STRAND*i + j, color, sizeof(vec3));
    }

    /* read the guide hair info */
    std::ifstream file(GUIDE_FILE, std::ios::binary);
    //if (!file.is_open())
        //throw std::exception("File not found!");

    int nGuide = 0;
    char buffer[128];
    std::vector<int> guides;

    if (file.is_open())
    {
        file.read(buffer, 4);
        nGuide = *reinterpret_cast<int*>(buffer);

        file.read(buffer, 8);
        std::cout << nGuide << std::endl;
        for (size_t i = 0; i < nGuide; i++)
        {
            file.read(buffer, 4);
            guides.push_back(*reinterpret_cast<int*>(buffer));
        }

        file.close();
    }
    else nGuide = 0;

    /* generate guid hair outstanding scheme */
    for (int i = 0; i < pHair->n_strands(); i++)
    {
        vec3 color{ 1, 1, 1 };
        for (int j = 0; j < N_PARTICLES_PER_STRAND; j++)
            memcpy(colorSet[GUIDE_COLOR] + N_PARTICLES_PER_STRAND*i + j, color, sizeof(vec3));
    }

    vec3 red{ 1.0f, 0.0f, 0.0f };
    for (int id : guides)
    {
        for (size_t i = 0; i < N_PARTICLES_PER_STRAND; i++)
            memcpy(colorSet[GUIDE_COLOR] + N_PARTICLES_PER_STRAND*id + i, red, sizeof(vec3));
    }

    /* read the grouping info */
    file.open(GROUP_FILE, std::ios::binary);
    if (!file.is_open()) throw std::exception("File not found!");

    file.read(buffer, 4);
    int nStrand = *reinterpret_cast<int*>(buffer);

    std::vector<int> groups(nStrand);
    for (size_t i = 0; i < nStrand; i++)
    {
        file.read(buffer, 4);
        groups[i] = (*reinterpret_cast<int*>(buffer));
    }

    file.close();

    /* generate grouping scheme */
    XMFLOAT3* groupColors = new DirectX::XMFLOAT3[nGuide];
    for (int i = 0; i < nGuide; i++)
    {
        vec3 color;
        wrColorGenerator::genRandSaturatedColor(color);
        memcpy(&groupColors[i], color, sizeof(vec3));
    }

    for (int i = 0; i < pHair->n_strands(); i++)
    {
        vec3 color{ 1.0f, 1.0f, 1.0f };
        for (int j = 0; j < N_PARTICLES_PER_STRAND; j++)
            memcpy(colorSet[GROUP_COLOR] + N_PARTICLES_PER_STRAND*i + j, i % 1 == 0 ? (void*)(groupColors + groups[i]) : (void*)(color), sizeof(vec3));
    }

    SAFE_DELETE_ARRAY(groupColors);
}


void HairBiDebugRenderer::nextColorScheme()
{
    colorScheme = static_cast<COLOR_SCHEME>((colorScheme + 1) % NUM_COLOR_SCHEME);
}

void HairBiDebugRenderer::setColorScheme(COLOR_SCHEME s)
{
    HRESULT hr;
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    V(pd3dImmediateContext->Map(pCBShadow, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
    auto pVSPerFrame = reinterpret_cast<ShadowMapConstBuffer*>(MappedResource.pData);
    pVSPerFrame->lightProjViewMatrix = lightProjView;
    pVSPerFrame->colorScheme = s;
    pd3dImmediateContext->Unmap(pCBShadow, 0);
}

