#include "precompiled.h"
#include <xmath.h>
#include <macros.h>

#include "CSkinningEngine.h"

namespace xhair
{
    void CSkinningEngine::transport(HairGeometry * hair0, HairGeometry * hair1)
    {
        assert(hair0->nStrand == getNLeft());
        assert(hair1->nStrand == getNRight());

        memcpy(&hair0->rigidTrans, &hair1->rigidTrans, sizeof(Matrix4));
        Matrix4 rigid;
        hair1->rigidTrans.GetTranspose(rigid);

        calcTrans(hair0);

        for (int i = 0; i < getNRight(); i++)
        {
            // compute all the positions
            int nguide = weights_->getNumberOfGuides(i);
            for (int j = 0; j < pps_; j++)
            {
                int id = i * pps_ + j;

                Point3 acc(0.0f);
                for (int k = 0; k < nguide; k++)
                {
                    auto weight = weights_->getWeights(i, j, k);
                    acc += weight.second * trans_[weight.first];
                }
                Point3 after_rigid(hair1->rigidTrans.TransposeMult(rest_state2->position[id]));
                hair1->position[id] = after_rigid + acc;
            }

            //  compute the directions by difference between adjacent particles
            hair1->direction[i* pps_] = 
                hair1->direction[i* pps_ + 1] - hair1->direction[i* pps_];

            hair1->direction[(i + 1)* pps_ - 1] = 
                hair1->direction[(i + 1)* pps_ - 1] - hair1->direction[(i + 1)* pps_ - 2];

            for (int j = 1; j < pps_ - 1; j++)
            {
                int id = i * pps_ + j;
                hair1->direction[id] = hair1->position[id + 1] - hair1->position[id - 1];
            }

            // normalize all the directions
            for (int id = i * pps_, j = 0; j < pps_; j++, id++)
            {
                hair1->direction[id].Normalize();
            }
        }
    }

    void CSkinningEngine::calcTrans(const HairGeometry * hair)
    {
        assert(hair->nStrand == getNLeft());

        trans_.resize(getNLeft());
        for (int i = 0; i < getNLeft(); i++)
        {
            Point3 after_rigid(hair->rigidTrans.TransposeMult(rest_state1->position[i]));
            trans_[i] = after_rigid - rest_state1->position[i];
        }
    }


    ISkinningEngine* CreateSkinningEngine(const ParamDict& param)
    {
        CSkinningEngine* ret = new CSkinningEngine(param);
        return ret;
    }


    //ReducedModel::ReducedModel(int para)
    //{
    //    if (para & 1) bPDB = true;
    //    else bPDB = false;

    //    if (para & 2) bHairBody = true;
    //    else bHairBody = false;

    //    skinning = new SkinningAndHairBodyCollisionEngineCPU(bHairBody);
    //}

    //ReducedModel::~ReducedModel()
    //{
    //    SAFE_DELETE(skinning);
    //    SAFE_DELETE(pPDB);
    //}

    //bool ReducedModel::loadFile(const char * fileName, HairGeometry * geom)
    //{
    //    bool hr;
    //    hairGeom = geom;

    //    V_RETURN(skinning->loadFile(fileName, hairGeom));

    //    std::ifstream file(g_paramDict["pbdfile"], std::ios::binary);
    //    int ngi;
    //    Read4Bytes(file, ngi);

    //    int *gid = new int[ngi];
    //    ReadNBytes(file, gid, ngi * sizeof(4));
    //    pPDB = new GroupPBD2(hairGeom, std::stof(g_paramDict["dr"]),
    //        std::stof(g_paramDict["balance"]), gid, ngi, std::stoi(g_paramDict["npbdgroup"]));
    //    V_RETURN(pPDB->initialize(hairGeom, std::stof(g_paramDict["dr"]),
    //        std::stof(g_paramDict["balance"]), gid, ngi, std::stoi(g_paramDict["npbdgroup"])));
    //    delete[]gid;

