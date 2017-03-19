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

#include "Anim2Loader.h"

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

        const HairGeometry* getHair() const { return hair_; }

    private:
        HairGeometry* loadHairGeometry(const string& file);

        string getStringParameter(int key) const;
        long long getIntParameter(int key) const;
        double getFloatParameter(int key) const;
        bool getBoolParameter(int key) const;

        string static_hair_name_;
        string head_collision_name_;

        HairGeometry *guide_geometry_ = nullptr, *guide_geometry0_ = nullptr;
        HairGeometry *hair_ = nullptr, *hair0_ = nullptr;

        IFilter* guide_simulator_ = nullptr;
        ISkinningEngine *skinning_engine_ = nullptr;
        ICollisionEngine *collision_engine_ = nullptr;
        IGroupPbd* group_pbd_ = nullptr;

        IHairLoader* mainloader_ = nullptr, *guideloader_ = nullptr;
    };

    HairEngine *_engine_instance;
}

extern "C"
{
    using namespace xhair;

    void initializeParameters(
        HairEngine* _engine_instance,
        const HairParameter* param,
        const CollisionParameter* col,
        const SkinningParameter* skin,
        const PbdParameter* pbd)
    {
        // hair parameters
        _engine_instance->params[P_bGuide].boolval = param->b_guide;
        _engine_instance->params[P_bCollision].boolval = param->b_collision;
        _engine_instance->params[P_bPbd].boolval = param->b_pbd;
        _engine_instance->params[P_root].stringval = param->root;

        // file register
        XR::ConfigReader reader(std::string(param->root) + "hair.ini"); // default main.hair
        XR::ParameterDictionary files;
        reader.getParamDict(files);
        reader.close();

        _engine_instance->params[P_hairFile].stringval = files["main"];
        _engine_instance->params[P_hairAnim].stringval = files["animation"];
        _engine_instance->params[P_guideAnim].stringval = files["ganimation"];
        _engine_instance->params[P_groupFile].stringval = files["pbdgourp"];
        _engine_instance->params[P_weightFile].stringval = files["weight"];
        _engine_instance->params[P_collisionFile].stringval = files["collision"];


        if (col)
        {
            // collision parameters
            _engine_instance->params[P_correctionTolerance].floatval = col->correction_tolerance;
            _engine_instance->params[P_correctionRate].floatval = col->correction_rate;
            _engine_instance->params[P_maxStep].floatval = col->maxstep;
        }

        if (skin)
        {
            // skinning parameters
            _engine_instance->params[P_simulateGuide].boolval = skin->simulateGuide;
        }

        if (pbd)
        {
            // pbd parameters
            _engine_instance->params[P_lambda].floatval = pbd->lambda;
            _engine_instance->params[P_chunkSize].floatval = pbd->chunksize;
            _engine_instance->params[P_maxIteration].intval = pbd->maxiteration;
        }
    }

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

        initializeParameters(_engine_instance, param, col, skin, pbd);
        int ret = _engine_instance->initialize();

        return ret;
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
        SAFE_DELETE(hair_);
        SAFE_DELETE(guide_geometry_);

        releaseHair(hair0_);
        SAFE_DELETE(hair0_);

        releaseHair(guide_geometry0_);
        SAFE_DELETE(guide_geometry0_);

        SAFE_DELETE(guide_simulator_);
        SAFE_DELETE(collision_engine_);
        SAFE_DELETE(group_pbd_);
        SAFE_DELETE(skinning_engine_);
    }

    int HairEngine::initialize()
    {
        // create
        hair0_ = loadHairGeometry(getStringParameter(P_root));
        if (!hair0_) return -1;

        hair_ = new HairGeometry;
        hair_->nParticle = hair0_->nParticle;

        string anim = getStringParameter(P_hairAnim);
        if (anim.size())
        {
            mainloader_ = new Anim2Loader(anim.c_str(), hair_);
            if (hair_->nParticle != hair0_->nParticle)
                return -1;
        }

        if (getBoolParameter(P_bPbd))
        {
            group_pbd_ = CreateGroupPdb(params);
            if (!group_pbd_) return -1;
        }

        if (getBoolParameter(P_bCollision))
        {
            collision_engine_ = CreateCollisionEngine(params);
            if (!collision_engine_) return -1;
        }

        if (getBoolParameter(P_bGuide))
        {
            skinning_engine_ = CreateSkinningEngine(params);
            if (!skinning_engine_) return -1;
        }
    }

    int HairEngine::update(const Matrix4& mat,
        float *particle_position,
        float *particle_direction)
    {
        hair_->position = reinterpret_cast<Point3*>(particle_position);
        hair_->direction = reinterpret_cast<Point3*>(particle_direction);

        if (!params.at(P_bGuide).boolval)
        {
            if (false /*TODO*/ && guide_geometry_ && skinning_engine_)
            {
                guide_simulator_->filter(guide_geometry_);
                skinning_engine_->transport(guide_geometry_, hair_);
            }

            if (false /*TODO*/ && collision_engine_ && getBoolParameter(P_bCollision))
            {
                collision_engine_->filter(hair_);
            }

            if (false /*TODO*/ && group_pbd_ && getBoolParameter(P_bPbd))
            {
                group_pbd_->filter(hair_);
            }
        }
        else
        {
            // no skinning, load anim2 directly
            mainloader_->filter(hair_);
        }

        return 0;
    }

    HairGeometry * HairEngine::loadHairGeometry(const string & file)
    {
        HairGeometry* hair = new HairGeometry;
        auto tmpptr = new Anim2Loader(file.c_str(), hair);
        delete tmpptr;
        return hair;
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