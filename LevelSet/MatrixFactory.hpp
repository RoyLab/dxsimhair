#pragma once
#include <Eigen\Sparse>
#include <linmath.h>
#include <deque>
#include <boost\log\trivial.hpp>
#include "HairStructs.h"
#include "EigenTypes.h"
#include "XTimer.hpp"

namespace XRwy
{
namespace Hair
{
	template <class Container>
	class MatrixFactory
	{
		typedef WR::SparseMat SparseMatrix;
		typedef WR::VecX Vector;
		typedef uint8_t gid_t;
		typedef Eigen::SimplicialLLT<SparseMatrix, Eigen::Upper, Eigen::NaturalOrdering<WR::SparseMat::Index>> LLTSolver;

		struct GroupCache
		{
			GroupCache() : LBase(20, 20), L(LBase.triangularView<Eigen::Lower>()) {}

			SparseMatrix A, LBase;
			Eigen::SparseTriangularView<SparseMatrix, Eigen::Lower> L;
			/// Eigen::SparseTriangularView<SparseMatrix, Eigen::Upper> LT; // no use???????

			Vector b, b0;
			float *buffer = 0;
			int buffersize = 0;
			///Eigen::SimplicialLLT<WR::SparseMat, Eigen::Upper> solver;
		};
	public:
		MatrixFactory(const char*, double k, int skipFactor=0);
		MatrixFactory(int* gids, int np, double k, int skipFactor=0);
		~MatrixFactory();

		void update(Container & id0, Container & id1, float* pos, uint32_t npos, double dr);
		float reportError(Container & id0, Container & id1, float* pos0, float* pos1, double dr);

	private:
		void initCache(double h);
		void computeb(int pass, float* pts, float dr);
		void updateL(Container & id0, Container & id1);
		bool isInit() const { return bInit; }
		void loadPosition(float* pts);
		void dispatchPosition(float* pts);
		void cachePosition();

		/** update b to x prime */
		void choleskySolve(int gid);

		/** check coherence of cache and pts */

		bool bInit = false;
		double k_;
		float balance;
		Container id0_, id1_;
		uint32_t nGroup;
		std::vector<gid_t> groupId; /** group id of each particle */
		std::vector<uint32_t>* groups = nullptr; /** particles in each group */
		std::vector<GroupCache> cache; /** A, L, b, buffer */
		std::vector<int> pid2matrixSeq; /** map from particle id to matrix position */
	};


	template<class Container>
	MatrixFactory<Container>::MatrixFactory(int* gids, int np, double k, int skipFactor)
	{
		k_ = k;

		groupId.resize(np);
		for (int i = 0; i < np; i++)
			groupId[i] = static_cast<gid_t>(gids[i]);

		auto minmaxpair = std::minmax_element(groupId.begin(), groupId.end());
		nGroup = *minmaxpair.second - *minmaxpair.first + 1;
		assert(nGroup > 0);

		/** initialize id, matrix map */
		// groupMatrixMapping = new std::deque<int>[nGroup];
		groups = new std::remove_reference<decltype(groups[0])>::type[nGroup];
		pid2matrixSeq.resize(np);
		int *counter = new int[nGroup];
		memset(counter, 0, sizeof(int) * nGroup);

		for (size_t i = 0; i < np; i++)
		{
			int gid = groupId[i];
			if (skipFactor < 1 || i % skipFactor != 0)
			{
				pid2matrixSeq[i] = 3 * counter[gid]++;
				groups[gid].push_back(i);
				//groupMatrixMapping[groupId].push_back(i);
			}
			else pid2matrixSeq[i] = -1;
		}

		assert([](int* arr, int n)->int {
			int count = 0;
			for (int i = 0; i < n; i++)
				count += arr[i];
			return count;
		}(counter, nGroup) * (skipFactor < 1 ? 1 : skipFactor) /
			(skipFactor < 1 ? 1 : skipFactor - 1) == np);

		/** alloc memory for cache */
		cache.resize(nGroup);
		for (uint32_t i = 0; i < nGroup; i++)
		{
			if (!counter[i]) continue;

			auto dim = counter[i] * 3;
			cache[i].A.resize(dim, dim);
			cache[i].b.resize(dim);
			cache[i].b0.resize(dim);

			cache[i].buffer = new float[dim];
			cache[i].buffersize = dim * sizeof(float);
		}

		delete[]counter;
	}


