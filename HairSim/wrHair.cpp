#include "wrHair.h"
#include <fstream>
#include <iostream>
#include <string>
#include "Parameter.h"
#include "linmath.h"
#include "wrMath.h"
#include "wrLogger.h"
#include "wrMacro.h"
#include "wrSpring.h"

using namespace WR;


namespace
{
	const int REAL_SPRING_DICT[][4] = {
		{ 1, 2, 3, 0 },
		{ 1, 2, 0, 3 },
		{ 1, 3, 2, 0 },
		{ 1, 1, 3, 0 },
		{ 1, 1, 2, 3 }
	};

	const int VIRTUAL_SPRING_DICT[][3] = {
		{ 1, 2, 3 },
		{ 1, 3, 0 }
	};

	int classifyRealParticle(WR::HairParticle* p)
	{
		int code = 0;
		for (int i = 0; i < 3; i++)
			code += ((((p - 1 - i)->isPerturbed()) ? 1 : 0) << i);

		switch (code)
		{
		case 0: return 0;
		case 1: return 1;
		case 2: return 2;
		case 5: return 3;
		case 4: return 4;
		default:
			assert(0);
			return -1;
		}
	}

	int classifyVirtualParticle(WR::HairParticle* p)
	{
		if ((p - 2)->isPerturbed()) return 0;
		else return 1;
	}

	void genRandParticle(float* r, const float* a, const float* b)
	{
		vec3 diff;
		vec3_sub(diff, b, a);

		vec3 randv = { randSignedFloat(), randSignedFloat(), randSignedFloat() };
		while (isZero(randv) || isCollinear(diff, randv))
		{
			for (int i = 0; i < 3; i++)
				randv[i] = randSignedFloat();
		}

		vec3 perp;
		vec3_mul_cross(perp, diff, randv);
		vec3_norm(perp, perp);

		float l = vec3_len(diff) * 0.8660254f; // sqrt(3)/2
		vec3_scale(r, perp, l);

		vec3 middle;
		vec3_add(middle, a, b);
		vec3_scale(middle, middle, 0.5f);

		vec3_add(r, r, middle);
	}
}

namespace WR
{
	Hair* HairParticle::m_hair = nullptr;
	Hair* HairStrand::m_hair = nullptr;

	Hair* loadFile(wchar_t* path)
	{
		std::ifstream file(path, std::ios::binary);
		if (file)
		{
			Hair *hair = nullptr;
			char cbuffer[sizeof(float) * 3 * N_PARTICLES_PER_STRAND];

			file.read(cbuffer, sizeof(int));
			int n_particles = *reinterpret_cast<int*>(cbuffer);

			file.seekg(3 * n_particles * sizeof(float) + sizeof(int));
			file.read(cbuffer, sizeof(int));
			int n_strands = *reinterpret_cast<int*>(cbuffer);

#ifdef COMPRESS
			n_strands /= COMPRESS_RATIO;
			n_particles /= COMPRESS_RATIO;
#endif

			wprintf(L"Loading %s, total particles: %d, total strands: %d\n", path, n_particles, n_strands);

			file.seekg(sizeof(int));
			hair = new Hair;
			hair->reserve(n_particles, n_strands);
			for (int i = 0; i < n_strands; i++)
			{
				file.read(cbuffer, sizeof(float) * 3 * N_PARTICLES_PER_STRAND);
				if (file.eof())
				{
					WR_LOG_FATAL << "unexpected eof flag";
					SAFE_DELETE(hair);
					break;
				}
				hair->add_strand(reinterpret_cast<float*>(cbuffer));

#ifdef COMPRESS
				file.seekg(sizeof(int) + (COMPRESS_RATIO * i + static_cast<int>(COMPRESS_RATIO * randf())) * sizeof(float) * 3 * N_PARTICLES_PER_STRAND);
#endif
			}

			file.close();
			return hair;
		}
		else return nullptr;
	}

