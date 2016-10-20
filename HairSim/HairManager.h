#pragma once
#include <d3d11.h>
#include <map>
#include <vector>
#include <string>
#include <DXUTcamera.h>

#include "XRwy_h.h"
#include "HairStructs.h"
#include "CFBXRendererDX11.h"

namespace XRwy
{
	struct FrameContent
	{
		int animID;
		int colorID;
		int rendMode;
		int displayMask;
	};

    class HairManager:
        public IUnknown
    {
        typedef void(*FPDraw)(ID3D11DeviceContext*, int, int, void*);

        struct SGeoManip
        {
            HairLoader*				loader;
            HairGeometry*           hair;
            ID3D11Buffer*           pVB[2];
            bool                    sync;

            void Release();
            void UpdateBuffers(ID3D11DeviceContext* pd3dDevice);
        };

        typedef std::map<std::string, ID3D11Buffer*>    VertexBufferDict;
        typedef std::vector<SGeoManip>                  HairGeometryList;
		
    public:
        HairManager(FBX_LOADER::CFBXRenderDX11*, MeshRenderer*);

        bool Initialize();
        void Release();
        void RenderInstance(CModelViewerCamera* camera, int hairId, double fTime, float fElapsedTime);
        void OnFrameMove(double fTime, float fElapsedTime, void* pUserContext);

		void toggleAnimation();
		void toggleDiffDisp();
		void ChangeColorScheme(int i);
		void SetActive(int i) { activeContentId = i; }
		void ChangeDrawBase(bool open, int incre = 0);
		void toggleDisp(char item);
		void togglePBD();

    private:
		void SetupContents();

        // do not release
        FBX_LOADER::CFBXRenderDX11*     pFbxLoader;
        MeshRenderer*                   pMeshRenderer;
        ID3D11Device*                   pd3dDevice;
        ID3D11DeviceContext*            pd3dImmediateContext;

		HairRenderer*               pHairRenderer = nullptr;
		FollicleRenderer*           pFollicleRenderer = nullptr;
        VertexBufferDict            dataBuffers;
        HairGeometryList            hairManips;
        ID3D11InputLayout*          pInputLayout = nullptr;

		std::vector<FrameContent>   contents;
		int							activeContentId = 0;
		bool						bAnim = true;
		bool						bFullShow = true;
		int							nDisplayBase = 0;
	};
}