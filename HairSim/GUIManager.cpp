#include <DXUT.h>
#include "GUIManager.h"
#include "SceneManager.h"

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_PAUSE               2
#define IDC_CHANGEDEVICE        3
#define IDC_NEXT_COLOR          4
#define IDC_PREV_COLOR          5
#define IDC_UPDATE_GD_PARA      6
#define IDC_TOGGLE_GD_MODE      7
#define IDC_STEP_GD_ID          8
#define IDC_GOTO_FRAME          9



void GUIManager::InitializeComponents()
{
    int iY = 30;
    int iYo = 26;
    AddButton(IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 0, iY, 170, 22);
    AddButton(IDC_CHANGEDEVICE, L"Change device (F2)", 0, iY += iYo, 170, 22, VK_F2);
    AddButton(IDC_PAUSE, L"Pause (F3)", 0, iY += iYo, 170, 22, VK_F3);
    AddButton(IDC_NEXT_COLOR, L"Next Color (F4)", 0, iY += iYo, 170, 22, VK_F4);
    AddButton(IDC_PREV_COLOR, L"Prev Color (F5)", 0, iY += iYo, 170, 22, VK_F5);
    AddButton(IDC_UPDATE_GD_PARA, L"update (F6)", 0, iY += iYo, 170, 22, VK_F6);
    AddButton(IDC_TOGGLE_GD_MODE, L"full/mono (F7)", 0, iY += iYo, 170, 22, VK_F7);
    AddButton(IDC_STEP_GD_ID, L"step id (F7)", 0, iY += iYo, 170, 22, VK_F8);
    AddButton(IDC_GOTO_FRAME, L"goto frame (F8)", 0, iY += iYo, 170, 22, VK_F9);
}

void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext)
{
 /*   switch (nControlID)
    {
    case IDC_TOGGLEFULLSCREEN:
        DXUTToggleFullScreen();
        break;
    case IDC_PAUSE:
        g_SceneMngr.set_bPause(!g_SceneMngr.get_bPause());
        break;
    case IDC_NEXT_COLOR:
        g_SceneMngr.nextColorScheme();
        break;
    case IDC_PREV_COLOR:
        g_SceneMngr.prevColorScheme();
        break;
    case IDC_CHANGEDEVICE:
        g_SettingsDlg.SetActive(!g_SettingsDlg.IsActive());
        break;
    case IDC_UPDATE_GD_PARA:
        g_SceneMngr.updateGDPara();
        break;
    case IDC_TOGGLE_GD_MODE:
        g_SceneMngr.toggleGDMode();
        break;
    case IDC_STEP_GD_ID:
        g_SceneMngr.stepId();
        break;
    case IDC_GOTO_FRAME:
        g_SceneMngr.redirectTo();
        break;
    }*/
}