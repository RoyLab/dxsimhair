#include <DXUT.h>
#include "HairManager.h"
#include "HairLoader.h"
#include "HairRenderer.h"
#include "HairColor.h"
#include "BasicRenderer.h"

namespace XRwy
{
    using namespace DirectX;

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

        for (int i = 0; i < 2; i++)
        {
            V(pd3dImmediateContext->Map(pVB[i], 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
            auto pData = reinterpret_cast<XMFLOAT3*>(MappedResource.pData);
            CopyMemory(pData, hair->position, sizeof(XMFLOAT3)* hair->nParticle);
            pd3dImmediateContext->Unmap(pVB[i], 0);
        }
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

        // suppose we have an example
        auto &example = hairManips[0].hair;

        // init color buffer
        ID3D11Buffer* buffer = nullptr;

        BlackHair* black = nullptr;
        bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bDesc.Usage = D3D11_USAGE_DEFAULT;
        bDesc.CPUAccessFlags = 0;
        bDesc.ByteWidth = example->nParticle * sizeof(XMFLOAT3);

        black = new BlackHair(example->nParticle);
        subRes.pSysMem = black->GetColorArray();
        SAFE_DELETE(black);

        V_RETURN(pd3dDevice->CreateBuffer(&bDesc, nullptr, &buffer));
        dataBuffers["black"] = buffer;

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
        SAFE_DELETE(seq);

        V_RETURN(pd3dDevice->CreateBuffer(&bDesc, nullptr, &buffer));
        dataBuffers["seq"] = buffer;

        // load group files

        // load guide information

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
        XMFLOAT4X4 world_o_0(hairManips[0].hair->rigidTrans);
        XMMATRIX world = XMLoadFloat4x4(&world_o_0);
        pMeshRenderer->SetMatrices(XMMatrixTranspose(world), pCamera->GetViewMatrix(), pCamera->GetProjMatrix());
        size_t nodeCount = pFbxLoader->GetNodeCount();

        int j = 2;
        auto material = pFbxLoader->GetNodeMaterial(j);
        pMeshRenderer->SetMaterial(&material);
        pMeshRenderer->SetRenderState();
        pFbxLoader->RenderNode(pd3dImmediateContext, j);

        pd3dImmediateContext->IASetInputLayout(pInputLayout);
        pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
        
        ID3D11Buffer* buffers[5] = { hairManips[0].pVB[0], hairManips[0].pVB[1], dataBuffers["black"], dataBuffers["seq"], hairManips[1].pVB[0] };
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
            hairManips[i].loader->nextFrame();
            hairManips[i].sync = false;

            hairManips[i].UpdateBuffers(pd3dImmediateContext);
            hairManips[i].sync = true;
        }
    }

}