    //    set_nFrame(skinning->get_nFrame());
    //    set_curFrame(-1);

    //    return true;
    //}

    //void ReducedModel::rewind()
    //{
    //    jumpTo(0);
    //}

    //void ReducedModel::nextFrame()
    //{
    //    skinning->nextFrame();
    //    if (bPDB)
    //        pPDB->solve(hairGeom);
    //    set_curFrame(skinning->get_curFrame());
    //}

    //void ReducedModel::jumpTo(int frameNo)
    //{
    //    if (frameNo % get_nFrame() == get_curFrame()) return;
    //    skinning->jumpTo(frameNo);
    //    if (bPDB)
    //        pPDB->solve(hairGeom);
    //    set_curFrame(skinning->get_curFrame());
    //}

    ////////////////////////////////////////////////////////////////
    /////////////////////////SkinningEngine/////////////////////////
    ////////////////////////////////////////////////////////////////

    //SkinningAndHairBodyCollisionEngine::SkinningAndHairBodyCollisionEngine()
    //{
    //    skinInfo = new SkinningInfo;
    //    reader = new ReconsReader;
    //}

    //SkinningAndHairBodyCollisionEngine::~SkinningAndHairBodyCollisionEngine()
    //{
    //    if (skinInfo)
    //    {
    //        SAFE_DELETE(skinInfo->restState);
    //        SAFE_DELETE(skinInfo->guidances);
    //        for (auto ptr : skinInfo->neighbor)
    //            delete ptr;

    //        delete skinInfo;
    //        skinInfo = nullptr;
    //    }

    //    SAFE_DELETE(reader);
    //    SAFE_DELETE(sampler);
    //}

    //SkinningAndHairBodyCollisionEngineCPU::SkinningAndHairBodyCollisionEngineCPU(bool flag)
    //{
    //    bHairBody = flag;
    //    hairRendererVersion = std::stoi(g_paramDict["hairrendversion"]);
    //    if (bHairBody)
    //        pCollision = WR::CreateGridCollisionObject(g_paramDict["collisionFile"].c_str());
    //    //pCollision = WR::loadCollisionObject(ADF_FILE);
    //}

    //SkinningAndHairBodyCollisionEngineCPU::~SkinningAndHairBodyCollisionEngineCPU()
    //{
    //    SAFE_DELETE(pCollision);
    //}

    //bool SkinningAndHairBodyCollisionEngineCPU::loadFile(const char* fileName, HairGeometry * geom)
    //{
    //    bool hr;
    //    skinResult = geom;
    //    V_RETURN(reader->loadFile(fileName, skinInfo));

    //    int sampleRate = std::stoi(g_paramDict["hairsample"]);
    //    int groupSampleNumber = std::stoi(g_paramDict["hairsamplegroup"]);
    //    int groupSampleSeed = std::stoi(g_paramDict["hairsamplegroupseed"]);
    //    sampler = new HairSampleSelector(skinInfo->restState, sampleRate, groupSampleNumber, groupSampleSeed);
    //    sampler->FillInHairStructs(skinResult);

    //    skinResult->allocMemory();
    //    DirectX::XMStoreFloat4x4(&skinResult->worldMatrix, DirectX::XMMatrixIdentity());

    //    set_nFrame(reader->get_nFrame());
    //    set_curFrame(-1);

    //    return true;
    //}

    //void SkinningAndHairBodyCollisionEngineCPU::rewind()
    //{
    //    if (get_curFrame() == 0) return;

    //    reader->rewind();
    //    interpolate();
    //    if (bHairBody)
    //        hairBodyCollision();

    //    set_curFrame(0);
    //}

    //void SkinningAndHairBodyCollisionEngineCPU::nextFrame()
    //{
    //    reader->nextFrame();
    //    interpolate();
    //    if (bHairBody)
    //        hairBodyCollision();

    //    set_curFrame(reader->get_curFrame());
    //}

