#include "wrHair.h"
#include "xmath.h"
#include <fstream>
#include "xlogger.h"
#include "wrSpring.h"
#include "EigenTypes.h"
#include <iostream>
using namespace std;

#define FULL_IMPLICIT
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

	inline void remove_vertical_comp(const Vec3& n, Vec3& v)
	{
		Vec3 diff = n.normalized();
		v -= v.dot(diff) * diff;
	}

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

		vec3 randv = { randsf(), randsf(), randsf() };
		while (vec3_iszero<3>(randv) || vec3_collinear(diff, randv))
		{
			for (int i = 0; i < 3; i++)
				randv[i] = randsf();
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

	struct StrainLimitPair
	{
		float squared_length;
		int Id[2]; // 0 > 1
	};

	Hair* HairParticle::m_hair = nullptr;
	Hair* HairStrand::m_hair = nullptr;

	Hair* loadFile(const char* path)
	{
		std::ifstream file(path, std::ios::binary);
		if (file)
		{
			Hair *hair = nullptr;
			char *cbuffer = new char[sizeof(float) * 3 * N_PARTICLES_PER_STRAND];

			file.read(cbuffer, sizeof(int));
			int n_particles = *reinterpret_cast<int*>(cbuffer);

			file.seekg(3 * n_particles * sizeof(float) + sizeof(int));
			file.read(cbuffer, sizeof(int));
			int n_strands = *reinterpret_cast<int*>(cbuffer);

			n_strands /= COMPRESS_RATIO;
			n_particles /= COMPRESS_RATIO;

			wprintf(L"Loading %s, total particles: %d, total strands: %d\n", path, n_particles, n_strands);

			file.seekg(sizeof(int));
			hair = new Hair;
			hair->reserve(n_particles, n_strands);
			for (int i = 0; i < n_strands; i++)
			{
				file.read(cbuffer, sizeof(float) * 3 * N_PARTICLES_PER_STRAND);
				if (file.eof())
				{
					XLOG_FATAL << "unexpected eof flag";
					SAFE_DELETE(hair);
					break;
				}
				hair->add_strand(reinterpret_cast<float*>(cbuffer));

				file.seekg(sizeof(int) + (COMPRESS_RATIO * i + static_cast<int>(COMPRESS_RATIO * randf())) * sizeof(float) * 3 * N_PARTICLES_PER_STRAND);
			}

			file.close();

			SAFE_DELETE_ARRAY(cbuffer);
			return hair;
		}
		else return nullptr;
	}

	// n is not used here
	bool Hair::add_strand(float* positions, size_t n)
	{
		vec3* pos = reinterpret_cast<vec3*>(positions);

		// compute all edge springs length
		vec3 *edgeSprings = new vec3[N_PARTICLES_PER_STRAND - 1];
		for (int i = 0; i < N_PARTICLES_PER_STRAND - 1; i++)
			vec3_sub(edgeSprings[i], pos[i + 1], pos[i]);

		// check for any co-linear condition
		// the index means a virtual node should be added AFTER the (i)th real node
		std::vector<size_t> collinearPos;
		bool flag = false; // indicate that the next particle is added
		for (int i = 0; i < N_PARTICLES_PER_STRAND - 2; i++)
		{
			// totally n-1 edges, then n-2 comparison
			if (vec3_collinear(edgeSprings[i], edgeSprings[i + 1]))
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

		// apply the memory, two extra virtual particles for the root
		// generate 2 virtual (but with [isVirtual] = false) hair root particles
		vec3 p[2];
		vec3_sub(p[0], pos[0], edgeSprings[0]);
		add_particle(strand, p[0], &PARTICLE_MASS, false, true, false);

		genRandParticle(p[1], p[0], pos[0]);
		add_particle(strand, p[1], &PARTICLE_MASS, false, true, false);

		// copy information into normal particles
		bool isThisVirtual = false;
		for (size_t i = 2, i1 = 0, i2 = 0; i < nParticles; i++)
		{
			vec3 p;
			if (isThisVirtual)
			{
				isThisVirtual = false;
				genRandParticle(p, pos[i1], pos[i1 - 1]);
				add_particle(strand, p, &PARTICLE_MASS, true, false, false);
			}
			else
			{
				if (i == 2) add_particle(strand, pos[i1], &PARTICLE_MASS, false, true, true);
				else add_particle(strand, pos[i1], &PARTICLE_MASS);

				if (collinearPos.size() > i2 && collinearPos[i2] == i1)
				{
					isThisVirtual = true;
					++i2;
				}
				++i1;
			}
		}

		// the last particle has a half mass
		//m_particles[strand.get_particle(-1)].set_mass_1(mass_1 * 2);
		m_particles[strand.get_particle(-1)].setMassPointer(&PARTICLE_MASS);

		SAFE_DELETE_ARRAY(edgeSprings);

		return true;
	}

	void Hair::add_particle(HairStrand& strand, const vec3& pos, float *mass, bool isPerturbed, bool isFixedPos, bool isVisible)
	{
		//size_t idx = add_particle(pos, mass, isPerturbed, isFixedPos);

		Vec3 posVec3;
		posVec3 << pos[0], pos[1], pos[2];

		size_t id = m_particles.size();
		size_t localId = strand.m_parIds.size();

		m_particles.emplace_back(posVec3, id, localId, isPerturbed, isFixedPos);
		m_particles.back().setMassPointer(mass);

		strand.push_back(id, isVisible);
	}

	//size_t Hair::add_particle(const vec3& p, float *mass, bool isPerturbed, bool isFixedPos)
	//{
	//	Vec3 pos;
	//	pos << p[0], p[1], p[2];
	//	size_t id = m_particles.size();
	//	m_particles.emplace_back(pos, id, isPerturbed, isFixedPos);
	//	m_particles.back().setMassPointer(mass);
	//	return id;
	//}

	void Hair::release()
	{
		mb_simInited = false;
		for (auto spring : m_springs)
			SAFE_DELETE(spring);

		for (auto limit : m_strain_limits)
			SAFE_DELETE(limit);

		m_particles.clear();
		m_springs.clear();
		m_strands.clear();
		m_segments.clear();
		m_strain_limits.clear();
	}

	bool Hair::init_simulation()
	{
		mb_simInited = true;

		init_matrices();
		add_inner_springs();
		add_strain_limits();

		return true;
	}
	void Hair::init_matrices()
	{
		size_t n = m_particles.size();

		m_position.resize(3 * n);

		m_velocity.resize(3 * n);
		m_velocity.setZero();

		m_filter.resize(3 * n);
		m_filter.setOnes();

		m_gravity.resize(3 * n);
		m_gravity.setZero();
		for (size_t i = 0; i < n; i++)
			triple(m_gravity, i) = Vec3(GRAVITY) / m_particles[i].getMass_1();

		m_mass_1.resize(3 * n, 3 * n);
		m_mass_1.reserve(VecX::Constant(3 * n, 1));

		m_mass.resize(3 * n, 3 * n);
		m_mass.reserve(VecX::Constant(3 * n, 1));

		m_wind_damping.resize(3 * n, 3 * n);
		m_wind_damping.reserve(VecX::Constant(3 * n, 1));

		for (size_t i = 0; i < 3 * n; i++)
			m_wind_damping.insert(i, i) = WIND_DAMPING_COEF;

		for (size_t i = 0; i < n; i++)
		{
			triple(m_position, i) = m_particles[i].get_ref();

			float mass = 1.f / m_particles[i].getMass_1();
			m_mass.insert(3 * i, 3 * i) = mass;
			m_mass.insert(3 * i + 1, 3 * i + 1) = mass;
			m_mass.insert(3 * i + 2, 3 * i + 2) = mass;

			if (m_particles[i].isFixedPos())
			{
				triple(m_filter, i) = Vec3::Zero();
			}
			else
			{
				float mass_1 = m_particles[i].getMass_1();

				m_mass_1.insert(3 * i, 3 * i) = mass_1;
				m_mass_1.insert(3 * i + 1, 3 * i + 1) = mass_1;
				m_mass_1.insert(3 * i + 2, 3 * i + 2) = mass_1;
			}
		}

	}

	void Hair::add_strain_limits()
	{
		StrainLimitPair* data;
		size_t n = m_strands.size();
		for (size_t i = 0; i < n; i++)
		{
			auto & strand = m_strands[i];
			size_t np = strand.m_parIds.size();
			for (size_t i = 4; i < np; i++)
			{
				data = new StrainLimitPair;

				int idx = strand.m_parIds[i];
				int idx2 = strand.m_parIds[i - 1];

				if (m_particles[idx].isPerturbed())
					data->Id[1] = idx2;
				else
				{
					if (m_particles[idx2].isPerturbed())
						data->Id[1] = strand.m_parIds[i - 2];
					else
						data->Id[1] = idx2;
				}

				data->Id[0] = idx;
				Vec3 diff = m_particles[data->Id[0]].get_ref() - m_particles[data->Id[1]].get_ref();
				data->squared_length = diff.dot(diff);
				m_strain_limits.push_back(data);
			}
		}
	}

	void Hair::add_inner_springs()
	{
		size_t n = m_strands.size();
		for (size_t i = 0; i < n; i++)
		{
			auto & strand = m_strands[i];
			size_t np = strand.m_parIds.size();
			for (size_t i = 3; i < np; i++)
				push_springs(strand, strand.m_parIds[i]);
		}
	}

	void Hair::push_springs(HairStrand &strand, int idx)
	{
		if (m_particles[idx].isPerturbed())
		{
			int type = classifyVirtualParticle(&m_particles[idx]);
			for (int i = 0; i < 3; i++)
				push_single_spring(strand, idx, VIRTUAL_SPRING_DICT[type][i]);
		}
		else
		{
			int type = classifyRealParticle(&m_particles[idx]);
			for (int i = 0; i < 4; i++) // TO-DO 可能在第2个粒子上会出问题
				push_single_spring(strand, idx, REAL_SPRING_DICT[type][i]);
		}
	}

	void Hair::push_single_spring(HairStrand &strand, int idx, int stride)
	{
		if (stride)
		{
			auto spring = new BiSpring;
			m_springs.push_back(spring);
			spring->setSpring(stride, &m_particles[idx], &m_particles[idx - stride], &K_SPRINGS[stride]);
			strand.m_springPointers.push_back(spring);
		}
	}

	void Hair::onFrame(Mat3 world, float fTime, float fTimeElapsed)
	{
		//static Mat3 lastWorld = Mat3::Identity();

		//float tStep = fTimeElapsed;
		//int nPass = 1;
		//if (fTimeElapsed > TIME_STEP)
		//{
		//	nPass = static_cast<int>(fTimeElapsed / TIME_STEP) + 1;
		//	tStep = fTimeElapsed / static_cast<float>(nPass);
		//}

		//if (nPass > MAX_PASS_NUMBER) nPass = MAX_PASS_NUMBER;

		//float start = fTime - fTimeElapsed;
		//auto matStep = (world - lastWorld) / nPass;
		//for (int i = 0; i < nPass; i++)
		//{
		//	lastWorld += matStep;
		//	step(lastWorld, (start += tStep), tStep);
		//	//std::cout << std::setprecision(2) << tStep << ' ';
		//	//Sleep(500);
		//}
	}

	void Hair::step(const Mat4& mWorld, float fTime, float fTimeElapsed, ICollisionObject *collisionObj, const Mat4& mWrold2Collision)
	{
		cout << "step" << endl;
		assert(mb_simInited);

		// modify root node's pos, vel. first 3.
		// 假设固定点都在匀速运动
		int g_count = 0;
		for (auto &strand : m_strands)
		{
			for (int j = 0; j < 3; j++)
			{
				size_t idx = strand.get_particle(j);
				Vec3 ref = get_particle(idx).get_ref();
				Vec3 newPos = (mWorld * Vec4(ref(0), ref(1), ref(2), 1.0)).segment(0, 3);
				Vec3 newVel = (newPos - Vec3(get_particle_position(idx))) / fTimeElapsed;
				triple(m_velocity, idx) = newVel;
			}

			int si = strand.get_particle(0) * 3, sn = strand.m_parIds.size() * 3;
			auto pos_seg = m_position.segment(si, sn);
			auto vel_seg = m_velocity.segment(si, sn);
			VecX dv = extract_dv(strand, fTimeElapsed);
			//if (si <= 1377 && si + sn > 1377) {
			//	cout << "before" << endl;
			//	for (int i = 0; i < sn; i += 3) {
			//		std::cout << si + i << ": dv=(" << dv(i) << ',' << dv(i + 1) << ',' << dv(i + 2) << ")  pos=(" << pos_seg(i) << ',' << pos_seg(i + 1) << ',' << pos_seg(i + 2) << ")  vel=(" << vel_seg(i) << ',' << vel_seg(i + 1) << ',' << vel_seg(i + 2) << ')' << endl;;
			//	}
			//	std::cout << std::endl;
			//}
			vel_seg += dv;
			pos_seg += vel_seg * fTimeElapsed;

			g_count++;
		}

		if (collisionObj && APPLY_COLLISION) {

			using Point3 = ICollisionObject::Point_3;

			for (int i = 0; i < m_position.rows(); i += 3) {
				Point3 temp = Point3(m_position(i), m_position(i + 1), m_position(i + 2));
				bool correct = collisionObj->position_correlation(temp, &temp);
				if (correct) {
					m_position(i) = temp[0];
					m_position(i + 1) = temp[1];
					m_position(i + 2) = temp[2];
				}
			}
		}

		//int si = 1377, sn = 81;
		//auto pos_seg = m_position.segment(si, sn);
		//auto vel_seg = m_velocity.segment(si, sn);
		//if (si <= 1377 && si + sn > 1377) {
		//	cout << "after" << endl;
		//	for (int i = 0; i < sn; i += 3) {
		//		std::cout << si + i << ": pos=(" << pos_seg(i) << ',' << pos_seg(i + 1) << ',' << pos_seg(i + 2) << ")  vel=(" << vel_seg(i) << ',' << vel_seg(i + 1) << ',' << vel_seg(i + 2) << ')' << endl;;
		//	}
		//	std::cout << std::endl;
		//}

		//size_t dim = m_position.size();
		//SparseMatAssemble K(dim, dim), B(dim, dim);
		//K.reserve(VecX::Constant(dim, 30));
		//B.reserve(VecX::Constant(dim, 30));

		//K.reserve_hash_map(10 * m_particles.size());
		//B.reserve_hash_map(10 * m_particles.size());

		//VecX C(dim);
		//C.setZero();

		//for (auto &spring : m_springs)
		//	spring->applyForces(K, B, C);

		//K.flush();
		//B.flush();

		//SparseMat T = B + m_wind_damping + K * fTimeElapsed;

		//VecX dv(dim);

		//if (APPLY_PCG)
		//{
		//	SparseMat A = m_mass + T * fTimeElapsed;
		//	VecX b = -fTimeElapsed * (((K * m_position - C) + T * m_velocity) - m_gravity);
		//	modified_pcg(A, b, dv);
		//}
		//else
		//{
		//	SparseMat A(dim, dim);
		//	A.setIdentity();
		//	A += m_mass_1 * T * fTimeElapsed;

		//	VecX b = m_mass_1 * (-fTimeElapsed * (((K * m_position - C) + T * m_velocity) - m_gravity));
		//	LU(A, b, dv);
		//}


		//if (APPLY_STRAINLIMIT) {
		//	m_position += (m_velocity + dv) * fTimeElapsed;
		//	resolve_strain_limits(m_position, m_velocity, fTimeElapsed);

		//	if (APPLY_PCG)
		//	{
		//		SparseMat A = m_mass + T * fTimeElapsed;
		//		VecX b = -fTimeElapsed * (((K * m_position - C) + T * m_velocity) - m_gravity);
		//		modified_pcg(A, b, dv);
		//	}
		//	else
		//	{
		//		SparseMat A(dim, dim);
		//		A.setIdentity();
		//		A += m_mass_1 * T * fTimeElapsed;

		//		VecX b = m_mass_1 * (-fTimeElapsed * (((K * m_position - C) + T * m_velocity) - m_gravity));
		//		LU(A, b, dv);
		//	}

		//	m_velocity += dv;
		//}
		//else {
		//	m_velocity += dv;
		//	m_position += m_velocity * fTimeElapsed;
		//}

		//if (APPLY_COLLISION)
		//resolve_body_collision(mWorld, m_position, m_velocity, fTimeElapsed);

		//for (size_t i = 0; i < dim; i++)
		//    if (std::isnan(newPos[i]))
		//        std::cout << i << std::endl;

		//m_position = newPos;
	}

	//VecX extract_dv(const MatX &K, const MatX &B, const VecX &C, Eigen::VectorBlock<VecX> pos, Eigen::VectorBlock<VecX> vel, const float fTimeElapsed) {
	//	//SparseMat T = B + m_wind_damping + K * fTimeElapsed;

	//	//VecX dv(dim);

	//	//if (APPLY_PCG)
	//	//{
	//	//	SparseMat A = m_mass + T * fTimeElapsed;
	//	//	VecX b = -fTimeElapsed * (((K * m_position - C) + T * m_velocity) - m_gravity);
	//	//	modified_pcg(A, b, dv);
	//	//}
	//	//else
	//	//{
	//	//	SparseMat A(dim, dim);
	//	//	A.setIdentity();
	//	//	A += m_mass_1 * T * fTimeElapsed;

	//	//	VecX b = m_mass_1 * (-fTimeElapsed * (((K * m_position - C) + T * m_velocity) - m_gravity));
	//	//	LU(A, b, dv);
	//	//}

	//	int dim = K.rows();

	//	MatX T = B + K * fTimeElapsed + (MatX::Identity(dim, dim) * WIND_DAMPING_COEF);

	//	MatX A = MatX::Identity(dim, dim) + (1.0 / PARTICLE_MASS * fTimeElapsed) * T;
	//	VecX b = (K * pos - C) + T * vel;
	//	for (int i = 0; i < dim; i += 3) {
	//		b(i) -= GRAVITY[0];
	//		b(i + 1) -= GRAVITY[1];
	//		b(i + 2) -= GRAVITY[2];
	//	}
	//	b *= -(1.0 / PARTICLE_MASS * fTimeElapsed);

	//	return A.ldlt().solve(b);
	//}

	VecX Hair::extract_dv(const HairStrand &strand, const float fTimeElapsed) const {
		//SparseMat T = B + m_wind_damping + K * fTimeElapsed;

		//VecX dv(dim);

		//if (APPLY_PCG)
		//{
		//	SparseMat A = m_mass + T * fTimeElapsed;
		//	VecX b = -fTimeElapsed * (((K * m_position - C) + T * m_velocity) - m_gravity);
		//	modified_pcg(A, b, dv);
		//}
		//else
		//{
		//	SparseMat A(dim, dim);
		//	A.setIdentity();
		//	A += m_mass_1 * T * fTimeElapsed;

		//	VecX b = m_mass_1 * (-fTimeElapsed * (((K * m_position - C) + T * m_velocity) - m_gravity));
		//	LU(A, b, dv);
		//}

		size_t dim = strand.m_parIds.size() * 3;

		float mass = PARTICLE_MASS;
		float mass_1 = 1.0 / PARTICLE_MASS;
		float gravity[3] = { mass * GRAVITY[0], mass * GRAVITY[1], mass * GRAVITY[2] };

		MatX K = MatX::Zero(dim, dim), B = MatX::Zero(dim, dim);
		VecX C = VecX::Zero(dim);

		Eigen::DenseIndex si = strand.get_particle(0) * 3;
		auto pos = m_position.segment(si, dim);
		auto vel = m_velocity.segment(si, dim);

		for (const auto springPtr : strand.m_springPointers) {
			springPtr->applyForces(K, B, C);
		}

		MatX T = B + K * fTimeElapsed + (MatX::Identity(dim, dim) * WIND_DAMPING_COEF);

		MatX A = MatX::Identity(dim, dim) + (mass_1 * fTimeElapsed) * T;
		VecX b = (K * pos - C) + T * vel;
		for (int i = 0; i < dim; i += 3) {
			b(i) -= gravity[0];
			b(i + 1) -= gravity[1];
			b(i + 2) -= gravity[2];
		}
		b *= -(mass_1 * fTimeElapsed);

		return A.ldlt().solve(b);
	}

	void Hair::simple_solve(const MatX& A, const VecX& b, VecX& x) const
	{
		x = A.ldlt().solve(b);
	}

	void Hair::LU(const SparseMat& A, const VecX& b, VecX& dv) const
	{
		Eigen::SparseLU<SparseMat> solver;
		solver.analyzePattern(A);
		solver.factorize(A);
		if (Eigen::Success != solver.info())
		{
			//std::cout << solver.lastErrorMessage() << std::endl;
			system("pause");
			exit(0);
		}
		dv = solver.solve(b);
	}

	void Hair::modified_pcg(const SparseMat& A, const VecX& b, VecX& dv) const
	{
		const size_t dim = b.size();

		SparseMat P(dim, dim), P_1(dim, dim);
		P_1.setZero();
		P.reserve(1);
		P_1.reserve(1);
		for (size_t i = 0; i < dim; i++)
		{
			P.insert(i, i) = 1.f / A.coeff(i, i);
			P_1.insert(i, i) = A.coeff(i, i);
		}

		VecX b_f(dim), r(dim), c(dim), q(dim), s(dim);
		float dnew, dold, a;

		const float tol = 1e-7, tol_square = tol * tol;

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


	void Hair::resolve_body_collision(const Mat3& mWorld, VecX& pos, VecX& vel, float t) const
	{
		//auto mInvWorld = mWorld.inverse();
		//size_t ns = m_strands.size();
		//for (size_t i = 0; i < ns; i++)
		//{
		//	size_t nvp = m_strands[i].m_visibleParticles.size();
		//	for (size_t j = 1; j < nvp; j++)
		//	{
		//		size_t idx = m_strands[i].m_visibleParticles[j];
		//		Vec3 p = mInvWorld * triple(pos, idx);
		//		ICollisionObject::Point_3 p1, p0 = ICollisionObject::Point_3(p[0], p[1], p[2]);
		//		bool isCollide = mp_data->pCollisionHead->position_correlation(p0, &p1, 3e-3f);
		//		if (isCollide)
		//		{
		//			//convert3(p, p1);
		//			p = mWorld * (Vec3() << p1[0], p1[1], p1[2]).finished();
		//			triple(pos, idx) = p;
		//			triple(vel, idx) = (p - Vec3(get_particle_position(idx))) / t;
		//			//triple(vel, idx) = Vec3();
		//		}
		//	}
		//}
	}


	void Hair::resolve_strain_limits(VecX& pos, VecX& vel, float t) const
	{
		bool flag = false;
		for (auto &limit : m_strain_limits)
		{
			//Vec3 diff = Vec3(get_particle_position(limit->Id[0])) - Vec3(get_particle_position(limit->Id[1]));
			Vec3 pred_diff = triple(pos, limit->Id[0]) - triple(pos, limit->Id[1]);
			float sqRatio = pred_diff.dot(pred_diff) / limit->squared_length;

			if (sqRatio > 1.21f)
			{
				flag = true;
				pred_diff *= 1.1 / sqrt(sqRatio);
			}
			else if (sqRatio < 0.81)
			{
				flag = true;
				pred_diff *= 0.9 / sqrt(sqRatio);
			}

			if (flag)
			{
				flag = false;
				Vec3 newpos = pred_diff + triple(pos, limit->Id[1]);
				triple(pos, limit->Id[0]) = newpos;
				//triple(vel, limit->Id[0]) = (pred_diff - diff) / t + triple(vel, limit->Id[1]);
				//triple(vel, limit->Id[0]) = (newpos - Vec3(get_particle_position(limit->Id[0]))) / t;
			}
		}
	}
}