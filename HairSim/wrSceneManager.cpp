#include "precompiled.h"
#include "wrSceneManager.h"
#include "wrHairRenderer.h"
#include "wrMeshRenderer.h"
#include <GeometricPrimitive.h>
#include <fstream>
#include <DXUTcamera.h>
#include "wrTypes.h"
#include "Parameter.h"
#include "LevelSet.h"
#include "wrColorGenerator.h"
#include "wrGeo.h"
#include "wrHair.h"
#include "CacheHair.h"
#include "HairDebugRenderer.h"
#include "BasicRenderer.h"
#include "TrainingGraph.h"


#include "SphereCollisionObject.h"

using namespace DirectX;


//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------
#pragma pack(push,1)

struct CB_VS_PER_FRAME
{
    XMFLOAT4X4  mViewProjection;
    XMFLOAT4X4  mWorld;
    XMFLOAT3    mViewPoint;
    float               time;
    XMFLOAT2    renderTargetSize;
    XMFLOAT2   nouse;
    float               nouse2;
    XMFLOAT3    lightDir;
    XMFLOAT4    lightDiffuse;
    XMFLOAT4    lightAmbient;
};
#pragma pack(pop)

#ifdef _DEBUG
#define ADF_FILE L"../../models/head_d"
#else
#define ADF_FILE L"../../models/head"
#endif

extern std::string CACHE_FILE;
extern std::string REF_FILE;

XRwy::EdgeVisualization visual;

wrSceneManager::wrSceneManager()
{
    m_bPause = false;
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

    /* load the Zhou's hair file */
    //auto hair = WR::loadFile(L"../../models/curly.hair");
    //hair->scale(0.01f);
    //hair->mirror(false, true, false);
    //WR::HairStrand::set_hair(hair);
    //WR::HairParticle::set_hair(hair);
    //hair->init_simulation();

    /* load the nCahce converted file */
    auto hair = new WR::CacheHair20;
    hair->loadFile(CACHE_FILE.c_str(), true);
    pHair = hair;

    auto hair0 = new WR::CacheHair20;
    hair0->loadFile(REF_FILE.c_str(), true);
    pHair0 = hair0;

    /* make the sphere as the collision object */
    //WR::Polyhedron_3 *P = WRG::readFile<WR::Polyhedron_3>("../../models/head.off");
    //WR::SphereCollisionObject* sphere = new WR::SphereCollisionObject;
    //sphere->setupFromPolyhedron(*P);
    //pCollisionHead = sphere;
    //delete P;

    if (APPLY_COLLISION)
        pCollisionHead = WR::loadCollisionObject(ADF_FILE);

    HRESULT hr;
    auto hairRenderer = new HairBiDebugRenderer(hair, hair0);
    pHairRenderer = hairRenderer;
    V_RETURN(hairRenderer->init());

    pMeshRenderer = new wrMeshRenderer;
    pMeshRenderer->setCamera(pCamera);
    V_RETURN(pMeshRenderer->init());

    pLineRenderer = new XRwy::LineRenderer;
    V_RETURN(pLineRenderer->init());

    visual.loadGraph(L"c0524.bg");
    visual.setRange(800, 25);

    return true;
}


void wrSceneManager::onFrame(double fTime, float fElapsedTime)
{
    XMMATRIX dxWorld = pCamera->GetWorldMatrix();
    XMFLOAT3X3 dxmWorld;
    XMStoreFloat3x3(&dxmWorld, dxWorld);
    
    WR::Mat3 wrmWorld;
    WR::convert3x3(wrmWorld, dxmWorld);

    WR::UserData userData;
    userData.pCollisionHead = pCollisionHead;

    if (!get_bPause())
    {
        pHair->onFrame(wrmWorld.transpose(), fTime, fElapsedTime, &userData);
        pHair0->onFrame(wrmWorld.transpose(), fTime, fElapsedTime, &userData);
    }
    pHairRenderer->onFrame(fTime, fElapsedTime);
    pMeshRenderer->onFrame(fTime, fElapsedTime);

    //pMeshRenderer->setTransformation();
    //pMeshRenderer->onFrame(fTime, fElapsedTime);
}



