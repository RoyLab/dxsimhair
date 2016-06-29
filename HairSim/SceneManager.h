#pragma once
#include <windef.h>
#include <DXUT.h>
#include <Effects.h>

#include "XRwy_h.h"
#include "BasicRenderer.h"
#include "CFBXRendererDX11.h"
#include "CacheHair.h"

class CModelViewerCamera;
class CDXUTTextHelper;

namespace XRwy
{
    class SceneManager:
        public IUnknown
    {
        typedef std::unique_ptr<DirectX::BasicEffect> EffectPtr;

    public:
        WR::CacheHair20*                pHair = nullptr;
        CModelViewerCamera*             pCamera = nullptr;
        FBX_LOADER::CFBXRenderDX11*     pFbxLoader = nullptr;

        EffectPtr                       upEffect;
        MeshRenderer*                   pMeshRenderer = nullptr;

    public:
        SceneManager(){}
        ~SceneManager(){}

        // callback
        LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
            void* pUserContext);
        void OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext);
        void OnFrameMove(double fTime, float fElapsedTime, void* pUserContext);
        bool ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext);
        bool IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output,
            const CD3D11EnumDeviceInfo *DeviceInfo,
            DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext);
        HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
            void* pUserContext);
        HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
        void OnD3D11ReleasingSwapChain(void* pUserContext);
        void OnD3D11DestroyDevice(void* pUserContext);
        void OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
            float fElapsedTime, void* pUserContext);

        // interface implementation
        bool Initialize();
        void Release();

        // user define
        void RenderText(CDXUTTextHelper*);

    private:
        HRESULT CreateFbxInputLayout(ID3D11Device* pd3dDevice);
    };
}