	// n is not used here
	bool Hair::add_strand(float* positions, size_t n)
	{
		vec3* pos = reinterpret_cast<vec3*>(positions);

		// compute all edge springs length
		vec3 edgeSprings[N_PARTICLES_PER_STRAND - 1];
		for (int i = 0; i < N_PARTICLES_PER_STRAND - 1; i++)
			vec3_sub(edgeSprings[i], pos[i + 1], pos[i]);

		// check for any co-linear condition
		// the index means a virtual node should be added AFTER the (i)th real node
		std::vector<size_t> collinearPos;
		bool flag = false; // indicate that the next particle is added
		for (int i = 0; i < N_PARTICLES_PER_STRAND - 2; i++)
		{
			// totally n-1 edges, then n-2 comparison
			if (isCollinear(edgeSprings[i], edgeSprings[i + 1]))
			{
				if (!flag) collinearPos.push_back(i);

				collinearPos.push_back(i + 1);
				flag = true;
			}
			else flag = false;
		}

		size_t nParticles = N_PARTICLES_PER_STRAND + collinearPos.size() + 2;
		m_strands.emplace_back();
		auto &strand = m_strands.back();
		strand.reserve(nParticles);
		const float mass_1 = 1.f / PARTICLE_MASS;

		// apply the memory, two extra virtual particles for the root
		// generate 2 virtual (but with [isVirtual] = false) hair root particles
		vec3 p[2];
		vec3_sub(p[0], pos[0], edgeSprings[0]);
		add_particle(strand, p[0], mass_1, false, true, false);

		genRandParticle(p[1], p[0], pos[0]);
		add_particle(strand, p[1], mass_1, false, true, false);

		// copy information into normal particles
		bool isThisVirtual = false;
		for (size_t i = 2, i1 = 0, i2 = 0; i < nParticles; i++)
		{
			vec3 p;
			if (isThisVirtual)
			{
				isThisVirtual = false;
				genRandParticle(p, pos[i1], pos[i1 - 1]);
				add_particle(strand, p, mass_1, true, false, false);
			}
			else
			{
				if (i == 2) add_particle(strand, pos[i1], mass_1, false, true, true);
				else add_particle(strand, pos[i1], mass_1);

				if (collinearPos.size() > i2 && collinearPos[i2] == i1)
				{
					isThisVirtual = true;
					++i2;
				}
				++i1;
			}
		}

		// the last particle has a half mass
		m_particles[strand.get_particle(-1)].set_mass_1(mass_1 * 2);

		return true;
	}

	void Hair::add_particle(HairStrand& strand, const vec3& pos, float mass_1, bool isPerturbed, bool isFixedPos, bool isVisible)
	{
		size_t idx = add_particle(pos, mass_1, isPerturbed, isFixedPos);
		strand.push_back(idx, isVisible);
	}

	size_t Hair::add_particle(const vec3& p, float mass_1, bool isPerturbed, bool isFixedPos)
	{
		Vec3 pos;
		convert3(pos, p);
		size_t id = m_particles.size();
		m_particles.emplace_back(pos, id, isPerturbed, isFixedPos);
		m_particles.back().set_mass_1(mass_1);
		return id;
	}

	void Hair::release()
	{
		mb_simInited = false;
		for (auto spring : m_springs)
			SAFE_DELETE(spring);

		m_particles.clear();
		m_springs.clear();
		m_strands.clear();
		m_segments.clear();
	}

	bool Hair::init_simulation()
	{
		mb_simInited = true;

		init_matrices();
		add_inner_springs();
		
		return true;
	}
	void Hair::init_matrices()
	{
		size_t n = m_particles.size();

		m_position.resize(3 * n);
		m_filter.resize(3 * n);
		m_mass_1.resize(3 * n, 3 * n);
		m_mass.resize(3 * n, 3 * n);
		m_velocity.resize(3 * n);

		m_filter.setOnes();
		m_mass_1.setZero();
		m_mass.setZero();
		//m_mass_1.reserve(1);
		for (size_t i = 0; i < n; i++)
		{
			triple(m_position, i) =  m_particles[i].get_ref();

			if (m_particles[i].isFixedPos())
			{
				triple(m_filter, i) = Vec3::Zero();
			}
			else
			{
				//m_mass_1.insert(3 * i, 3 * i) = m_particles[i].get_mass_1();
				//m_mass_1.insert(3 * i + 1, 3 * i + 1) = m_particles[i].get_mass_1();
				//m_mass_1.insert(3 * i + 2, 3 * i + 2) = m_particles[i].get_mass_1();
				squared_triple(m_mass_1, i) = m_particles[i].get_mass_1() * Mat3::Identity();
				squared_triple(m_mass, i) =  1 / m_particles[i].get_mass_1() * Mat3::Identity();
			}
		}

		m_velocity.setZero();
	}

	void Hair::add_inner_springs()
	{
		size_t n = m_strands.size();
		for (size_t i = 0; i < n; i++)
		{
			auto & strand = m_strands[i];
			size_t np = strand.m_parIds.size();
			for (size_t i = 3; i < np; i++)
				push_springs(strand.m_parIds[i]);
		}
	}

	void Hair::push_springs(int idx)
	{
		if (m_particles[idx].isPerturbed())
		{
			int type = classifyVirtualParticle(&m_particles[idx]);
			for (int i = 0; i < 3; i++)
				push_single_spring(idx, VIRTUAL_SPRING_DICT[type][i]);
		}
		else
		{
			int type = classifyRealParticle(&m_particles[idx]);
			for (int i = 0; i < 4; i++) // TO-DO 可能在第2个粒子上会出问题
				push_single_spring(idx, REAL_SPRING_DICT[type][i]);
		}
	}

	void Hair::push_single_spring(int idx, int stride)
	{
		if (stride)
		{
			auto spring = new BiSpring;
			m_springs.push_back(spring);
			spring->setSpring(stride, &m_particles[idx], &m_particles[idx - stride], K_SPRINGS[stride]);
		}
	}

