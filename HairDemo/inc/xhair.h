#pragma once

#include <memory>

#include <cy\cyPoint.h>
#include <cy\cyMatrix.h>

#include <macros.h>

namespace xhair
{
    /// region: forward declaration

    class HairEngine;

    /// region: auxilliary structs and classes

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

    /// region: interface and basic hair structs

    struct HairGeometry
    {
        Matrix4                 rigidTrans;
        UniqueArray<Point3>     position;
        UniqueArray<Point3>     direction;

        size_t                  nParticle = 0;
        size_t                  nStrand = 0;
        size_t                  particlePerStrand = 0;
    };

    struct BlendHairGeometry :
        public HairGeometry
    {
        std::vector<Point3> trans;
    };

    class IFilter
    {
    public:
        virtual void filter(HairGeometry* hair) = 0;
        virtual ~IFilter() {}
    };

    // all interfaces are sync method
    // do not require specific hair reps since hair anim reps may change
    class IHairLoader: public IFilter
    {
        COMMON_PROPERTY(int, nFrame);
        COMMON_PROPERTY(int, curFrame);
    public:
        virtual int nextframe(HairGeometry* hair) = 0;
        virtual void jumpTo(int frameNo) = 0;
        virtual void rewind() { jumpTo(0); }

        virtual ~IHairLoader() {}
    };

    class IHairTransport
    {
    public:
        virtual void transport(HairGeometry* hair0, HairGeometry* hair1) = 0;
    };

    /// region: globals

    extern HairEngine* _engine_instance;
}