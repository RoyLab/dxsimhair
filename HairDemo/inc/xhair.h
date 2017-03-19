#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <cy\cyPoint.h>
#include <cy\cyMatrix.h>

#include <macros.h>

namespace xhair
{
    /// region: forward declaration

    class HairEngine;

    /// region: auxilliary structs and classes

    struct Notype
    {
        union
        {
            long long intval;
            double floatval;
            bool boolval;
        };
        std::string stringval;
    };

    typedef std::unordered_map<int, Notype> ParamDict;

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

    enum ParameterEnum
    {
        // hair geometry
        P_nparticle,
        P_pps, // particle per strand
        P_nstrand,

        // global setup
        P_bGuide,
        P_bCollision,
        P_bPbd,
        P_root,

        // collision
        P_correctionTolerance,
        P_correctionRate,
        P_maxStep,

        // skinning
        P_simulateGuide,

        // group pbd
        P_detectRange,
        P_lambda,
        P_chunkSize,
        P_maxIteration,

        // file paths
        P_hairFile,

        P_hairAnim,
        P_guideAnim,

        P_collisionFile,
        P_groupFile,
        P_weightFile,

        ParameterCount
    };

    struct HairGeometry
    {
        Matrix4                 rigidTrans;
        Point3*                 position = nullptr;
        Point3*                 direction = nullptr;

        int                     nParticle = 0;
        int                     nStrand = 0;
        int                     particlePerStrand = 0;
    };

    inline void allocateHair(HairGeometry * hair)
    {
        if (hair)
        {
            hair->position = new Point3[hair->nParticle];
            hair->direction = new Point3[hair->nParticle];
        }
    }

    inline void releaseHair(HairGeometry* hair)
    {
        if (hair)
        {
            SAFE_DELETE_ARRAY(hair->position);
            SAFE_DELETE_ARRAY(hair->direction);
        }
    }

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
        enum FilterParam { Next, Jump };

        int nextframe(HairGeometry* hair)
        {
            setFilter(Next);
            filter(hair);
        };

        void jumpTo(HairGeometry* hair, int frameNo)
        {
            setFilter(Jump+frameNo);
            filter(hair);
        }

        virtual ~IHairLoader() {}

    protected:
        virtual void setFilter(int param) = 0;

    };

    class IHairTransport
    {
    public:
        virtual void transport(HairGeometry* hair0, HairGeometry* hair1) = 0;
    };

    /// region: globals
    extern HairEngine* _engine_instance;
}