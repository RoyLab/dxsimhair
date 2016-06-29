#pragma once

namespace XRwy
{
    class IUnknown
    {
    public:
        virtual ~IUnknown(){}

        virtual bool init() = 0;
        virtual void release() = 0;
    };
}