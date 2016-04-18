#pragma once
#include "wrSceneManager.h"
#include <DirectXMath.h>
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
    void setTransformation(const float* trans4x4);
    void setOffset(const float* vec);

private:
    ID3D11Device*           pd3dDevice = nullptr;
    ID3D11DeviceContext*    pd3dImmediateContext = nullptr;

    EffectPtr               pEffect;
    CModelViewerCamera*     pCamera = nullptr;
    DirectX::XMMATRIX       translation;

    bool                    enableControl = false;
};

