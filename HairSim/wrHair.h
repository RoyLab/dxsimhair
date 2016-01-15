#pragma once
#include "wrMacro.h"
#include "wrTypes.h"
#include "linmath.h"
#include <list>

namespace WR
{
	class Hair;
	class ISpring;

	Hair *loadFile(wchar_t*);

#ifndef N_PARTICLES_PER_STRAND
#define N_PARTICLES_PER_STRAND      25
#endif

	class HairParticle
	{
		COMMON_PROPERTY(float, mass_1);
		COMMON_PROPERTY(Vec3, ref);

		friend class Hair;
	public:
		HairParticle(const Vec3& pos, size_t id, bool isPerturbed = false, bool isFixedPos = false) :
			m_ref(pos), m_Id(id),
			mb_perturbed(isPerturbed), mb_fixedPos(isFixedPos){}

		size_t get_Id() const { return m_Id; }
		bool isPerturbed() const { return mb_perturbed; }
		bool isFixedPos() const { return mb_fixedPos; }

		template <class Matrix>
		Vec3 transposeFromReference(const Matrix& mat) const
		{
			return mat * m_ref;
		}

	private:
		const size_t		m_Id;

		// mark only the split nodes, NOT the root ones.
		const bool      mb_perturbed = false;
		const bool		mb_fixedPos = false;
	};

	class HairStrand
	{
		STATIC_PROPERTY(Hair*, hair);

		friend class Hair;
	public:
		int get_visible_particle(int idx) const	{ return m_visibleParticles[idx]; }
		int get_particle(int idx) const
		{ 
			if (idx >= 0) return m_parIds[idx];
			else return m_parIds[m_parIds.size() + idx];
		}

		void reserve(size_t np, size_t nvp = N_PARTICLES_PER_STRAND){ m_parIds.reserve(np); m_visibleParticles.reserve(nvp); }
		void push_back(int Id, bool isVisible)
		{
			m_parIds.push_back(Id);
			if (isVisible) m_visibleParticles.push_back(Id);
		}

	private:
		std::vector<int> m_parIds;
		std::vector<int> m_visibleParticles;
	};

	typedef WR::HairParticle* HairSegment[2];

	class Hair
	{
	public:
		Hair(){}
		~Hair(){ release(); }

		void release();

		// add springs, add tetrahedrons, add segments
		bool init_simulation();
		void onFrame(Mat3 world, float fTime, float fTimeElapsed);

		void scale(float x);
		void mirror(bool, bool, bool);

		bool add_strand(float* positions, size_t n = N_PARTICLES_PER_STRAND);
		void reserve(size_t np, size_t ns) { m_strands.reserve(ns); }

		size_t n_strands() const{ return m_strands.size(); }
		const HairStrand& get_strand(size_t idx) const { return m_strands[idx]; }

		size_t n_particles() const{ return m_particles.size(); }
		const HairParticle& get_particle(size_t idx) const { return m_particles[idx]; }

		const float* get_visible_particle_position(size_t i, size_t j) const { return get_particle_position(get_strand(i).get_visible_particle(j)); }
		const float* get_particle_position(size_t i) const { return reinterpret_cast<const float*>(&m_position(3 * i)); }

	private:
		size_t add_particle(const vec3&, float mass_1, bool isPerturbed = false, bool isFixedPos = false);
		void add_particle(HairStrand& strand, const vec3&, float mass_1, bool isPerturbed = false, bool isFixedPos = false, bool isVisible = true);
		void init_matrices();
		void add_inner_springs();
		void push_springs(int idx);
		void push_single_spring(int idx, int stride);
		void step(const Mat3& mWorld, float fTime, float fTimeElapsed);

		template <class _M>
		void filter(_M& mat) const
		{

		}
		void solve(const SparseMat& A, const VecX& b, VecX& deltaV) const;

		std::vector<HairParticle>	m_particles;
		std::list<ISpring*>			m_springs;
		std::vector<HairStrand>		m_strands;
		std::vector<HairSegment>		m_segments;

		VecX							m_position;
		VecX							m_velocity;
		VecX							m_filter;
		SparseMat					m_mass_1;

		bool							mb_simInited = false;
	};
}