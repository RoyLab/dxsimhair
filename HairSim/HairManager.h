#pragma once
#include <d3d11.h>
#include <map>
#include <vector>
#include <string>
#include <DXUTcamera.h>

#include "XRwy_h.h"
#include "HairGeometry.h"
#include "CFBXRendererDX11.h"

namespace XRwy
{
    // the color generator is responsible for the storage management of color array
    class IHairColorGenerator
    {
    public:
        virtual ~IHairColorGenerator(){}
        virtual const XMFLOAT3* GetColorArray() const = 0;
    };

    class HairManager:
        public IUnknown
    {
        typedef void(*FPDraw)(ID3D11DeviceContext*, int, int, void*);

        struct SGeoManip
        {
            HairAnimationLoader*    loader;
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
        void RenderAll(CModelViewerCamera* camera, double fTime, float fElapsedTime);
        void OnFrameMove(double fTime, float fElapsedTime, void* pUserContext);

		void toggleAnimation();

    private:
        // do not release
        FBX_LOADER::CFBXRenderDX11*     pFbxLoader;
        MeshRenderer*                   pMeshRenderer;
        ID3D11Device*                   pd3dDevice;
        ID3D11DeviceContext*            pd3dImmediateContext;

        HairRenderer*               pHairRenderer = nullptr;
        VertexBufferDict            dataBuffers;
        HairGeometryList            hairManips;
        ID3D11InputLayout*          pInputLayout = nullptr;

		bool						bAnim = true;
    };
}