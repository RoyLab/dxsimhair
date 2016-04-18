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


#include "SphereCollisionObject.h"

using namespace DirectX;
extern std::string CACHE_FILE;
extern std::string GUIDE_FILE;

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

#ifdef _DEBUG
#define ADF_FILE L"../../models/head_d"
#else
#define ADF_FILE L"../../models/head"
#endif

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
    hair0->loadFile("D:/s4000.anim2", true);
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

    /* read the guide hair info */
    std::ifstream file(GUIDE_FILE, std::ios::binary);
    if (!file.is_open()) throw std::exception("File not found!");

    int nGuide = 0;
    char buffer[128];
    file.read(buffer, 4);
    nGuide = *reinterpret_cast<int*>(buffer);

    file.read(buffer, 8);
    std::vector<int> guides;
    std::cout << nGuide << std::endl;
    for (size_t i = 0; i < nGuide; i++)
    {
        file.read(buffer, 4);
        guides.push_back(*reinterpret_cast<int*>(buffer));
    }

    file.close();


    /* generate guid hair outstanding scheme */
    XMFLOAT3* colors = new DirectX::XMFLOAT3[pHair->n_strands() * N_PARTICLES_PER_STRAND];
    for (int i = 0; i < pHair->n_strands(); i++)
    {
        vec3 color;
        wrColorGenerator::genRandSaturatedColor(color);
        for (int j = 0; j < N_PARTICLES_PER_STRAND; j++)
            memcpy(&colors[N_PARTICLES_PER_STRAND*i + j], color, sizeof(vec3));
    }

    //vec3 red{ 1.0f, 0.0f, 0.0f };
    //for (int id : guides)
    //{
    //    for (size_t i = 0; i < N_PARTICLES_PER_STRAND; i++)
    //        memcpy(&colors[N_PARTICLES_PER_STRAND*id+i], red, sizeof(vec3));
    //}

    ///* read the grouping info */
    //file.open("s4000.group", std::ios::binary);
    //if (!file.is_open()) throw std::exception("File not found!");

    //file.read(buffer, 4);
    //int nStrand = *reinterpret_cast<int*>(buffer);

    //std::vector<int> groups(nStrand);
    //std::cout << nStrand << std::endl;
    //for (size_t i = 0; i < nStrand; i++)
    //{
    //    file.read(buffer, 4);
    //    groups[i] = (*reinterpret_cast<int*>(buffer));
    //}

    //file.close();

    ///* generate grouping scheme */
    //XMFLOAT3* colors = new DirectX::XMFLOAT3[pHair->n_strands() * N_PARTICLES_PER_STRAND];
    //for (int i = 0; i < pHair->n_strands(); i++)
    //{
    //    vec3 color;
    //    wrColorGenerator::genRandSaturatedColor(color);
    //    for (int j = 0; j < N_PARTICLES_PER_STRAND; j++)
    //        memcpy(&colors[N_PARTICLES_PER_STRAND*i + j], color, sizeof(vec3));
    //}

    //XMFLOAT3* groupColors = new DirectX::XMFLOAT3[nGuide];
    //for (int i = 0; i < nGuide; i++)
    //{
    //    vec3 color;
    //    wrColorGenerator::genRandSaturatedColor(color);
    //    memcpy(&groupColors[i], color, sizeof(vec3));
    //}

    //for (int i = 0; i < pHair->n_strands(); i++)
    //{
    //    vec3 color{ 0.0f, 0.0f, 0.0f };
    //    for (int j = 0; j < N_PARTICLES_PER_STRAND; j++)
    //        memcpy(&colors[N_PARTICLES_PER_STRAND*i + j], i % 5 == 0 ? (void*)(groupColors + groups[i]) : (void*)(color), sizeof(vec3));
    //}

    //SAFE_DELETE_ARRAY(groupColors);

    V_RETURN(hairRenderer->init(colors));
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

    vec3 offset0{ -2.f, 0, 0 };
    pMeshRenderer->setTransformation(pHair0->get_rigidMotionMatrix());
    pMeshRenderer->setOffset(offset0);
    pMeshRenderer->render(fTime, fElapsedTime);

    vec3 offset1{ 2.f, 0, 0 };
    pMeshRenderer->setTransformation(pHair0->get_rigidMotionMatrix());
    pMeshRenderer->setOffset(offset1);
    pMeshRenderer->render(fTime, fElapsedTime);
}


void wrSceneManager::release()
{
    SAFE_RELEASE(pcbVSPerFrame);

    if (pHairRenderer) pHairRenderer->release();
    if (pMeshRenderer) pMeshRenderer->release();

    SAFE_DELETE(pMeshRenderer);
    SAFE_DELETE(pCollisionHead);
    SAFE_DELETE(pHairRenderer);
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

void wrSceneManager::onKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext)
{

}

