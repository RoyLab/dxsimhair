#include "precompiled.h"
#include "wrSceneManager.h"
#include "wrHairRenderer.h"
#include "wrHairSimulator.h"
#include "wrMeshRenderer.h"
#include <GeometricPrimitive.h>
#include <DXUTcamera.h>
#include "wrTypes.h"
#include "Parameter.h"



using namespace DirectX;

//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------
#pragma pack(push,1)

struct CB_VS_PER_FRAME
{
    XMFLOAT4X4  mViewProjection;
    XMFLOAT4X4  mWorld;

    float       time;

    XMFLOAT3    lightDir;
    XMFLOAT4    lightDiffuse;
    XMFLOAT4    lightAmbient;
};
#pragma pack(pop)


wrSceneManager::wrSceneManager()
{
}


wrSceneManager::~wrSceneManager()
{
}


bool wrSceneManager::init()
{
    pd3dDevice = DXUTGetD3D11Device();
    pd3dImmediateContext = DXUTGetD3D11DeviceContext();

    initConstantBuffer();
	init_global_param();

    pHair = WR::loadFile(L"../../models/curly.hair");
	pHair->scale(0.01f);
	pHair->mirror(false, true, false);

	WR::HairStrand::set_hair(pHair);
	WR::HairParticle::set_hair(pHair);
	pHair->init_simulation();

    HRESULT hr;
    pHairRenderer = new wrHairRenderer(*pHair);
    V_RETURN(pHairRenderer->init());

    pMeshRenderer = new wrMeshRenderer;
    pMeshRenderer->setCamera(pCamera);
    V_RETURN(pMeshRenderer->init());

    return true;
}


void wrSceneManager::onFrame(double fTime, float fElapsedTime)
{
	XMMATRIX dxWorld = pCamera->GetWorldMatrix();
	XMFLOAT3X3 dxmWorld;
	XMStoreFloat3x3(&dxmWorld, dxWorld);
	
	WR::Mat3 wrmWorld;
	WR::convert3x3(wrmWorld, dxmWorld);

	pHair->onFrame(wrmWorld.transpose(), fTime, fElapsedTime);
    pHairRenderer->onFrame(fTime, fElapsedTime);
    pMeshRenderer->onFrame(fTime, fElapsedTime);
}



void wrSceneManager::render(double fTime, float fElapsedTime)
{
    setPerFrameConstantBuffer(fTime, fElapsedTime);
    pHairRenderer->render(fTime, fElapsedTime);
    pMeshRenderer->render(fTime, fElapsedTime);
}


void wrSceneManager::release()
{
    SAFE_RELEASE(pcbVSPerFrame);

    if (pHairRenderer) pHairRenderer->release();
    if (pMeshRenderer) pMeshRenderer->release();

    SAFE_DELETE(pMeshRenderer);
    SAFE_DELETE(pHairRenderer);
    SAFE_DELETE(pHair);
}


void wrSceneManager::setPerFrameConstantBuffer(double fTime, float fElapsedTime)
{
    // Get the projection & view matrix from the camera class
    XMMATRIX mWorld = pCamera->GetWorldMatrix();
    XMMATRIX mView = pCamera->GetViewMatrix();
    XMMATRIX mProj = pCamera->GetProjMatrix();

    XMMATRIX mViewProjection = mView * mProj;

    // Set the constant buffers
    HRESULT hr;
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    V(pd3dImmediateContext->Map(pcbVSPerFrame, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
    auto pVSPerFrame = reinterpret_cast<CB_VS_PER_FRAME*>(MappedResource.pData);

    XMStoreFloat4x4(&pVSPerFrame->mViewProjection, XMMatrixTranspose(mViewProjection));
    XMStoreFloat4x4(&pVSPerFrame->mWorld, XMMatrixTranspose(mWorld));

    pVSPerFrame->time = (float)fTime;

    pVSPerFrame->lightDir = XMFLOAT3(0, 0.707f, -0.707f);
    pVSPerFrame->lightDiffuse = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
    pVSPerFrame->lightAmbient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.f);

    pd3dImmediateContext->Unmap(pcbVSPerFrame, 0);
    pd3dImmediateContext->VSSetConstantBuffers(0, 1, &pcbVSPerFrame);
}


bool wrSceneManager::initConstantBuffer()
{
    HRESULT hr;
    D3D11_BUFFER_DESC cbDesc;

    ZeroMemory(&cbDesc, sizeof(cbDesc));
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    cbDesc.ByteWidth = sizeof(CB_VS_PER_FRAME);
    V_RETURN(pd3dDevice->CreateBuffer(&cbDesc, nullptr, &pcbVSPerFrame));
    DXUT_SetDebugName(pcbVSPerFrame, "CB_VS_PER_FRAME");

    return true;
}
