#include <DXUT.h>
#include "HairManager.h"
#include "HairLoader.h"
#include "HairRenderer.h"
#include "HairColor.h"
#include "BasicRenderer.h"
#include "D3D11Tools.h"


namespace XRwy
{
    using namespace DirectX;

    DebugBuffers gbuffers;
    LineRenderer gL;
    ID3D11InputLayout* pL;

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
        layout[0].AlignedByteOffset = 0;

        layout[1].InputSlot = 0;
        layout[1].AlignedByteOffset = 0;

        layout[2].InputSlot = 2;
        layout[2].AlignedByteOffset = 0;

        layout[3].InputSlot = 1;
        layout[3].AlignedByteOffset = 0;

        layout[4].InputSlot = 4;
        layout[4].AlignedByteOffset = 0;
        
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

        BlackHair* black = nullptr;
        bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bDesc.Usage = D3D11_USAGE_DEFAULT;
        bDesc.CPUAccessFlags = 0;
        bDesc.ByteWidth = example->nParticle * sizeof(XMFLOAT3);

        black = new BlackHair(example->nParticle);
        subRes.pSysMem = black->GetColorArray();

        V_RETURN(pd3dDevice->CreateBuffer(&bDesc, &subRes, &buffer));
        dataBuffers["black"] = buffer;
        SAFE_DELETE(black);

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
        gL.Initialize();

        CopyMemory(layout, LineRenderer::LayoutDesc, sizeof(D3D11_INPUT_ELEMENT_DESC)* 2);

        layout[0].InputSlot = 0;
        layout[0].AlignedByteOffset = 0;

        layout[1].InputSlot = 1;
        layout[1].AlignedByteOffset = 0;

        gL.GetVertexShaderBytecode(&pVSBufferPtr, &nVSBufferSz, 0);
        V_RETURN(pd3dDevice->CreateInputLayout(layout, 1, pVSBufferPtr, nVSBufferSz, &pL));

        hairManips[0].loader->nextFrame();
        SetupDebugBuffers(&gbuffers);
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

        ReleaseDebugBuffers(&gbuffers);
        delete this;
    }

    void HairManager::RenderAll(CModelViewerCamera* pCamera, double fTime, float fElapsedTime)
    {
        XMFLOAT4X4 world_o_0(hairManips[0].hair->rigidTrans);
        XMMATRIX world = XMLoadFloat4x4(&world_o_0);
        pMeshRenderer->SetMatrices(XMMatrixTranspose(world), pCamera->GetViewMatrix(), pCamera->GetProjMatrix());
        size_t nodeCount = pFbxLoader->GetNodeCount();

        int j = 2;
        auto material = pFbxLoader->GetNodeMaterial(j);
        pMeshRenderer->SetMaterial(&material);
        pMeshRenderer->SetRenderState();
        pFbxLoader->RenderNode(pd3dImmediateContext, j);

        // render hairs
        LineRenderer::ConstantBuffer bf;
        XMStoreFloat4x4(&bf.viewProjMatrix, XMMatrixTranspose(pCamera->GetViewMatrix() * pCamera->GetProjMatrix()));
        XMStoreFloat4x4(&bf.worldMatrix, XMMatrixTranspose(pCamera->GetProjMatrix() * pCamera->GetViewMatrix()));
        gL.SetConstantBuffer(&bf);
        gL.SetRenderState();
        pd3dImmediateContext->IASetInputLayout(pL);


        UINT strides2[] = { sizeof(XMFLOAT3) };
        UINT offsets2[] = { 0 };
        pd3dImmediateContext->IASetVertexBuffers(0, 1, &gbuffers.pVB, strides2, offsets2);
        pd3dImmediateContext->IASetIndexBuffer(gbuffers.pIB, DXGI_FORMAT_R32_UINT, 0);
        pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        //pd3dImmediateContext->DrawIndexed(3, 0, 0);

        ID3D11Buffer* buffers[5] = { hairManips[0].pVB[0], dataBuffers["black"] };
        UINT strides[5] = { sizeof(XMFLOAT3), sizeof(XMFLOAT3), sizeof(XMFLOAT3), sizeof(int), sizeof(XMFLOAT3) };
        UINT offsets[5] = { 0 };
        pd3dImmediateContext->IASetVertexBuffers(0, 1, &hairManips[0].pVB[0], strides, offsets);
        pd3dImmediateContext->IASetIndexBuffer(dataBuffers["indices"], DXGI_FORMAT_R32_UINT, 0);
        drawStrand(pd3dImmediateContext, 0, hairManips[0].hair->nParticle, &hairManips[0].hair->particlePerStrand);
        int start = 0, num = hairManips[0].hair->nParticle; void* perStrand = &hairManips[0].hair->particlePerStrand;
        int factor = *reinterpret_cast<int*>(perStrand);

        assert(num % factor == 0);
        int nStrand = num / factor;

        pd3dImmediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP);
        int ptr = start;
        for (int i = 0; i < nStrand; i++, ptr += factor)
            pd3dImmediateContext->DrawIndexed(factor, ptr, 0);
        //pd3dImmediateContext->DrawIndexed(3, 0, 0);
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