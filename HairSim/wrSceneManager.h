#pragma once

class CModelViewerCamera;
class wrHairRenderer;
class wrHair;
class wrHairSimulator;
class wrMeshRenderer;

class wrRendererInterface
{
    virtual bool init() = 0;
    virtual void release() = 0;
    virtual void onFrame(double, float) = 0;
    virtual void render(double, float) = 0;
};

class wrSceneManager
{
public:
    wrSceneManager();
    ~wrSceneManager();

    void setCamera(CModelViewerCamera* camera) { pCamera = camera; }

    bool init();
    void release();
    void onFrame(double, float);
    void render(double, float);

private:
    void setPerFrameConstantBuffer(double, float);
    bool initConstantBuffer();

private:
    wrHair*                     pHair = nullptr;
    CModelViewerCamera*         pCamera = nullptr;
    wrHairRenderer*             pHairRenderer = nullptr;
    wrHairSimulator*            pSimulator = nullptr;

    wrMeshRenderer*             pMeshRenderer = nullptr;

    ID3D11Device*               pd3dDevice = nullptr;
    ID3D11DeviceContext*        pd3dImmediateContext = nullptr;

    ID3D11Buffer*               pcbVSPerFrame = nullptr;
};