	template<class Container>
	MatrixFactory<Container>::MatrixFactory(const char* fgroup, double k, int skipFactor)
	{
		/** read *.group file */
		std::ifstream file(fgroup, std::ios::binary);
		assert(file.is_open());
		int ngi;
		Read4Bytes(file, ngi);

		int *gid = new int[ngi];
		ReadNBytes(file, gid, ngi * sizeof(4));
		file.close();

		new (this)MatrixFactory(gid, ngi, k, skipFactor);
	}

	template<class Container>
	float MatrixFactory<Container>::reportError(Container & id0, Container & id1, float* pos0, float* pos1, double dr)
	{
		const uint32_t npair = id0_.size();
		float error = 0.0f;
		const uint32_t np = groupId.size();

		for (uint32_t i = 0; i < np*3; i++)
			error += std::pow((pos0[i] - pos1[i]), 2);

		//float snap = error;
		//BOOST_LOG_TRIVIAL(debug) << "error a: " << error;

		for (uint32_t i = 0; i < npair; i++)
		{
			uint32_t id[2] = { id0_[i], id1_[i] };
			assert(id[0] < id[1]);

			int mseq[2] = { pid2matrixSeq[id[0]],  pid2matrixSeq[id[1]] };
			if (mseq[0] < 0 && mseq[1] < 0) continue;

			float* p0[2] = { pos0 + 3 * id[0], pos0 + 3 * id[1] };
			float* p1[2] = { pos1 + 3 * id[0], pos1 + 3 * id[1] };

			vec3 v10_0, v10_1;
			vec3_sub(v10_0, p0[0], p0[1]);
			vec3_sub(v10_1, p1[0], p1[1]);

			vec3_scale(v10_0, v10_0, dr / vec3_len(v10_0)); // from p1 to p0
			vec3_sub(v10_0, v10_0, v10_1); // from p1 to p0
			error += balance * vec3_mul_inner(v10_0, v10_0);
		}
		//BOOST_LOG_TRIVIAL(debug) << "error b: " << error-snap;

		return error;
	}


	template<class Container>
	MatrixFactory<Container>::~MatrixFactory()
	{
		SAFE_DELETE_ARRAY(groups);
	}

	template<class Container>
	void MatrixFactory<Container>::update(Container & id0, Container & id1, float* pos, uint32_t npos, double dr)
	{
#ifdef XRWY_PROFILE
		XTIMER_HELPER(setClock("mf"));
#endif

		assert(npos == groupId.size());
		assert(id0.size() == id1.size());

		if (!isInit())
		{
			bInit = true;

			id0_.swap(id0);
			id1_.swap(id1);

			/** h is given here explicitly, because prefactorization do not allow change on h */
			initCache(1);
		}
		else
		{
			updateL(id0, id1);
			id0_.swap(id0);
			id1_.swap(id1);
		}

		loadPosition(pos);

		float *de_pos = new float[npos * 3];
		memcpy(de_pos, pos, sizeof(float) * npos * 3);

#ifdef XRWY_DEBUG
		BOOST_LOG_TRIVIAL(debug) << "error 0: " << reportError(id0, id1, pos, de_pos, dr);
#endif

		const int maxiter = 4;
		for (int i = 0; i < maxiter; i++)
		{
			computeb(i, pos, dr);
			for (int j = 0; j < nGroup; j++)
			{
				if (groups[j].empty()) continue;
				choleskySolve(j);
			}
			cachePosition();
			dispatchPosition(de_pos);
#ifdef XRWY_PROFILE
			BOOST_LOG_TRIVIAL(debug) << XTIMER_HELPER(millisecondsAndReset("mf"));
#endif
#ifdef XRWY_DEBUG
			BOOST_LOG_TRIVIAL(debug) << "error "<< i+1 <<": " << reportError(id0, id1, pos, de_pos, dr);
#endif
		}
		dispatchPosition(pos);

		delete[] de_pos;
	}

