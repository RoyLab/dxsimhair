#include "DXUT.h"
#include "wrHairSimulator.h"
#include "wrLogger.h"

namespace
{
    const float MAX_TIME_STEP = 10.0f * 1.0e-3f;
    const float K_SPRINGS[N_SPRING_USED] = { 5.0e-2f };
    const vec3 GRAVITY = { 0.0f, -10.0f, 0.0f };
    const int MAX_PASS_NUMBER = 5;
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

    WR_LOG_DEBUG << " TIME: " << fTime << " e: " << fTimeElapsed << " nPass: " << nPass << std::endl;
}


void wrHairSimulator::step(wrHair* hair, float fTime, float fTimeElapsed)
{
    auto pStrands = hair->getStrands();
    int n_strands = hair->n_strands();

    for (int i = 0; i < n_strands; i++)
    {
        auto &strand = pStrands[i];
        for (int j = 0; j < N_PARTICLES_PER_STRAND; j++)
        {
            auto &particle = strand.getParticles()[j];
            vec3_rz(particle.force);

            // apply springs
            vec3 diff[N_SPRING_USED];
            for (int k = 0; k < N_SPRING_USED; k++)
            {
                auto sibling = particle.siblings[k];
                if (sibling)
                {
                    vec3 diff;
                    vec3_sub(diff, sibling->position, particle.position);

                    float cLen = vec3_len(diff);
                    float fScalar = K_SPRINGS[k] * (cLen - particle.springLens[k]) / particle.springLens[k];
                    vec3 force;
                    vec3_scale(force, diff, fScalar);
                    
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
            vec3_scale(dx, particle.v_middle, fTimeElapsed);
            vec3_add(particle.position, particle.position, dx);
        }

    } // total hair

    auto &sampler = hair->getStrand(3).getParticles()[3];
    //WR_LOG_TRACE << "pos: " << sampler.position[0] << " " << sampler.position[1] << " " << sampler.position[1] << std::endl;
    //WR_LOG_TRACE << "force: " << sampler.force[0] << " " << sampler.force[1] << " " << sampler.force[1] << std::endl;
    //WR_LOG_TRACE << "vel: " << sampler.velocity[0] << " " << sampler.velocity[1] << " " << sampler.velocity[1] << std::endl;
}
