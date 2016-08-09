#include "SkinningEngine.h"

#include <DXUT.h>
#include <cstdlib>
#include <cctype>
#include <algorithm>
#include "HairManager.h"
#include "HairLoader.h"
#include "HairRenderer.h"
#include "HairColor.h"
#include "BasicRenderer.h"
#include "FollicleRenderer.h"
#include "ZhouHairLoader.hpp"

namespace
{
	enum COLOR_SCHEME { CGREY, CRANDOM, CDIR, NUM_COLOR_SCHEME };
	enum DISPLAY_MASK { DISP_HEAD = 0x01, DISP_FOLLICLE = 0x02, DISP_HAIR = 0x04 };

	const char COLOR_SEMATICS[NUM_COLOR_SCHEME][16] =
	{
		"COLOR_grey", "COLOR_random", "COLOR_guide"
	};
}

namespace XRwy
{
    using namespace DirectX;

	struct DrawPara
	{
		int lineSeg;
		int base;
		int factor;
	};

    XMMATRIX ComputeHeadTransformation(const float* trans4x4)
    {
        using namespace DirectX;
        auto target0 = XMFLOAT4X4(trans4x4);
        auto trans0 = XMFLOAT3(0.0f, -0.643f, 0.282f);
        auto scale0 = XMFLOAT3(5.346f, 5.346f, 5.346f);
        return XMMatrixAffineTransformation(XMLoadFloat3(&scale0), XMVectorZero(), XMVectorZero(), XMLoadFloat3(&trans0))*XMMatrixTranspose(XMLoadFloat4x4(&target0));
    }

	HairLoader* CreateHairLoader(const char* fileName, HairGeometry* geom, void* para)
	{
		std::string name(fileName);
		int last = name.rfind('.');
		std::string posfix = name.substr(last);
		std::transform(posfix.begin(), posfix.end(), posfix.begin(), [](unsigned char c) { return std::tolower(c); });

		HairLoader* result = nullptr;
		if (posfix == ".anim2")
			result = new HairAnimationLoader;
		else if (posfix == ".hair")
			result = new ZhouHairLoader;
		else if (posfix == ".recons")
			result = new ReducedModel(*reinterpret_cast<int*>(para));

		if (result)
			result->loadFile(fileName, geom);

		return result;
	}

    void drawStrand(ID3D11DeviceContext* context, int start, int num, void* drawPara)
    {
        auto para = reinterpret_cast<DrawPara*>(drawPara);

        assert(num % para->factor == 0);
        int nStrand = num / para->factor;

        context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP);
        int ptr = start;
        for (int i = 0; i < nStrand; i++, ptr += para->factor)
            context->DrawIndexed(1 + para->lineSeg, ptr, para->base);
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

