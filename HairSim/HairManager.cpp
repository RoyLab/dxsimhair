#include <DXUT.h>
#include "HairManager.h"
#include "HairLoader.h"
#include "HairRenderer.h"
#include "HairColor.h"
#include "BasicRenderer.h"

namespace XRwy
{
    using namespace DirectX;

    LineRenderer gLineRenderer;
    ID3D11InputLayout* pL;

    XMMATRIX ComputeHeadTransformation(const float* trans4x4)
    {
        using namespace DirectX;
        auto target0 = XMFLOAT4X4(trans4x4);
        auto trans0 = XMFLOAT3(0.0f, -0.643f, 0.282f);
        auto scale0 = XMFLOAT3(5.346f, 5.346f, 5.346f);
        return XMMatrixAffineTransformation(XMLoadFloat3(&scale0), XMVectorZero(), XMVectorZero(), XMLoadFloat3(&trans0))*XMMatrixTranspose(XMLoadFloat4x4(&target0));
    }

    void drawStrand(ID3D11DeviceContext* context, int start, int num, void* perStrand)
    {
        int factor = *reinterpret_cast<int*>(perStrand);

        assert(num % factor == 0);
        int nStrand = num / factor;

        context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP);
        int ptr = start;
        for (int i = 0; i < nStrand; i++, ptr += factor)
            context->DrawIndexed(factor, ptr, 0);
    }

    void HairManager::SGeoManip::Release()
    {
        SAFE_DELETE(loader);
        SAFE_DELETE(hair);
        SAFE_RELEASE(pVB[0]);
        SAFE_RELEASE(pVB[1]);
    }

    void HairManager::SGeoManip::UpdateBuffers(ID3D11DeviceContext* pd3dImmediateContext)
    {
        HRESULT hr;
        D3D11_MAPPED_SUBRESOURCE MappedResource;

        V(pd3dImmediateContext->Map(pVB[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
        CopyMemory(MappedResource.pData, hair->position, sizeof(XMFLOAT3)* hair->nParticle);
        pd3dImmediateContext->Unmap(pVB[0], 0);

        V(pd3dImmediateContext->Map(pVB[1], 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
        CopyMemory(MappedResource.pData, hair->direction, sizeof(XMFLOAT3)* hair->nParticle);
        pd3dImmediateContext->Unmap(pVB[1], 0);
    }

    HairManager::HairManager(FBX_LOADER::CFBXRenderDX11* fbx, MeshRenderer* meshrend):
        pFbxLoader(fbx), pMeshRenderer(meshrend)
    {
    }

    bool HairManager::Initialize()
    {
        pd3dDevice = DXUTGetD3D11Device();
        pd3dImmediateContext = DXUTGetD3D11DeviceContext();

        HRESULT hr;
        const void* pVSBufferPtr = nullptr;
        size_t nVSBufferSz = 0;

        pHairRenderer = new HairRenderer;
        V_RETURN(pHairRenderer->Initialize());

        // create input layout
        D3D11_INPUT_ELEMENT_DESC layout[5];
        UINT numElements = ARRAYSIZE(layout);
        CopyMemory(layout, HairRenderer::LayoutDesc, sizeof(D3D11_INPUT_ELEMENT_DESC) * numElements);

        layout[0].InputSlot = 3;
        layout[1].InputSlot = 0;
        layout[2].InputSlot = 2;
        layout[3].InputSlot = 1;
        layout[4].InputSlot = 4;
        
        pHairRenderer->GetVertexShaderBytecode(&pVSBufferPtr, &nVSBufferSz, 0);
        V_RETURN(pd3dDevice->CreateInputLayout(layout, numElements, pVSBufferPtr, nVSBufferSz, &pInputLayout));

        // load hair animaitions
        const char animFiles[2][128] = {
            "C:/data/c0514.anim2",
            "C:/data/c0524-opt-05-28 04h27m57s.anim2",
        };

        SGeoManip geoManip;
        ZeroMemory(&geoManip, sizeof(SGeoManip));

        D3D11_BUFFER_DESC bDesc;
        ZeroMemory(&bDesc, sizeof(D3D11_BUFFER_DESC));

        D3D11_SUBRESOURCE_DATA subRes;
        ZeroMemory(&subRes, sizeof(D3D11_SUBRESOURCE_DATA));

        bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bDesc.Usage = D3D11_USAGE_DYNAMIC;
        bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        for (int i = 0; i < 2; i++)
        {
            geoManip.loader = new HairAnimationLoader;
            geoManip.hair = new HairGeometry;

            geoManip.loader->loadFile(animFiles[i], geoManip.hair);
            bDesc.ByteWidth = geoManip.hair->nParticle * sizeof(XMFLOAT3);

            V_RETURN(pd3dDevice->CreateBuffer(&bDesc, nullptr, &geoManip.pVB[0]));
            V_RETURN(pd3dDevice->CreateBuffer(&bDesc, nullptr, &geoManip.pVB[1]));

            hairManips.push_back(geoManip);
        }

        // suppose the two animation are for the same hair
        auto &example = hairManips[0].hair;
        ID3D11Buffer* buffer = nullptr;

        // create index buffer
        bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bDesc.Usage = D3D11_USAGE_DEFAULT;
        bDesc.CPUAccessFlags = 0;
        bDesc.ByteWidth = example->nParticle * sizeof(DWORD);

        DWORD* indices = new DWORD[example->nParticle];
        for (int i = 0; i < example->nParticle; i++)
            indices[i] = i;

        subRes.pSysMem = indices;
        V_RETURN(pd3dDevice->CreateBuffer(&bDesc, &subRes, &buffer));
        dataBuffers["indices"] = buffer;
        SAFE_DELETE_ARRAY(indices);

        // init color buffer
        IHairColorGenerator* color = nullptr;
        bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bDesc.Usage = D3D11_USAGE_DEFAULT;
        bDesc.CPUAccessFlags = 0;
        bDesc.ByteWidth = example->nParticle * sizeof(XMFLOAT3);

        color = new GreyHair(example->nParticle, 0x10);
        subRes.pSysMem = color->GetColorArray();
        V_RETURN(pd3dDevice->CreateBuffer(&bDesc, &subRes, &buffer));
        dataBuffers["grey"] = buffer;
        SAFE_DELETE(color);

        // init sequence buffer
        bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bDesc.Usage = D3D11_USAGE_DEFAULT;
        bDesc.CPUAccessFlags = 0;
        bDesc.ByteWidth = example->nParticle * sizeof(int);

        int* seq = new int[example->nParticle];
        for (int i = 0; i < example->nParticle;)
        {
            for (int j = 0; j < example->particlePerStrand; j++)
                seq[i++] = j;
        }
        subRes.pSysMem = seq;
        V_RETURN(pd3dDevice->CreateBuffer(&bDesc, &subRes, &buffer));
        dataBuffers["seq"] = buffer;
        SAFE_DELETE(seq);

        // load group files

        // load guide information
        gLineRenderer.Initialize();
        
        CopyMemory(layout, LineRenderer::LayoutDesc, sizeof(D3D11_INPUT_ELEMENT_DESC)* 2);

        layout[0].InputSlot = 0;
        layout[1].InputSlot = 1;

        gLineRenderer.GetVertexShaderBytecode(&pVSBufferPtr, &nVSBufferSz, 0);
        V_RETURN(pd3dDevice->CreateInputLayout(layout, 2, pVSBufferPtr, nVSBufferSz, &pL));

        hairManips[0].loader->nextFrame();
        hairManips[1].loader->nextFrame();
        return true;
    }

    void HairManager::Release()
    {
        SAFE_RELEASE(pInputLayout);
        SAFE_RELEASE(pHairRenderer);
        for (auto &manip : hairManips)
            manip.Release();

        for (auto &buffer : dataBuffers)
            SAFE_RELEASE(buffer.second);

        delete this;
    }

    void HairManager::RenderAll(CModelViewerCamera* pCamera, double fTime, float fElapsedTime)
    {
        XMMATRIX world = ComputeHeadTransformation(hairManips[0].hair->rigidTrans);
        pMeshRenderer->SetMatrices(world, pCamera->GetViewMatrix(), pCamera->GetProjMatrix());
        size_t nodeCount = pFbxLoader->GetNodeCount();

        int j = 2;
        auto material = pFbxLoader->GetNodeMaterial(j);
        pMeshRenderer->SetMaterial(&material);
        pMeshRenderer->SetRenderState();
        pFbxLoader->RenderNode(pd3dImmediateContext, j);

        // render hairs
        pd3dImmediateContext->IASetIndexBuffer(dataBuffers["indices"], DXGI_FORMAT_R32_UINT, 0);

        //pd3dImmediateContext->IASetInputLayout(pL);
        //ID3D11Buffer* buffers[] = { hairManips[0].pVB[0], dataBuffers["grey"] };
        //UINT strides[] = { sizeof(XMFLOAT3), sizeof(XMFLOAT3) };
        //UINT offsets[] = { 0, 0 };

        //LineRenderer::ConstantBuffer bf;
        //XMStoreFloat4x4(&bf.viewProjMatrix, XMMatrixTranspose(pCamera->GetViewMatrix() * pCamera->GetProjMatrix()));
        //gLineRenderer.SetConstantBuffer(&bf);
        //gLineRenderer.SetRenderState();
        //pd3dImmediateContext->IASetVertexBuffers(0, 2, buffers, strides, offsets);
        //drawStrand(pd3dImmediateContext, 0, hairManips[0].hair->nParticle, &hairManips[0].hair->particlePerStrand);

        HairRenderer::ConstBuffer bf;
        bf.mode = 0;
        XMStoreFloat3(&bf.viewPoint, pCamera->GetEyePt());
        XMStoreFloat4x4(&bf.projViewWorld, XMMatrixTranspose(pCamera->GetViewMatrix() * pCamera->GetProjMatrix()));

        XMFLOAT4X4 proj;
        pHairRenderer->GetShadowMapProjMatrix(proj);
        XMFLOAT3 lightPos = XMFLOAT3(10.0f, 10.0f, -10.0f);
        XMFLOAT3 lightTarget = XMFLOAT3(0.0f, 0.0f, 0.0f);
        XMFLOAT3 lightUp = XMFLOAT3(0.0f, 1.0f, 0.0f);

        XMStoreFloat4x4(&bf.lightProjViewWorld, XMMatrixTranspose(XMMatrixLookAtLH(XMLoadFloat3(&lightPos),
            XMLoadFloat3(&lightTarget), XMLoadFloat3(&lightUp))*XMLoadFloat4x4(&proj)));

        pHairRenderer->SetConstantBuffer(&bf);

        pd3dImmediateContext->IASetInputLayout(pInputLayout);
        ID3D11Buffer* buffers[5] = { hairManips[0].pVB[0], hairManips[0].pVB[1], dataBuffers["grey"], dataBuffers["seq"], hairManips[1].pVB[0] };
        UINT strides[5] = { sizeof(XMFLOAT3), sizeof(XMFLOAT3), sizeof(XMFLOAT3), sizeof(int), sizeof(XMFLOAT3) };
        UINT offsets[5] = { 0 };
        pd3dImmediateContext->IAGetVertexBuffers(0, 5, buffers, strides, offsets);
        pHairRenderer->SetRenderState(0);
        drawStrand(pd3dImmediateContext, 0, hairManips[0].hair->nParticle, &hairManips[0].hair->particlePerStrand);
        pHairRenderer->SetRenderState(1);
        drawStrand(pd3dImmediateContext, 0, hairManips[0].hair->nParticle, &hairManips[0].hair->particlePerStrand);
    }

    void HairManager::OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
    {
        for (int i = 0; i < 2; i++)
        {
            //hairManips[i].loader->nextFrame();
            hairManips[i].sync = false;

            hairManips[i].UpdateBuffers(pd3dImmediateContext);
            hairManips[i].sync = true;
        }
    }

}