	template<class Container>
	void MatrixFactory<Container>::loadPosition(float* pts)
	{
		for (uint32_t i = 0; i < nGroup; i++)
		{
			auto &pool = groups[i];
			for (uint32_t j = 0; j < pool.size(); j++)
				memcpy(cache[i].buffer + 3 * j, pts + 3 * pool[j], sizeof(float)*3);
		}
	}

	template<class Container>
	void MatrixFactory<Container>::dispatchPosition(float* pts)
	{
		for (uint32_t i = 0; i < nGroup; i++)
		{
			auto &pool = groups[i];
			for (uint32_t j = 0; j < pool.size(); j++)
				memcpy(pts + 3 * pool[j], cache[i].buffer + 3 * j, sizeof(float) * 3);
		}
	}

	template<class Container>
	void MatrixFactory<Container>::cachePosition()
	{
		for (uint32_t i = 0; i < nGroup; i++)
		{
			auto &infos = cache[i];
			memcpy(infos.buffer, infos.b.data(), infos.buffersize);
		}
	}


	template<class Container>
	void MatrixFactory<Container>::initCache(double h)
	{
		std::deque<Eigen::Triplet<float>> *assemble = new std::deque<Eigen::Triplet<float>>[nGroup];
		const uint32_t npair = id0_.size();
		balance = h*h*k_;

		for (uint32_t i = 0; i < npair; i++)
		{
			uint32_t id[2] = { id0_[i], id1_[i] };
			assert(id[0] < id[1]);

			int mseq[2] = { pid2matrixSeq[id[0]],  pid2matrixSeq[id[1]] };
			if (mseq[0] < 0 && mseq[1] < 0) continue;

			uint32_t g[2] = { groupId[id[0]], groupId[id[1]] };
			for (int j = 0; j < 2; j++)
			{
				if (mseq[j] < 0) continue;
				for (int k = 0; k < 3; k++)
					assemble[g[j]].push_back(Eigen::Triplet<float>(mseq[j] +k, mseq[j] +k, 1));
			}

			if (g[0] == g[1])
			{
				/** upper triangle, row < column */
				for (int k = 0; k < 3; k++)
					assemble[g[0]].push_back(Eigen::Triplet<float>(mseq[0] + k, mseq[1] + k, -1));
			}
		}

		LLTSolver solver;
		SparseMatrix tmpSM;
		for (uint32_t i = 0; i < nGroup; i++)
		{
			auto &infos = cache[i];
			tmpSM = infos.A;
			tmpSM.setFromTriplets(assemble[i].begin(), assemble[i].end());
			infos.A.setIdentity();
			infos.A += tmpSM * balance;

			solver.compute(infos.A);
			assert(solver.info() == Eigen::Success);

			infos.LBase = solver.matrixL();
		}
	}

