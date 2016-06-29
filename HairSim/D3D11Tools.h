#pragma once
#include <d3d11.h>
#include <iostream>

void DumpLiveObjects(ID3D11Device* pd3dDevice, const char* name, bool detail = false)
{
    auto para = D3D11_RLDO_SUMMARY;
    if (detail)
        para |= D3D11_RLDO_DETAIL;

    char info[200];

    strcpy(info, name);
    strcat_s(info, ": begin........................................\n");
    OutputDebugStringA(info);

    ID3D11Debug * d3dDebug = nullptr;
    if (SUCCEEDED(pd3dDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&d3dDebug))))
    {
        d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);
        d3dDebug->Release();
    }

    strcpy(info, name);
    strcat_s(info, ": end........................................\n");
    OutputDebugStringA(info);
}