void wrSceneManager::render(double fTime, float fElapsedTime)
{
    setPerFrameConstantBuffer(fTime, fElapsedTime);
    pHairRenderer->render(fTime, fElapsedTime);

    auto ctx = DXUTGetD3D11DeviceContext();
    UINT stride = sizeof(HairDebugVertexInput);
    UINT offset = 0;
    ctx->IASetVertexBuffers(0, 1, &((HairBiDebugRenderer*)pHairRenderer)->pVB, &stride, &offset);
    pLineRenderer->setRenderState();
    visual.render();

    vec3 offset0{ -2.f, 0, 0 };
    pMeshRenderer->setTransformation(pHair0->get_rigidMotionMatrix());
    pMeshRenderer->setOffset(offset0);
    //pMeshRenderer->render(fTime, fElapsedTime);

    vec3 offset1{ 2.f, 0, 0 };
    pMeshRenderer->setTransformation(pHair0->get_rigidMotionMatrix());
    pMeshRenderer->setOffset(offset1);
    //pMeshRenderer->render(fTime, fElapsedTime);
}


void wrSceneManager::release()
{
    SAFE_RELEASE(pcbVSPerFrame);

    if (pHairRenderer) pHairRenderer->release();
    if (pMeshRenderer) pMeshRenderer->release();
    if (pLineRenderer) pMeshRenderer->release();

    SAFE_DELETE(pMeshRenderer);
    SAFE_DELETE(pCollisionHead);
    SAFE_DELETE(pHairRenderer);
    SAFE_DELETE(pLineRenderer);
    SAFE_DELETE(pHair);
    SAFE_DELETE(pHair0);
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
    XMStoreFloat3(&pVSPerFrame->mViewPoint, pCamera->GetEyePt());

    pVSPerFrame->time = (float)fTime;
    pVSPerFrame->renderTargetSize = XMFLOAT2(nWidth, nHeight);
    pVSPerFrame->lightDir = XMFLOAT3(0, 0.707f, -0.707f);
    pVSPerFrame->lightDiffuse = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
    pVSPerFrame->lightAmbient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.f);

    pd3dImmediateContext->Unmap(pcbVSPerFrame, 0);
    pd3dImmediateContext->VSSetConstantBuffers(0, 1, &pcbVSPerFrame);
    pd3dImmediateContext->PSSetConstantBuffers(0, 1, &pcbVSPerFrame);
    pd3dImmediateContext->GSSetConstantBuffers(0, 1, &pcbVSPerFrame);
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

void wrSceneManager::onKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext)
{
    static int a = 500;
    static int b = 5;
    if (!bKeyDown) return;
    switch (nChar)
    {
    case 'A':
    case 'a':
        a += 10;
        visual.setRange(a, b);
        break;
    case 'B':
    case 'b':
        a -= 10;
        visual.setRange(a, b);
        break;
    case 'q':
    case 'Q':
        ((HairBiDebugRenderer*)pHairRenderer)->pointFlag = !((HairBiDebugRenderer*)pHairRenderer)->pointFlag;
        break;
    }
}

void wrSceneManager::nextColorScheme()
{
    auto ptr = reinterpret_cast<HairBiDebugRenderer*>(pHairRenderer);
    ptr->nextColorScheme();
}

void wrSceneManager::prevColorScheme()
{
    auto ptr = reinterpret_cast<HairBiDebugRenderer*>(pHairRenderer);
    ptr->prevColorScheme();
}

void wrSceneManager::updateGDPara()
{
    std::ifstream file("../id.txt");
    if (!file.is_open()) throw std::exception("File not found!");
    int id;
    file >> id;
    file.close();
    auto ptr = reinterpret_cast<HairBiDebugRenderer*>(pHairRenderer);
    ptr->activateMonoGroup(id);
}

void  wrSceneManager::toggleGDMode()
{
    auto ptr = reinterpret_cast<HairBiDebugRenderer*>(pHairRenderer);
    ptr->toggleGDMode();
}

void  wrSceneManager::stepId()
{
    auto ptr = reinterpret_cast<HairBiDebugRenderer*>(pHairRenderer);
    std::ifstream file("../id.txt");
    if (!file.is_open()) throw std::exception("File not found!");
    int id, frame;
    file >> id;

    file >> frame;
    file.close();

    ptr->activateMonoGroup(id++);

    std::ofstream ofile("../id.txt");
    if (!ofile.is_open()) throw std::exception("File not found!");
    ofile << id << std::endl;
    ofile << frame << std::endl;
    ofile.close();
}


void wrSceneManager::redirectTo()
{
    auto ptr = reinterpret_cast<WR::CacheHair*>(pHair);
    auto ptr0 = reinterpret_cast<WR::CacheHair*>(pHair0);

    std::ifstream file("../id.txt");
    if (!file.is_open()) throw std::exception("File not found!");
    int id, frame;
    file >> id;
    file >> frame;
    file.close();

    ptr->jumpTo(frame);
    ptr0->jumpTo(frame);
}