	template<class Container>
	void MatrixFactory<Container>::computeb(int pass, float* pts, float dr)
	{
		const float coef = balance * dr;
		const uint32_t npair = id0_.size();

		/** compute b0 */
		if (pass == 0)
		{
			// clear b, b0, init b0 with buffer
			for (uint32_t i = 0; i < nGroup; i++)
			{
				auto &infos = cache[i];
				infos.b.setZero();
				memcpy(infos.b0.data(), infos.buffer, infos.buffersize);

				/** check whether b0 has the same content as pts */
				assert([](decltype(infos.b0) m, float *pts,
					std::remove_pointer<decltype(this->groups)>::type group)->bool {
					for (int i = 0; i < group.size(); i++)
					{
						for (int j = 0; j < 3; j++)
						{
							if (m[3 * i + j] != pts[3 * group[i] + j])
								return false;
						}
					}
					return true;
				}(infos.b0, pts, groups[i]));
			}

			for (uint32_t i = 0; i < npair; i++)
			{
				uint32_t id[2] = { id0_[i], id1_[i] };
				assert(id[0] < id[1]);
				uint32_t g[2] = { groupId[id[0]], groupId[id[1]] };

				int mseq[2] = { pid2matrixSeq[id[0]],  pid2matrixSeq[id[1]] };
				if (mseq[0] < 0 && mseq[1] < 0) continue;

				float* p[2] = { pts + 3 * id[0], pts + 3 * id[1] };
#ifdef _DEBUG
				float* p2[2] = { cache[g[0]].buffer + mseq[0], cache[g[1]].buffer + mseq[1] };
				assert(p[0][0] == p2[0][0] && p[0][1] == p2[0][1] && p[0][2] == p2[0][2]
					&& p[1][0] == p2[1][0] && p[1][1] == p2[1][1] && p[1][2] == p2[1][2]);
#endif

				vec3 v10;
				vec3_sub(v10, p[0], p[1]);
				vec3_scale(v10, v10, coef / vec3_len(v10)); // from p1 to p0

				if (mseq[0] >= 0)
				{
					for (int k = 0; k < 3; k++)
						cache[g[0]].b0[mseq[0] + k] += v10[k];
				}

				if (mseq[1] >= 0)
				{
					for (int k = 0; k < 3; k++)
						cache[g[1]].b0[mseq[1] + k] -= v10[k];
				}

				if (g[0] != g[1])
				{
					if (mseq[1] >= 0)
					{
						for (int k = 0; k < 3; k++)
							cache[g[1]].b[mseq[1] + k] += balance * p[0][k];
					}

					if (mseq[0] >= 0)
					{
						for (int k = 0; k < 3; k++)
							cache[g[0]].b[mseq[0] + k] += balance * p[1][k];
					}
				}
			}

		}
		else
		{
			for (uint32_t i = 0; i < nGroup; i++)
				cache[i].b.setZero();

			/** this is change over iter */
			for (uint32_t i = 0; i < npair; i++)
			{
				uint32_t id[2] = { id0_[i], id1_[i] };
				assert(id[0] < id[1]);

				int mseq[2] = { pid2matrixSeq[id[0]],  pid2matrixSeq[id[1]] };
				if (mseq[0] < 0 && mseq[1] < 0) continue;

				uint32_t g[2] = { groupId[id[0]], groupId[id[1]] };
				float* p[2] = { cache[g[0]].buffer + mseq[0], cache[g[1]].buffer + mseq[1] };

				if (g[0] != g[1])
				{
					if (mseq[1] >= 0)
					{
						for (int k = 0; k < 3; k++)
							cache[g[1]].b[mseq[1] + k] += balance * p[0][k];
					}

					if (mseq[0] >= 0)
					{
						for (int k = 0; k < 3; k++)
							cache[g[0]].b[mseq[0] + k] += balance * p[1][k];
					}
				}
			}
		}


		for (uint32_t i = 0; i < nGroup; i++)
		{
			auto &infos = cache[i];
			infos.b += infos.b0;
		}
	}

	template<class Container>
	void MatrixFactory<Container>::updateL(Container & id0, Container & id1)
	{
		// find difference

		// update each L
	}


	template<class Container>
	void MatrixFactory<Container>::choleskySolve(int gid)
	{
		GroupCache &infos = cache[gid];

		infos.L.solveInPlace(infos.b);
		infos.LBase.transpose().triangularView<Eigen::Upper>().solveInPlace(infos.b);
	}


} /// Hair
} /// XRwy
