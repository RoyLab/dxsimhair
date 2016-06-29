#include <DXUT.h>
#include <DXUTcamera.h>
#include <SDKmisc.h>

#include "SceneManager.h"


namespace XRwy
{
    using namespace DirectX;

    LRESULT SceneManager::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext)
    {

        pCamera->HandleMessages(hWnd, uMsg, wParam, lParam);
        return 0;
    }

    void SceneManager::OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext)
    {
        
    }

    void SceneManager::OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
    {
        // Update the camera's position based on user input 
        pCamera->FrameMove(fElapsedTime);
    }

    bool SceneManager::ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext)
    {
        pDeviceSettings->d3d11.sd.SampleDesc.Count = 4;
        pDeviceSettings->d3d11.sd.SampleDesc.Quality = 4;

        return true;
    }

    bool SceneManager::IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output,
        const CD3D11EnumDeviceInfo *DeviceInfo, DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext)
    {
        return true;
    }

    HRESULT SceneManager::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
        void* pUserContext)
    {
        // Setup the camera's view parameters
        static const XMVECTORF32 s_vecEye = { 1.0f, 1.0f, -2.0f, 0.f };
        pCamera->SetViewParams(s_vecEye, g_XMZero);

        return S_OK;
    }

    HRESULT SceneManager::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
    {
        // Setup the camera's projection parameters
        float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
        pCamera->SetProjParams(XM_PI / 4, fAspectRatio, 0.1f, 1000.0f);
        pCamera->SetWindow(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);
        pCamera->SetButtonMasks(MOUSE_LEFT_BUTTON, MOUSE_WHEEL, MOUSE_MIDDLE_BUTTON);

        //g_SceneMngr.resize(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);
        return S_OK;
    }

    void SceneManager::OnD3D11ReleasingSwapChain(void* pUserContext)
    {

    }

    void SceneManager::OnD3D11DestroyDevice(void* pUserContext)
    {

    }

    void SceneManager::OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
        float fElapsedTime, void* pUserContext)
    {

    }

    // interface implementation
    bool SceneManager::Initialize()
    {
        pCamera = new CModelViewerCamera;
        return true;
    }

    void SceneManager::Release()
    {
        SAFE_DELETE(pCamera);
        SAFE_DELETE(pHair);

        delete this;
    }

    void SceneManager::RenderText(CDXUTTextHelper* helper)
    {
        //auto pHair = dynamic_cast<WR::CacheHair20*>(g_SceneMngr.pHair);
        //g_pTxtHelper->DrawFormattedTextLine(L"Frame: %d / %d", pHair->getCurrentFrame(), pHair->getFrameNumber());
    }

}