	void Hair::onFrame(Mat3 world, float fTime, float fTimeElapsed)
	{
		static Mat3 lastWorld = Mat3::Identity();

		float tStep = fTimeElapsed;
		int nPass = 1;
		if (fTimeElapsed > MAX_TIME_STEP)
		{
			nPass = static_cast<int>(fTimeElapsed / MAX_TIME_STEP) + 1;
			tStep = fTimeElapsed / static_cast<float>(nPass);
		}

		if (nPass > MAX_PASS_NUMBER) nPass = MAX_PASS_NUMBER;

		float start = fTime - fTimeElapsed;
		auto matStep = (world - lastWorld) / nPass;
		for (int i = 0; i < nPass; i++)
		{
			lastWorld += matStep;
			step(lastWorld, (start += tStep), tStep);
			//Sleep(500);
		}
	}

	void Hair::step(const Mat3& mWorld, float fTime, float fTimeElapsed)
	{
		const float tdiv2 = fTimeElapsed / 2;

		// modify root node's pos, vel. first 3.
		// 假设固定点都在匀速运动
		for (auto &strand : m_strands)
		{
			for (int j = 0; j < 3; j++)
			{
				size_t idx = strand.get_particle(j);
				Vec3 newPos = get_particle(idx).transposeFromReference(mWorld);
				Vec3 newVel = (newPos - Vec3(get_particle_position(idx))) / fTimeElapsed;
				triple(m_velocity, idx) = newVel;
				//triple(m_position, idx) = newPos;
			}
		}

		size_t dim = m_position.size();
		SparseMat K(dim, dim), B(dim, dim);
		VecX C(dim);

		K.setZero();
		B.setZero();
		C.setZero();

		for (auto &spring : m_springs)
			spring->applyForces(K, B, C);

		// to-do add wind damping

		//SparseMat A  = m_mass + (B + K * tdiv2) * tdiv2;
		SparseMat A(dim, dim);
		A.setIdentity();
		A += m_mass_1 * (B + K * tdiv2) * tdiv2;
		VecX b = - m_mass_1 * tdiv2 * ((K * m_position - C) + (B + K * tdiv2) * m_velocity);

		// z = 0
		VecX dv(dim), dv1(dim);MatX to(21, 4);
		modified_pcg(A, b, dv);
		simple_solve(A, b, dv1);
		filter(dv1, dv);

		//to.col(0) = A * dv - b;
		//to.col(1) = A * dv1 - b;
		//to.col(2) = dv;
		//to.col(3) = dv1;
		//to.col(4) = b;
		////write("a.txt", m_mass_1);
		//write("B.txt", K);
		////write("K.txt", K);
		//WR_LOG_DEBUG << std::endl << to;

		//to.col(3) = m_velocity.block<21, 1>(0, 0);
		//to.col(2) = (K * m_position - C).block<21, 1>(0, 0);
		//to.col(1) = b.block<21, 1>(0, 0);
		//to.col(0) = dv.block<21, 1>(0, 0);

		//WR_LOG_DEBUG << std::endl << to;

		m_velocity += 2 * dv;
		m_position += m_velocity * fTimeElapsed;
	}

	void Hair::simple_solve(const SparseMat& A, const VecX& b, VecX& x) const
	{
		x = A.ldlt().solve(b);
	}

	void Hair::modified_pcg(const SparseMat& A, const VecX& b, VecX& dv) const
	{
		const size_t dim = b.size();

		SparseMat P(dim, dim), P_1(dim, dim);
		P_1.setZero();
		//P.reserve(1);
		//P_1.reserve(1);
		for (size_t i = 0; i < dim; i++)
		{
			//P.insert(i, i) = 1.f / A.coeff(i, i);
			//P_1.insert(i, i) = A.coeff(i, i);
			P_1(i, i) = A(i, i);
		}
		P = P_1.inverse();

		VecX b_f(dim), r(dim), c(dim), q(dim), s(dim);
		float dnew, dold, a;

		const float tol = 1e-5, tol_square = tol * tol;

		dv.setZero();
		filter(b, b_f);
		const float delta0 = b_f.transpose() * P * b_f;
		filter(b - A*dv, r);
		filter(P_1 * r, c);

		dnew = r.transpose() * c;

		const float thresh = tol_square * delta0;
		while (dnew > thresh)
		{
			filter(A * c, q);
			a = dnew / (c.transpose() * q);
			dv += a * c;
			r -= a * q;
			s = P_1 * r;
			dold = dnew;
			dnew = r.transpose() * s;
			filter(s + (dnew / dold) * c, c);
		}
	}

	void Hair::scale(float x)
	{
		for (auto &p : m_particles)
			p.m_ref *= x;
	}

	void Hair::mirror(bool x, bool y, bool z)
	{
		for (auto &p : m_particles)
		{
			if (x) p.m_ref.x() = -p.m_ref.x();
			if (y) p.m_ref.y() = -p.m_ref.y();
			if (z) p.m_ref.z() = -p.m_ref.z();
		}
	}

}