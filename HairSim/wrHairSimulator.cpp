#include "precompiled.h"
#include "wrHairSimulator.h"
#include "wrLogger.h"

namespace
{
    const float MAX_TIME_STEP = 5.0e-3f;
    const float K_SPRINGS[N_SPRING_USED] = { 2.0e-4f };
    const vec3 GRAVITY = { 0.0f, -10.0f, 0.0f };
    const int MAX_PASS_NUMBER = 2;
}


wrHairSimulator::wrHairSimulator()
{
}


wrHairSimulator::~wrHairSimulator()
{
}


bool wrHairSimulator::init(wrHair* hair)
{
    auto pStrands = hair->getStrands();
    int n_strands = hair->n_strands();
    for (int i = 0; i < n_strands; i++)
    {
        auto& strand = pStrands[i];
        auto particles = strand.getParticles();
        for (int j = 0; j < N_PARTICLES_PER_STRAND; j++)
        {
            if (j + 1 < N_PARTICLES_PER_STRAND)
            {
                auto n1 = particles + j + 1;
                particles[j].siblings[0] = n1;
                vec3_sub(particles[j].diffs[0], n1->position, particles[j].position);
                particles[j].springLens[0] = vec3_len(particles[j].diffs[0]);
            }
        }
    }

    return false;
}


void wrHairSimulator::onFrame(wrHair* hair, float fTime, float fTimeElapsed)
{
    float tStep = fTimeElapsed;
    int nPass = 1;
    if (fTimeElapsed > MAX_TIME_STEP)
    {
        nPass = static_cast<int>(fTimeElapsed / MAX_TIME_STEP) + 1;
        tStep = fTimeElapsed / static_cast<float>(nPass);
    }

    if (nPass > MAX_PASS_NUMBER) nPass = MAX_PASS_NUMBER;

    float start = fTime - fTimeElapsed;
    for (int i = 0; i < nPass; i++)
    {
        step(hair, (start += tStep), tStep);
        //Sleep(500);
    }

    //WR_LOG_DEBUG << " TIME: " << fTime << " e: " << fTimeElapsed << " nPass: " << nPass << std::endl;
}


void wrHairSimulator::step(wrHair* hair, float fTime, float fTimeElapsed)
{
    auto pStrands = hair->getStrands();
    int n_strands = hair->n_strands();

    for (int i = 0; i < n_strands; i++)
    {
        auto &strand = pStrands[i];
        for (int j = 0; j < N_PARTICLES_PER_STRAND; j++)
            vec3_rz(strand.getParticles()[j].force);

        for (int j = 0; j < N_PARTICLES_PER_STRAND; j++)
        {
            auto &particle = strand.getParticles()[j];

            // apply springs
            for (int k = 0; k < N_SPRING_USED; k++)
            {
                auto sibling = particle.siblings[k];
                if (sibling)
                {
                    vec3 diff;
                    vec3_sub(diff, sibling->position, particle.position);

                    float cLen = vec3_len(diff);
#ifdef NUMERICAL_TRACE
                    particle.cLen = cLen;
#endif
                    float fScalar = K_SPRINGS[k] * (cLen - particle.springLens[k]) / particle.springLens[k];
                    vec3 force;
                    vec3_norm(force, diff);
                    vec3_scale(force, force, fScalar);
                    
                    vec3_add(particle.force, particle.force, force);
                    vec3_sub(sibling->force, sibling->force, force);
                }
            }
        } // per strand end

        // compute v[n+1/2]
        for (int j = 0; j < N_PARTICLES_PER_STRAND; j++)
        {
            auto &particle = strand.getParticles()[j];

            vec3 acc;
            vec3_scale(acc, particle.force, particle.mass_1);
            vec3_add(acc, acc, GRAVITY); 
#ifdef NUMERICAL_TRACE
            vec3_copy(particle.acc1, acc);
#endif

            vec3 dv;
            vec3_scale(dv, acc, fTimeElapsed / 2.0f);
            vec3_add(particle.v_middle, particle.velocity, dv);
        }

        // apply constrain
        vec3_rz(strand.getParticles()[0].v_middle);

        // compute x[n+1]
        for (int j = 0; j < N_PARTICLES_PER_STRAND; j++)
        {
            auto &particle = strand.getParticles()[j];
            vec3 dx;
            vec3_scale(dx, particle.v_middle, fTimeElapsed / 2.0f);
            vec3_add(particle.pos_middle, particle.position, dx);
            vec3_add(particle.position, particle.pos_middle, dx);
        }

        // discard v, recomputing
        for (int j = 0; j < N_PARTICLES_PER_STRAND; j++)
            vec3_rz(strand.getParticles()[j].force);

        for (int j = 0; j < N_PARTICLES_PER_STRAND; j++)
        {
            auto &particle = strand.getParticles()[j];

            // apply springs
            for (int k = 0; k < N_SPRING_USED; k++)
            {
                auto sibling = particle.siblings[k];
                if (sibling)
                {
                    vec3 diff;
                    vec3_sub(diff, sibling->pos_middle, particle.pos_middle);

                    float cLen = vec3_len(diff);
                    float fScalar = K_SPRINGS[k] * (cLen - particle.springLens[k]) / particle.springLens[k];
                    vec3 force;
                    vec3_norm(force, diff);
                    vec3_scale(force, force, fScalar);

                    vec3_add(particle.force, particle.force, force);
                    vec3_sub(sibling->force, sibling->force, force);
                }
            }
        } // per strand end

        // compute v[n+1/2]
        for (int j = 0; j < N_PARTICLES_PER_STRAND; j++)
        {
            auto &particle = strand.getParticles()[j];

            vec3 acc;
            vec3_scale(acc, particle.force, particle.mass_1);
            vec3_add(acc, acc, GRAVITY);
#ifdef NUMERICAL_TRACE
            vec3_copy(particle.acc2, acc);
#endif

            vec3 dv;
            vec3_scale(dv, acc, fTimeElapsed / 2.0f);
            vec3_add(particle.v_middle, particle.velocity, dv);

            vec3 v2;
            vec3_scale(v2, particle.v_middle, 2.0f);
            vec3_sub(particle.velocity, v2, particle.velocity);
        }

    } // total hair

#ifdef NUMERICAL_TRACE
    auto &sampler = hair->getStrand(3).getParticles()[1];
    auto &sampler2 = hair->getStrand(3).getParticles()[0];
    WR_LOG_TRACE << std::endl
        << "pos: " << sampler.position[0] << " " << sampler.position[1] << " " << sampler.position[2] << std::endl
        << "pos0: " << sampler2.position[0] << " " << sampler2.position[1] << " " << sampler2.position[2] << std::endl
        << "force: " << sampler.force[0] << " " << sampler.force[1] << " " << sampler.force[2] << std::endl
        << "vel: " << sampler.velocity[0] << " " << sampler.velocity[1] << " " << sampler.velocity[2] << std::endl
        << "len1: " << sampler2.cLen << " " << sampler2.springLens[0] << std::endl
        << "len2: " << sampler.cLen << " " << sampler.springLens[0] << std::endl
        << "acc1: " << sampler.acc1[0] << " " << sampler.acc1[1] << " " << sampler.acc1[2] << std::endl
        << "acc2: " << sampler.acc2[0] << " " << sampler.acc2[1] << " " << sampler.acc2[2] << std::endl;

    WR_LOG_TRACE << "heihei";
#endif
}
