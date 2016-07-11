#pragma once
#include <DXUTgui.h>


class GUIManager:
    public CDXUTDialog
{
public:
    void InitializeComponents();
};

void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext);
