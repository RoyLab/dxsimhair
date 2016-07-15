#include <DXUT.h>
#include <cstdlib>
#include "HairManager.h"
#include "HairLoader.h"
#include "HairRenderer.h"
#include "HairColor.h"
#include "BasicRenderer.h"

namespace
{
	enum COLOR_SCHEME { CGREY, CRANDOM, CGUIDE, NUM_OF_COLOR_SCHEME };

	const char COLOR_SEMATICS[NUM_OF_COLOR_SCHEME][16] =
	{
		"COLOR_grey", "COLOR_random", "COLOR_guide"
	};
}

namespace XRwy
{
    using namespace DirectX;

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

        SGeoManip geoManip;
        ZeroMemory(&geoManip, sizeof(SGeoManip));

        D3D11_BUFFER_DESC bDesc;
        ZeroMemory(&bDesc, sizeof(D3D11_BUFFER_DESC));

        D3D11_SUBRESOURCE_DATA subRes;
        ZeroMemory(&subRes, sizeof(D3D11_SUBRESOURCE_DATA));

        bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bDesc.Usage = D3D11_USAGE_DYNAMIC;
        bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		std::vector<std::string> animFiles;
		int n = std::atoi(g_paramDict["numanim"].c_str());
		animFiles.emplace_back(g_paramDict["reffile"]);

		char chs[4];
		for (int i = 1; i < n; i++)
		{
			std::string key("animfile");
			key += itoa(i, chs, 10);
			animFiles.emplace_back(g_paramDict[key]);
		}

        for (int i = 0; i < n; i++)
        {
            geoManip.loader = new HairAnimationLoader;
            geoManip.hair = new HairGeometry;

            geoManip.loader->loadFile(animFiles[i].c_str(), geoManip.hair);
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

        // init color buffer
        IHairColorGenerator* color = nullptr;
        bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bDesc.Usage = D3D11_USAGE_DEFAULT;
        bDesc.CPUAccessFlags = 0;
        bDesc.ByteWidth = example->nParticle * sizeof(XMFLOAT3);

        color = new GreyHair(example->nParticle, 0x10);
        subRes.pSysMem = color->GetColorArray();
        V_RETURN(pd3dDevice->CreateBuffer(&bDesc, &subRes, &buffer));
        dataBuffers[COLOR_SEMATICS[CGREY]] = buffer;
        SAFE_DELETE(color);

        color = new RandomColorHair(example->nParticle, example->particlePerStrand, genRandSaturatedColor);
        subRes.pSysMem = color->GetColorArray();
        V_RETURN(pd3dDevice->CreateBuffer(&bDesc, &subRes, &buffer));
        dataBuffers[COLOR_SEMATICS[CRANDOM]] = buffer;
        SAFE_DELETE(color);

		SetupContents();

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

    void HairManager::RenderInstance(CModelViewerCamera* pCamera, int hairId, double fTime, float fElapsedTime)
    {
		pd3dImmediateContext->GSSetShader(nullptr, nullptr, 0);

		auto &frame = contents[hairId];
		XMMATRIX world = ComputeHeadTransformation(hairManips[frame.animID].hair->rigidTrans);
        pMeshRenderer->SetMatrices(world, pCamera->GetViewMatrix(), pCamera->GetProjMatrix());

        int j = 2;
        auto material = pFbxLoader->GetNodeMaterial(j);
        pMeshRenderer->SetMaterial(&material);
        pMeshRenderer->SetRenderState();
		pFbxLoader->RenderNode(pd3dImmediateContext, j);

        // render hairs
        pd3dImmediateContext->IASetIndexBuffer(dataBuffers["indices"], DXGI_FORMAT_R32_UINT, 0);

        HairRenderer::ConstBuffer bf;
        bf.mode = frame.rendMode;
        XMStoreFloat3(&bf.viewPoint, pCamera->GetEyePt());
        XMStoreFloat4x4(&bf.projViewWorld, DirectX::XMMatrixTranspose(pCamera->GetViewMatrix() * pCamera->GetProjMatrix()));

        XMFLOAT4X4 proj;
        pHairRenderer->GetShadowMapProjMatrix(proj);
        XMFLOAT3 lightPos = XMFLOAT3(10.0f, 10.0f, -10.0f);
        XMFLOAT3 lightTarget = XMFLOAT3(0.0f, 0.0f, 0.0f);
        XMFLOAT3 lightUp = XMFLOAT3(0.0f, 1.0f, 0.0f);

        XMStoreFloat4x4(&bf.lightProjViewWorld, DirectX::XMMatrixTranspose(XMMatrixLookAtLH(XMLoadFloat3(&lightPos),
            XMLoadFloat3(&lightTarget), XMLoadFloat3(&lightUp))*XMLoadFloat4x4(&proj)));

        pHairRenderer->SetConstantBuffer(&bf);

        pd3dImmediateContext->IASetInputLayout(pInputLayout);
        ID3D11Buffer* buffers[5] = { hairManips[frame.animID].pVB[0], hairManips[frame.animID].pVB[1], dataBuffers[COLOR_SEMATICS[frame.colorID]], dataBuffers["seq"], hairManips[0].pVB[0] };
        UINT strides[5] = { sizeof(XMFLOAT3), sizeof(XMFLOAT3), sizeof(XMFLOAT3), sizeof(int), sizeof(XMFLOAT3) };
        UINT offsets[5] = { 0 };
        pd3dImmediateContext->IASetVertexBuffers(0, 5, buffers, strides, offsets);
        pHairRenderer->SetRenderState(0);
        drawStrand(pd3dImmediateContext, 0, hairManips[frame.animID].hair->nParticle, &hairManips[frame.animID].hair->particlePerStrand);
        pHairRenderer->SetRenderState(1);
        drawStrand(pd3dImmediateContext, 0, hairManips[frame.animID].hair->nParticle, &hairManips[frame.animID].hair->particlePerStrand);

		if (activeContentId == hairId)
		{
			g_pTxtHelper->Begin();
			g_pTxtHelper->SetInsertionPos(5, 100);
			g_pTxtHelper->SetForegroundColor(Colors::Red);
			g_pTxtHelper->DrawTextLine(L"Activated");
			g_pTxtHelper->End();
		}
    }

    void HairManager::OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
    {
		if (bAnim)
		{
			for (int i = 0; i < hairManips.size(); i++)
			{
				hairManips[i].loader->nextFrame();
				hairManips[i].sync = false;

				hairManips[i].UpdateBuffers(pd3dImmediateContext);
				hairManips[i].sync = true;
			}
		}
    }

	void HairManager::toggleAnimation()
	{
		bAnim = !bAnim;
	}

	void HairManager::toogleDiffDisp()
	{
		auto& mode = contents[activeContentId].rendMode;
		mode = (mode == 0) ? 1 : 0;
	}

	void HairManager::SetupContents()
	{
		int n = std::stoi(g_paramDict["numframe"]);
		contents.resize(n);
		char buffer[4];
		for (int i = 0; i < n; i++)
		{
			std::string str("frame");
			str += itoa(i, buffer, 10);
			contents[i].animID = std::stoi(g_paramDict[str]);
			contents[i].colorID = CRANDOM;
			contents[i].rendMode = 0;
		}
	}

}