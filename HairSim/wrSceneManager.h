#pragma once
#include "wrMacro.h"

class CModelViewerCamera;
class wrHairRenderer;
class wrMeshRenderer;

namespace WR
{
    class IHair;
    class ICollisionObject;
}

class wrRendererInterface
{
public:
    virtual ~wrRendererInterface(){}
    virtual void release() = 0;
    virtual void onFrame(double, float) = 0;
    virtual void render(double, float) = 0;
};

class wrSceneManager
{
    COMMON_PROPERTY(bool, bPause);
public:
    wrSceneManager();
    ~wrSceneManager();

    void setCamera(CModelViewerCamera* camera) { pCamera = camera; }

    bool init();
    void release();
    void onFrame(double, float);
    void render(double, float);
    void onKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext);
    void nextColorScheme();
    void prevColorScheme();

private:
    void setPerFrameConstantBuffer(double, float);
    bool initConstantBuffer();

private:
    WR::IHair*                  pHair = nullptr;
    WR::IHair*                  pHair0 = nullptr;
    CModelViewerCamera*         pCamera = nullptr;
    wrRendererInterface*        pHairRenderer = nullptr;
    wrMeshRenderer*             pMeshRenderer = nullptr;

    ID3D11Device*               pd3dDevice = nullptr;
    ID3D11DeviceContext*        pd3dImmediateContext = nullptr;

    ID3D11Buffer*               pcbVSPerFrame = nullptr;
    WR::ICollisionObject*       pCollisionHead = nullptr;
};


