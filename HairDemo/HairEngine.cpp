#include "precompiled.h"

#define XRWY_EXPORTS
#include "HairEngine.h"

#include <string>

#include <xlogger.h>
#include <cmdline.h>
#include <xstruct.hpp>
#include <XConfigReader.hpp>

#define XTIMER_INSTANCE
#include <xtimer.hpp>

#include "xhair.h"
#include "ISkinningEngine.h"
#include "ICollisionEngine.h"
#include "IGroupPbd.h"

using std::string;

namespace xhair
{
    class HairEngine
    {
    public:
        ParamDict params;
    public:
        HairEngine() {}
        ~HairEngine();

        int initialize();

        int update(const Matrix4& mat,
            float *particle_position,
            float *particle_direction);

        const HairGeometry* getHair() const { return hair_geometry_; }

    private:
        HairGeometry* loadHairGeometry(const string& file);

        string getStringParameter(int key) const;
        long long getIntParameter(int key) const;
        double getFloatParameter(int key) const;
        bool getBoolParameter(int key) const;

        string static_hair_name_;
        string head_collision_name_;

        HairGeometry *guide_geometry_ = nullptr;
        HairGeometry *hair_geometry_ = nullptr;

        IFilter* guide_simulator_ = nullptr;
        ISkinningEngine *skinning_engine_ = nullptr;
        ICollisionEngine *collision_engine_ = nullptr;
        IGroupPbd* group_pbd_ = nullptr;
    };

    HairEngine *_engine_instance;
}

extern "C"
{
    using namespace xhair;

    XRWY_DLL int InitializeHairEngine(
        const HairParameter* param,
        const CollisionParameter* col,
        const SkinningParameter* skin,
        const PbdParameter* pbd)
    {
        if (_engine_instance)
            ReleaseHairEngine();

        _engine_instance = new HairEngine;
        if (!_engine_instance)
            return -1;

        // init

        // record all the parameters
        // record hair parameters
        _engine_instance->params[P_bGuide].boolval = param->b_guide;
        _engine_instance->params[P_bCollision].boolval = param->b_collision;
        _engine_instance->params[P_bPbd].boolval = param->b_pbd;
        _engine_instance->params[P_root].stringval = param->root;

        // record collision parameters
        _engine_instance->params[P_correctionTolerance].floatval = col->correction_tolerance;
        _engine_instance->params[P_correctionRate].floatval = col->correction_rate;
        _engine_instance->params[P_maxStep].floatval = col->maxstep;

        // record skinning parameters

        // record pbd parameters
        _engine_instance->params[P_lambda].floatval = pbd->lambda;
        _engine_instance->params[P_chunkSize].floatval = pbd->chunksize;
        _engine_instance->params[P_maxIteration].intval = pbd->maxiteration;

        // file register
        XR::ConfigReader reader(std::string(param->root)+"hair.ini");
        _engine_instance->params[P_groupFile].stringval = pbd->groupfile;
        _engine_instance->params[P_weightFile].stringval = skin->weightfile;
        _engine_instance->params[P_collisionFile].stringval = col->collisionfile;

        // initialize 
        int ret = _engine_instance->initialize();

        if (ret != 0) return ret;

        return 0;
    }

    XRWY_DLL int UpdateParameter(int key, const char* value, char type)
    {
        auto res = _engine_instance->params.find(key);
        if (res == _engine_instance->params.end())
        {
            std::cerr << "Error, pending parameter not found: " << key << std::endl;
            return -1;
        }
        switch (type)
        {
        case 'I':
        case 'i':
            res->second.intval = std::atoi(value);
            break;
        case 'F':
        case 'f':
            res->second.floatval = std::atof(value);
            break;
        case 'S':
        case 's':
            res->second.stringval = value;
            break;
        case 'B':
        case 'b':
            res->second.boolval = std::atoi(value) != 0;
            break;
        default:
            std::cerr << "Error, pending parameter type not found: " << type << std::endl;
            return -1;
        }

        return 0;
    }

    XRWY_DLL int UpdateHairEngine(
        const float head_matrix[16],
        float *particle_positions,
        float *particle_directions)
    {
        Matrix4 rigidmotion(head_matrix);
        return _engine_instance->update(rigidmotion,
            particle_positions, particle_directions);
    }

    XRWY_DLL void ReleaseHairEngine()
    {
        SAFE_DELETE(_engine_instance);
    }

}
namespace xhair
{
    HairEngine::~HairEngine()
    {
        SAFE_DELETE(guide_geometry_);
        SAFE_DELETE(hair_geometry_);

        SAFE_DELETE(guide_simulator_);
        SAFE_DELETE(collision_engine_);
        SAFE_DELETE(group_pbd_);
        SAFE_DELETE(skinning_engine_);
    }

    int HairEngine::initialize()
    {
        // create
        HairGeometry* hair = loadHairGeometry(getStringParameter(P_root));
        if (!hair) return -1;

        group_pbd_ = CreateGroupPdb(params);
        if (!group_pbd_) return -1;

        collision_engine_ = CreateCollisionEngine(params);
        if (!collision_engine_) return -1;

        skinning_engine_ = CreateSkinningEngine(params);
        if (!skinning_engine_) return -1;
    }

    int HairEngine::update(const Matrix4& mat,
        float *particle_position,
        float *particle_direction)
    {
        if (guide_geometry_ && skinning_engine_)
        {
            guide_simulator_->filter(guide_geometry_);
            skinning_engine_->transport(guide_geometry_, hair_geometry_);
        }

        if (collision_engine_ && getBoolParameter(P_bCollision))
        {
            collision_engine_->filter(hair_geometry_);
        }

        if (group_pbd_ && getBoolParameter(P_bPbd))
        {
            group_pbd_->filter(hair_geometry_);
        }

        return 0;
    }

    HairGeometry * HairEngine::loadHairGeometry(const string & file)
    {
        return nullptr;
    }

    string HairEngine::getStringParameter(int key) const
    {
        return params.find(key)->second.stringval;
    }

    long long HairEngine::getIntParameter(int key) const
    {
        return params.find(key)->second.intval;
    }

    double HairEngine::getFloatParameter(int key) const
    {
        return params.find(key)->second.floatval;
    }

    bool HairEngine::getBoolParameter(int key) const
    {
        return params.find(key)->second.boolval;
    }
}