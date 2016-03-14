#include <GFSDK_HairWorks.h>
#include <DirectXMath.h>
#include <cassert>

using namespace DirectX;

void test()
{
    GFSDK_HairSDK* g_hairSDK = GFSDK_LoadHairSDK("GFSDK_HairWorks.win32.dll",
        GFSDK_HAIRWORKS_VERSION);
    assert(g_hairSDK);

    GFSDK_HairAssetDescriptor hairAssetDescriptor;
    hairAssetDescriptor.m_NumGuideHairs = 20; // number of guide hairs
    hairAssetDescriptor.m_NumVertices = 20;  // number of total hair cvs
    hairAssetDescriptor.m_pVertices = vertices; // cv position data
    hairAssetDescriptor.m_pEndIndices = endIndices; // index to last cv for each guide curve
    //...
    g_hairSDK->CreateHairAsset(hairAssetDescriptor, &hairAssetID);
    g_hairSDK->InitRenderResources(d3dDevice);

    GFSDK_HairInstanceID hairInstanceID;
    g_hairSDK->CreateHairInstance(hairAssetID, &hairInstanceID);


    g_hairSDK->SetCurrentContext(d3dContext);

    XMMATRIX projection; // ... get projection matrix from your camera definition
    XMMATRIX view;           // ... get model view matrix from your camera definition
    g_hairSDK->SetViewProjection((const gfsdk_float4x4*)&view, (const gfsdk_float4x4*)&projection);

    GFSDK_HairInstanceDescriptor hairInstanceDescriptor;
    hairInstanceDescriptor.m_width = 0.2;
    hairInstanceDescriptor.m_density = 1.0;
    hairInstanceDescriptor.m_lengthNoise = 0.0f;
    hairInstanceDescriptor.m_simulate = true;
    //...
    g_hairSDK->UpdateInstanceDescriptor(hairInstanceID, hairInstanceDescriptor);
}