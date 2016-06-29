#pragma once

namespace XRwy
{
    class IUnknown
    {
    public:
        virtual bool Initialize() = 0;
        virtual void Release() = 0;
    };
}