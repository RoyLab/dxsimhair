#pragma once

#include "macros.h"
#include "linmath.h"
#include "EigenTypes.h"
#include "wrConstants.h"
#include "ICollisionObject.h"
#include "Collider.h"

namespace WR
{
	class Hair;
	class ISpring;
	class StrainLimitPair;

	Hair *loadFile(const char*, const XRwy::Collider *collider, bool use_scale, float scale_x, float scale_y, float scale_z);

	class HairParticle
	{
		COMMON_PROPERTY(Vec3, ref);
		STATIC_PROPERTY(Hair*, hair);

		friend class Hair;
	public:
		HairParticle(const Vec3& pos, size_t id, size_t localId, bool isPerturbed = false, bool isFixedPos = false) :
			m_ref(pos), m_Id(id), m_LocalId(localId),
			mb_perturbed(isPerturbed), mb_fixedPos(isFixedPos) {}

		size_t get_Id() const { return m_Id; }
		size_t get_LocalId() const { return m_LocalId; }
		bool isPerturbed() const { return mb_perturbed; }
		bool isFixedPos() const { return mb_fixedPos; }
		const Vec3 get_pos() const;

		float getMass() { return *mass;  }
		float getMass_1() { return 1.0f / (*mass); }
		void setMassPointer(float* massPointer) {
			mass = massPointer;
		}

	private:
		size_t m_Id;
		size_t m_LocalId; //the local id in strand
		float *mass;

		// mark only the split nodes, NOT the root ones.
		bool            mb_perturbed;
		bool            mb_fixedPos;
	};

	class HairStrand
	{
		STATIC_PROPERTY(Hair*, hair);

		friend class Hair;
	public:
		int get_visible_particle(int idx) const { return m_visibleParticles[idx]; }
		int get_particle(int idx) const
		{
			if (idx >= 0) return m_parIds[idx];
			else return m_parIds[m_parIds.size() + idx];
		}

		void reserve(size_t np, size_t nvp = N_PARTICLES_PER_STRAND) { m_parIds.reserve(np); m_visibleParticles.reserve(nvp); }
		void push_back(int Id, bool isVisible)
		{
			m_parIds.push_back(Id);
			if (isVisible) m_visibleParticles.push_back(Id);
		}

	private:
		std::vector<int> m_parIds;
		std::vector<int> m_visibleParticles;
		std::vector<ISpring*> m_springPointers;
	};

	typedef WR::HairParticle* HairSegment[2];

	class Hair
	{
	public:
		Hair() {}
		~Hair() { release(); }

		void release();

		// add springs, add tetrahedrons, add segments
		bool init_simulation();
		void onFrame(Mat3 world, float fTime, float fTimeElapsed);

		void scale(float x);
		void mirror(bool, bool, bool);

		bool add_strand(float* positions, const XRwy::Collider *collider, size_t n = N_PARTICLES_PER_STRAND);
		void reserve(size_t np, size_t ns) { m_strands.reserve(ns); m_particles.reserve(np); }

		size_t n_strands() const { return m_strands.size(); }
		const HairStrand& get_strand(size_t idx) const { return m_strands[idx]; }

		size_t n_particles() const { return m_particles.size(); }
		const HairParticle& get_particle(size_t idx) const { return m_particles[idx]; }

		const float* get_visible_particle_position(size_t i, size_t j) const { return get_particle_position(get_strand(i).get_visible_particle(j)); }
		const float* get_particle_position(size_t i) const { return reinterpret_cast<const float*>(&m_position(3 * i)); }

		void step(const Mat4& mWorld, float fTime, float fTimeElapsed, const XRwy::Collider *collider, const float mWrold2Collision[16]);

	private:
		//size_t add_particle(const vec3&, float *mass, bool isPerturbed = false, bool isFixedPos = false);
		void add_particle(HairStrand& strand, const vec3&, float *mass, bool isPerturbed = false, bool isFixedPos = false, bool isVisible = true);
		void init_matrices();
		void add_inner_springs();
		void push_springs(HairStrand &strand, int idx);
		void push_single_spring(HairStrand &strand, int idx, int stride);

		// 保证了从发根到发梢的次序！！非常重要
		void add_strain_limits();

		void resolve_strain_limits(VecX& pos, VecX& vel, float t) const;
		void resolve_body_collision(const Mat3& mWorld, VecX& pos, VecX& vel, float t) const;

		template <class _M1, class _M2>
		void filter(const _M1& vec, _M2& res) const { res = m_filter.cwiseProduct(vec); }
		void modified_pcg(const SparseMat& A, const VecX& b, VecX& dv) const;
		void LU(const SparseMat& A, const VecX& b, VecX& dv) const;
		void simple_solve(const MatX& A, const VecX& b, VecX& dv) const;
		VecX extract_dv(const HairStrand &strand, const float fTimeElapsed) const;

		std::vector<HairParticle>       m_particles;
		std::list<ISpring*>             m_springs;
		std::vector<HairStrand>         m_strands;
		std::vector<HairSegment>        m_segments;
		std::list<StrainLimitPair*>     m_strain_limits;

		VecX                            m_position;
		VecX                            m_velocity;
		VecX                            m_filter, m_gravity;
		SparseMat                       m_mass_1, m_mass, m_wind_damping;

		bool                            mb_simInited = false;
	};

	inline const Vec3 HairParticle::get_pos() const
	{
		return Vec3(m_hair->get_particle_position(m_Id));
	}
}