    //void SkinningAndHairBodyCollisionEngineCPU::jumpTo(int frameNo)
    //{
    //    if (get_curFrame() == frameNo) return;

    //    reader->jumpTo(frameNo);
    //    interpolate();
    //    if (bHairBody)
    //        hairBodyCollision();
    //    set_curFrame(reader->get_curFrame());
    //}

    //void SkinningAndHairBodyCollisionEngineCPU::interpolate()
    //{
    //    CopyMemory(skinResult->rigidTrans, skinInfo->guidances->rigidTrans, sizeof(float) * 16);
    //    mat4x4 rigid;
    //    mat4x4_transpose(rigid, reinterpret_cast<vec4*>(skinResult->rigidTrans));
    //    auto factor = skinInfo->guidances->particlePerStrand;
    //    sampler->ResetIterator();
    //    for (int i = 0; i < skinResult->nStrand; i++)
    //    {
    //        int thisId = sampler->GetNextId();
    //        auto& weight = skinInfo->weights[thisId];
    //        for (int i2 = 0; i2 < factor; i2++)
    //        {
    //            size_t idx = i * factor + i2;
    //            size_t idxNosample = factor * thisId + i2;
    //            vec3 tmp{ 0, 0, 0 }, tmp2;
    //            for (int j = 0; j < weight.n; j++)
    //            {
    //                size_t guideId = weight.guideID[j];
    //                vec3_scale(tmp2,
    //                    reinterpret_cast<float*>(&skinInfo->guidances->trans[guideId*factor + i2]),
    //                    weight.weights[j]);
    //                vec3_add(tmp, tmp, tmp2);
    //            }
    //            auto dest = reinterpret_cast<float*>(skinResult->position + idx);
    //            auto rest = reinterpret_cast<float*>(&skinInfo->restState->position[idxNosample]);
    //            vec3 pos;
    //            mat4x4_mul_vec3(pos, rigid, rest);
    //            vec3_add(dest, pos, tmp);
    //        }

    //        if (hairRendererVersion == 1)
    //        {
    //            vec3 dir;
    //            float* pos1, *pos2;
    //            for (int j = 1; j < factor; j++)
    //            {
    //                pos1 = reinterpret_cast<float*>(skinResult->position + i*factor + j);
    //                pos2 = reinterpret_cast<float*>(skinResult->position + i*factor + j + 1);
    //                vec3_sub(dir, pos2, pos1);
    //                float norm = vec3_len(dir);
    //                vec3_scale(dir, dir, 1 / norm);
    //                memcpy(skinResult->direction + i*factor + j, dir, sizeof(vec3));
    //            }
    //            memcpy(skinResult->direction + i*factor, skinResult->direction + i*factor + 1, sizeof(vec3));
    //        }
    //    }
    //}


    //void SkinningAndHairBodyCollisionEngineCPU::hairBodyCollision()
    //{
    //    mat4x4 rigid, rigidInv;
    //    mat4x4_transpose(rigid, reinterpret_cast<vec4*>(skinResult->rigidTrans));
    //    mat4x4_invert(rigidInv, rigid);
    //    for (size_t i = 0; i < skinResult->nParticle; i++)
    //    {
    //        if (i % skinResult->particlePerStrand == 0) continue;
    //        vec3 pos; mat4x4_mul_vec3(pos, rigidInv, reinterpret_cast<float*>(skinResult->position + i));
    //        WR::ICollisionObject::Point_3 p1, p0 = WR::ICollisionObject::Point_3(pos[0], pos[1], pos[2]);
    //        bool isCollide = pCollision->position_correlation(p0, &p1, 6e-2f);
    //        if (isCollide)
    //        {
    //            vec3 p;
    //            XR::convert3(p, p1);
    //            mat4x4_mul_vec3(pos, rigid, p);
    //            CopyMemory(skinResult->position + i, pos, sizeof(vec3));
    //        }
    //    }
    //}

}

