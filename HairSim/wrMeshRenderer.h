#pragma once
#include "wrSceneManager.h"
#include <Effects.h>

class wrMeshRenderer:
    public wrRendererInterface
{
public:
    typedef std::unique_ptr<DirectX::BasicEffect> EffectPtr;

    wrMeshRenderer();
    ~wrMeshRenderer();

    bool init();
    void release();
    void onFrame(double, float){}
    void render(double, float);

    void setMatrices();
    void setCamera(CModelViewerCamera* camera) { pCamera = camera; }

private:
    ID3D11Device*           pd3dDevice = nullptr;
    ID3D11DeviceContext*    pd3dImmediateContext = nullptr;

    EffectPtr               pEffect;
    CModelViewerCamera*     pCamera = nullptr;
};

