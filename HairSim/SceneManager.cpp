#include <DXUT.h>
#include <DXUTcamera.h>
#include <SDKmisc.h>
#include <VertexTypes.h>
//#include <vld.h>
#include <ctime>
#include <random>

#include "XRwy_h.h"
#include "SceneManager.h"
#include "BasicRenderer.h"
#include "HairManager.h"
#include "SplitLayout.h"

// 所有的全局变量
namespace XRwy
{
    ParamDict g_paramDict;
}

namespace XRwy
{
    using namespace DirectX;

    SceneManager::SceneManager()
    {
        // initialize config parameter
        ConfigReader reader("../config2.ini");
        reader.getParamDict(g_paramDict);
        reader.close();

        // initialize random seed
        int rs = std::stoi(g_paramDict["randseed"]);
        if (rs < 0)
            srand(time(0));
        else srand(rs);

		// initialize layout
		int numframe = std::stoi(g_paramDict["numframe"]);
		splitLayout = new SplitLayout(numframe);

    }

    LRESULT SceneManager::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext)
    {
        pCamera->HandleMessages(hWnd, uMsg, wParam, lParam);

		if (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONDBLCLK)
		{
			int iMouseX = (short)LOWORD(lParam);
			int iMouseY = (short)HIWORD(lParam);
			int id = splitLayout->PosInFrameID(iMouseX, iMouseY);
			if (id < 0) id = 0;
			pHairManager->SetActive(id);
		}

        return 0;
    }

    void SceneManager::OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext)
    {
		if (!bKeyDown) return;

		if (nChar <= '9' && nChar >= '1')
		{
			pHairManager->ChangeColorScheme(nChar - '0');
		}

		int p = 1;
		switch (nChar)
		{
		case 'q':
		case 'Q':
			pHairManager->ChangeDrawBase(true);
			break;
		case 'w':
		case 'W':
			pHairManager->ChangeDrawBase(false, 1);
			break;
		case 'e':
		case 'E':
			pHairManager->ChangeDrawBase(false, -1);
			break;
		case 'f':
		case 'F':
			pHairManager->toggleDisp('f');// follicle
			break;
		case 'h':
		case 'H':
			pHairManager->toggleDisp('h');// hair
			break;
		case 'm':
		case 'M':
			pHairManager->toggleDisp('m');// mesh
			break;
		case 's':
		case 'S':
			pHairManager->OnFrameMove(0, 0, &p);
			break;
		default:
			break;
		}
    }

    void SceneManager::OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
    {
        // Update the camera's position based on user input 
        pCamera->FrameMove(fElapsedTime);
        if (pHairManager)
            pHairManager->OnFrameMove(fTime, fElapsedTime, pUserContext);
    }


    void SceneManager::OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
        float fElapsedTime, void* pUserContext)
    {
		splitLayout->BeginLayout();
		for (int i = 0; i < splitLayout->GetFrameNumber(); i++)
		{
			splitLayout->SetupFrame(i, pCamera);
			if (pHairManager)
				pHairManager->RenderInstance(pCamera, i, fTime, fElapsedTime);
		}
		splitLayout->EndLayout();
	}


    bool SceneManager::ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext)
    {
        pDeviceSettings->d3d11.sd.SampleDesc.Count = std::stoi(g_paramDict["multisample"]);
        pDeviceSettings->d3d11.sd.SampleDesc.Quality = 0;

        return true;
    }

    bool SceneManager::IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output,
        const CD3D11EnumDeviceInfo *DeviceInfo, DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext)
    {
        return true;
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

    HRESULT SceneManager::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
        void* pUserContext)
    {
		D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT;
        HRESULT hr;
        auto pd3dImmediateContext = DXUTGetD3D11DeviceContext();

		V_RETURN(CreateD3DRelatedResource());

        // Setup the camera's view parameters
        static const XMVECTORF32 s_vecEye = { 3.0f, 3.0f, -6.0f, 0.f };
        pCamera->SetViewParams(s_vecEye, g_XMZero);

        // initialize renderers
        upEffect.reset(new BasicEffect(pd3dDevice));
        V_RETURN(pMeshRenderer->Initialize());
        
        if (pHairManager)
            V_RETURN(pHairManager->Initialize());

        // load contents
        V_RETURN(pFbxLoader->LoadFBX(g_paramDict["headfbx"].c_str(), pd3dDevice, pd3dImmediateContext));
        V_RETURN(CreateFbxInputLayout(pd3dDevice));

        return S_OK;
    }

    void SceneManager::OnD3D11DestroyDevice(void* pUserContext)
    {
		ReleaseD3DRelatedResource();
    }


    // interface implementation
    bool SceneManager::Initialize()
    {
        pCamera = new CModelViewerCamera;

        return true;
    }

    void SceneManager::Release()
    {
		ReleaseD3DRelatedResource();

        SAFE_DELETE(pCamera);
		SAFE_DELETE(splitLayout);

        delete this;
    }

	bool SceneManager::CreateD3DRelatedResource()
	{
		pFbxLoader = new FBX_LOADER::CFBXRenderDX11;
		pMeshRenderer = new MeshRenderer;
		pHairManager = new HairManager(pFbxLoader, pMeshRenderer);

		return true;
	}

	void SceneManager::ReleaseD3DRelatedResource()
	{
		SAFE_DELETE(pFbxLoader);
		SAFE_RELEASE(pMeshRenderer);
		SAFE_RELEASE(pHairManager);
		upEffect.reset(nullptr);
	}

    void SceneManager::RenderText(CDXUTTextHelper* helper)
    {
        //auto pHair = dynamic_cast<WR::CacheHair20*>(g_SceneMngr.pHair);
        //g_pTxtHelper->DrawFormattedTextLine(L"Frame: %d / %d", pHair->getCurrentFrame(), pHair->getFrameNumber());
    }

    HRESULT SceneManager::CreateFbxInputLayout(ID3D11Device* pd3dDevice)
    {
        HRESULT hr;
        const void* pVSBufferPtr = nullptr;
        size_t nVSBufferSz = 0;
        const size_t nInputElem = VertexPositionNormalTexture::InputElementCount;
        D3D11_INPUT_ELEMENT_DESC iDesc[nInputElem];
        CopyMemory(&iDesc, &VertexPositionNormalTexture::InputElements, sizeof(D3D11_INPUT_ELEMENT_DESC)*nInputElem);

        pMeshRenderer->GetVertexShaderBytecode(&pVSBufferPtr, &nVSBufferSz, upEffect.get());
        V_RETURN(pFbxLoader->CreateInputLayout(pd3dDevice, pVSBufferPtr, nVSBufferSz, iDesc, nInputElem));

        return S_OK;
    }

}