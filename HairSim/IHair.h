#pragma once
#include "wrTypes.h"

namespace WR
{
    class IHair
    {
    public:
        virtual ~IHair(){}
        virtual size_t n_strands() const = 0;
        virtual const float* get_visible_particle_position(size_t i, size_t j) const = 0;
        virtual void onFrame(Mat3 world, float fTime, float fTimeElapsed, void* = nullptr) = 0;
    };
}