		if (hair->direction)
		{
			V(pd3dImmediateContext->Map(pVB[1], 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
			CopyMemory(MappedResource.pData, hair->direction, sizeof(XMFLOAT3)* hair->nParticle);
			pd3dImmediateContext->Unmap(pVB[1], 0);
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

		pFollicleRenderer = new FollicleRenderer;
		V_RETURN(pFollicleRenderer->Initialize());

		nDisplayBase = std::atoi(g_paramDict["linesegbase"].c_str());

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
        D3D11_BUFFER_DESC bDesc;
        D3D11_SUBRESOURCE_DATA subRes;

        ZeroMemory(&geoManip, sizeof(SGeoManip));
        ZeroMemory(&bDesc, sizeof(D3D11_BUFFER_DESC));
        ZeroMemory(&subRes, sizeof(D3D11_SUBRESOURCE_DATA));

        bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bDesc.Usage = D3D11_USAGE_DYNAMIC;
        bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		struct AnimItem { std::string name; int para; } aitem;
		std::vector<AnimItem> animFiles;
		int n = std::atoi(g_paramDict["numanim"].c_str());
		aitem.name = g_paramDict["reffile"];
		aitem.para = std::stoi(g_paramDict["para0"], 0, 2);
		animFiles.push_back(aitem);

		char chs[4];
		for (int i = 1; i < n; i++)
		{
			std::string key("animfile"), key2("para");
			key += itoa(i, chs, 10); key2 += itoa(i, chs, 10);
			animFiles.push_back(AnimItem{ g_paramDict[key], std::stoi(g_paramDict[key2], 0, 2) });
		}

        for (int i = 0; i < n; i++)
        {
            geoManip.hair = new HairGeometry;
            geoManip.loader = CreateHairLoader(animFiles[i].name.c_str(), geoManip.hair, &(animFiles[i].para));
            bDesc.ByteWidth = geoManip.hair->nParticle * sizeof(XMFLOAT3);

            V_RETURN(pd3dDevice->CreateBuffer(&bDesc, nullptr, &geoManip.pVB[0]));
            V_RETURN(pd3dDevice->CreateBuffer(&bDesc, nullptr, &geoManip.pVB[1]));

            hairManips.push_back(geoManip);
        }

        // suppose the two animation are for the same hair
		int ei = 0;
		int maxnHair = 0;
		for (int i = 0; i < hairManips.size(); i++)
		{
			if (maxnHair < hairManips[i].hair->nParticle)
			{
				maxnHair = hairManips[i].hair->nParticle;
				ei = i;
			}
		}
        auto &example = hairManips[ei].hair;
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

		// create follicle index buffer
		bDesc.ByteWidth = example->nStrand * sizeof(DWORD);

		indices = new DWORD[example->nStrand];
		for (int i = 0; i < example->nStrand; i++)
			indices[i] = i * example->particlePerStrand;

		subRes.pSysMem = indices;
		V_RETURN(pd3dDevice->CreateBuffer(&bDesc, &subRes, &buffer));
		dataBuffers["follicleindices"] = buffer;
		SAFE_DELETE_ARRAY(indices);

        // init sequence buffer
        bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bDesc.Usage = D3D11_USAGE_DEFAULT;
        bDesc.CPUAccessFlags = 0;
        bDesc.ByteWidth = example->nParticle * sizeof(int);

        int* seq = new int[example->nParticle];
        for (int i = 0; i < example->nParticle;)
        {
			seq[i++] = i;
            //for (int j = 0; j < example->particlePerStrand; j++)
            //    seq[i++] = j;
        }
        subRes.pSysMem = seq;
        V_RETURN(pd3dDevice->CreateBuffer(&bDesc, &subRes, &buffer));
        dataBuffers["seq"] = buffer;
        SAFE_DELETE(seq);

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
		SAFE_RELEASE(pFollicleRenderer);
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
		auto& hair = hairManips[frame.animID].hair;

		if (frame.displayMask & DISP_HEAD)
		{
			XMMATRIX world = ComputeHeadTransformation(hair->rigidTrans);
			pMeshRenderer->SetMatrices(world, pCamera->GetViewMatrix(), pCamera->GetProjMatrix());

			int j = 2;
			auto material = pFbxLoader->GetNodeMaterial(j);
			pMeshRenderer->SetMaterial(&material);
			pMeshRenderer->SetRenderState();
			pFbxLoader->RenderNode(pd3dImmediateContext, j);
		}


        // render hairs
		if (frame.displayMask & DISP_HAIR)
		{
			pd3dImmediateContext->IASetIndexBuffer(dataBuffers["indices"], DXGI_FORMAT_R32_UINT, 0);

			HairRenderer::ConstBuffer bf;
			bf.mode = frame.rendMode;
			XMStoreFloat3(&bf.viewPoint, pCamera->GetEyePt());
			XMStoreFloat4x4(&bf.projViewWorld, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&hair->worldMatrix) *
				pCamera->GetViewMatrix() * pCamera->GetProjMatrix()));

			XMFLOAT4X4 proj;
			pHairRenderer->GetShadowMapProjMatrix(proj);
			XMFLOAT3 lightPos = XMFLOAT3(10.0f, 10.0f, -10.0f);
			XMFLOAT3 lightTarget = XMFLOAT3(0.0f, 0.0f, 0.0f);
			XMFLOAT3 lightUp = XMFLOAT3(0.0f, 1.0f, 0.0f);

			XMStoreFloat4x4(&bf.lightProjViewWorld, DirectX::XMMatrixTranspose(XMMatrixLookAtLH(XMLoadFloat3(&lightPos),
				XMLoadFloat3(&lightTarget), XMLoadFloat3(&lightUp))*XMLoadFloat4x4(&proj)));

			pHairRenderer->SetConstantBuffer(&bf);
			pd3dImmediateContext->IASetInputLayout(pInputLayout);

			ID3D11Buffer* colorBuffer;
			if (frame.colorID == CDIR)
				colorBuffer = hairManips[frame.animID].pVB[1];
			else
				colorBuffer = dataBuffers[COLOR_SEMATICS[frame.colorID]];

			ID3D11Buffer* buffers[5] = { hairManips[frame.animID].pVB[0], hairManips[frame.animID].pVB[1],
				colorBuffer, dataBuffers["seq"], hairManips[0].pVB[0] };
			UINT strides[5] = { sizeof(XMFLOAT3), sizeof(XMFLOAT3), sizeof(XMFLOAT3), sizeof(int), sizeof(XMFLOAT3) };
			UINT offsets[5] = { 0 };
			pd3dImmediateContext->IASetVertexBuffers(0, 5, buffers, strides, offsets);

			DrawPara para = { bFullShow ? hair->particlePerStrand - 1 : std::atoi(g_paramDict["lineseg"].c_str()),
				bFullShow ? 0 : nDisplayBase, hair->particlePerStrand };

			pHairRenderer->SetRenderState(0);
			drawStrand(pd3dImmediateContext, 0, hair->nParticle, &para);
			pHairRenderer->SetRenderState(1);
			drawStrand(pd3dImmediateContext, 0, hair->nParticle, &para);
		}

		// render follicles
		if (frame.displayMask & DISP_FOLLICLE)
		{
			FollicleRenderer::ConstBuffer bf2;
			XMStoreFloat3(&bf2.viewPoint, pCamera->GetEyePt());
			XMStoreFloat4x4(&bf2.projViewWorld, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&hair->worldMatrix) *
				pCamera->GetViewMatrix() * pCamera->GetProjMatrix()));
			bf2.pointSize = std::stof(g_paramDict["pointsize"]);

