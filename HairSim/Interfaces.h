#pragma once

namespace XR
{
    class IObject
    {
    };

    class IRenderer
    {
    public:
        virtual ~IRenderer(){}

        virtual void initialize(){}
        virtual void resize(size_t w, size_t h){}
        virtual void release(){}
        virtual void onFrame(double, float){}

        virtual void render(double, float){}
    };
}