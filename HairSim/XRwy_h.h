#pragma once
#include <DXUT.h>
#include <DXUTSettingsDlg.h>
#include <SDKmisc.h>
#include "ConfigReader.h"

class GUIManager;

extern CDXUTDialogResourceManager	g_DialogResourceManager; // manager for shared resources of dialogs
extern CD3DSettingsDlg				g_SettingsDlg;          // Device settings dialog
extern CDXUTTextHelper*				g_pTxtHelper;

extern GUIManager					g_HUD;                  // dialog for standard controls

namespace XRwy
{
    class IUnknown
    {
    public:
        virtual ~IUnknown(){}

        virtual bool Initialize() = 0;
        virtual void Release() = 0;
    };


    // class declaration
    class IRenderer;
    class LineRenderer;
    class MeshRenderer;
    class HairRenderer;

    class RenderTextureClass;
    class HairAnimationLoader;

    class HairManager;
	class SceneManager;

	extern ParamDict g_paramDict;
}

extern XRwy::SceneManager*			g_SceneMngr;