			pFollicleRenderer->SetConstantBuffer(&bf2);
			pFollicleRenderer->SetRenderState();

			ID3D11Buffer* colorBuffer;
			if (frame.colorID == CDIR)
				colorBuffer = hairManips[frame.animID].pVB[1];
			else
				colorBuffer = dataBuffers[COLOR_SEMATICS[frame.colorID]];
			ID3D11Buffer* buffers[5] = { hairManips[frame.animID].pVB[0], hairManips[frame.animID].pVB[1],
				colorBuffer, dataBuffers["seq"], hairManips[0].pVB[0] };
			UINT strides[5] = { sizeof(XMFLOAT3), sizeof(XMFLOAT3), sizeof(XMFLOAT3), sizeof(int), sizeof(XMFLOAT3) };
			UINT offsets[5] = { 0 };
			pd3dImmediateContext->IASetVertexBuffers(0, 5, buffers, strides, offsets);
			pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
			pd3dImmediateContext->IASetInputLayout(pInputLayout);
			pd3dImmediateContext->IASetIndexBuffer(dataBuffers["follicleindices"], DXGI_FORMAT_R32_UINT, 0);
			pd3dImmediateContext->DrawIndexed(hair->nStrand, 0, 0);
		}


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
		bool force = false;
		if (pUserContext && *reinterpret_cast<int*>(pUserContext) == 1)
			force = true;

		if (bAnim || force)
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

	void HairManager::toggleDiffDisp()
	{
		auto& mode = contents[activeContentId].rendMode;
		mode = (mode == 0) ? 1 : 0;
	}

	void HairManager::ChangeColorScheme(int i)
	{
		contents[activeContentId].colorID = i > NUM_COLOR_SCHEME ? NUM_COLOR_SCHEME - 1 : i - 1;
	}

	void HairManager::toggleDisp(char item)
	{
		switch (item)
		{
		case 'f':
			contents[activeContentId].displayMask ^= DISP_FOLLICLE;
			break;
		case 'm':
			contents[activeContentId].displayMask ^= DISP_HEAD;
			break;
		case 'h':
			contents[activeContentId].displayMask ^= DISP_HAIR;
			break;
		default:
			break;
		}
	}

	void HairManager::ChangeDrawBase(bool open, int incre)
	{
		if (open)
			bFullShow = !bFullShow;

		if (incre)
			nDisplayBase = (nDisplayBase + hairManips[0].hair->particlePerStrand - 1 + incre) % (hairManips[0].hair->particlePerStrand - 1);
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
			contents[i].displayMask = std::stoi(g_paramDict["dispmask"], nullptr, 16);
		}
	}

}