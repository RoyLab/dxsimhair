#pragma once

#include <memory>

#include <cy\cyPoint.h>
#include <cy\cyMatrix.h>

namespace xhair
{
    template< typename T >
    struct array_deleter
    {
        void operator ()(T const * p)
        {
            delete[] p;
        }
    };

    template <class T>
    class UniqueArray : public std::unique_ptr<T, array_deleter<T>>
    {
    public:
        T& operator[](size_t i) { return get()[i]; }
        const T& operator[](size_t i) const { return get()[i]; }
    };

    typedef cyPoint3f Point3;
    typedef cyMatrix4f Matrix4;


    template <class T>
    class IntWrapper
    {
    public:
        IntWrapper() {}
        IntWrapper(const T& p)
        {
            i = p.id;
        }

        IntWrapper& operator=(const T& p)
        {
            i = p.id;
            return *this;
        }

        IntWrapper& operator=(T& p)
        {
            i = p.id;
            return *this;
        }

        int i;
    };

    struct IntPair
    {
        IntPair(int a, int b) :i(a), number(b) {}
        int i, number;
    };

    struct HairGeometry
    {
        Matrix4                 rigidTrans;
        UniqueArray<Point3>     position;
        UniqueArray<Point3>     direction;

        size_t                  nParticle = 0;
        size_t                  nStrand = 0;
        size_t                  particlePerStrand = 0;
    };

    // all interfaces are sync method
    // do not require specific hair reps since hair anim reps may change
    class HairLoader
    {
        COMMON_PROPERTY(int, nFrame);
        COMMON_PROPERTY(int, curFrame);
    public:
        virtual bool loadFile(const char* fileName, HairGeometry * geom) = 0;
        virtual void rewind() {}
        virtual void nextFrame() {}
        virtual void jumpTo(int frameNo) {}

        virtual ~HairLoader() {}
    };

    class IHairCorrection
    {
    public:
        virtual void solve(HairGeometry* hair) = 0;
        virtual bool initialize(HairGeometry* hair, float dr, float balance, const int* groupInfo, size_t ngi, int nGroup) = 0;
        virtual ~IHairCorrection() {}
    };
}