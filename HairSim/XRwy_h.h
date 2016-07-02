#pragma once

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
}