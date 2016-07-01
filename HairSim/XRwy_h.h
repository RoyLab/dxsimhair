#pragma once

namespace XRwy
{
    class IUnknown
    {
    public:
        virtual bool Initialize() = 0;
        virtual void Release() = 0;
    };


    // class declaration
    class IRenderer;
    class LineRenderer;
    class MeshRenderer;
    class HairRenderer;

    class RenderTextureClass;
}