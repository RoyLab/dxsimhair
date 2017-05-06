#pragma once
#include <Eigen\Sparse>
#include "xmath.h"
#include <iostream>
#include <vector>
#include <set>
#include <boost\log\trivial.hpp>
#include "HairStructs.h"
#include "EigenTypes.h"
#include "XTimer.hpp"
#include "XSparseMatrix.hpp"
#include "SparseCholeskyUpdate.hpp"

namespace XRwy
{
namespace Hair
{
	template <class Container>
	class MatrixFactory
	{
		typedef WR::SparseMat SparseMatrix;
		typedef XR::SPDLowerMatrix<float> XSparseMatrix;
		typedef WR::VecX Vector;
		typedef uint8_t gid_t;
		typedef Eigen::SimplicialLLT<SparseMatrix, Eigen::Upper, Eigen::NaturalOrdering<WR::SparseMat::Index>> LLTSolver;

		struct GroupCache
		{
			GroupCache() {}

			SparseMatrix A;
			XSparseMatrix LBase;
			//Eigen::SparseTriangularView<SparseMatrix, Eigen::Lower> L;
			/// Eigen::SparseTriangularView<SparseMatrix, Eigen::Upper> LT; // no use???????

			Vector b, b0;
			float *buffer = 0;
			int buffersize = 0;
			///Eigen::SimplicialLLT<WR::SparseMat, Eigen::Upper> solver;
		};
	public:
		MatrixFactory(const char*, double k, int skipFactor = 0);
		MatrixFactory(const int* gids, int np, double k, int skipFactor = 0);
		~MatrixFactory();

		void update(Container & id0, Container & id1, Container & id0p, Container & id1p, Container & id0n, Container & id1n, float* pos, uint32_t npos, double dr);
		float reportError(Container & id0, Container & id1, float* pos0, float* pos1, double dr);

	private:
		void recomputeA(double h, Container & id0, Container & id1);
		void computeb(Container & id0, Container & id1, int pass, float* pts, float dr);
		void updateL(Container & id0, Container & id1);
		void updateL(Container & id0p, Container & id1p, Container & id0n, Container & id1n);
		
		bool isInit() const { return bInit; }
		void loadPosition(float* pts); // load from pts into cache buffer
		void dispatchPosition(float* pts); // collect from cache buffer
		void cachePosition(); // load from b to cache
		void _update(SparseMatrix& m, int id, int id2 = -1);
		void _update(XSparseMatrix& m, int id, int id2 = -1);
		void _downdate(GroupCache& cache, int id, int id2 = -1);

		/** update b to x prime */
		void choleskySolve(int gid);

		/** check coherence of cache and pts */

		bool bInit = false;
		double k_;
		float balance, sqrtBalance;
		uint32_t nGroup;
		std::vector<gid_t> groupId; /** group id of each particle */
		std::vector<uint32_t>* groups = nullptr; /** particles in each group */
		std::vector<GroupCache> cache; /** A, L, b, buffer */
		std::vector<int> pid2matrixSeq; /** map from particle id to matrix position */
	};

	template<class Container>
	MatrixFactory<Container>::MatrixFactory(const int* gids, int np, double k, int skipFactor)
	{
		k_ = k;

		groupId.resize(np);
		for (int i = 0; i < np; i++)
			groupId[i] = static_cast<gid_t>(gids[i / skipFactor]);

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
				pid2matrixSeq[i] = counter[gid]++;
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

			auto dim = counter[i];
			auto dimx3 = dim * 3;
			cache[i].A.resize(dim, dim);
			cache[i].b.resize(dimx3);
			cache[i].b0.resize(dimx3);

			cache[i].buffer = new float[dimx3];
			cache[i].buffersize = dimx3 * sizeof(float);
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
		const uint32_t npair = id0.size();
		float error = 0.0f;
		const uint32_t np = groupId.size();

		for (uint32_t i = 0; i < np * 3; i++)
			error += std::pow((pos0[i] - pos1[i]), 2);

		//float snap = error;
		//BOOST_LOG_TRIVIAL(debug) << "error a: " << error;

		//std::cout << "error:\n" << PRINT_TRIPLE(pos1+3 * 251492) << std::endl;
		//std::cout << PRINT_TRIPLE(pos1+3 * 251467) << std::endl;
		//std::cout  << PRINT_TRIPLE(pos1+3 * 254067) << std::endl;
		//std::cout  << PRINT_TRIPLE(pos1+3* 248817) << std::endl;

		for (uint32_t i = 0; i < npair; i++)
		{
			uint32_t id[2] = { id0[i], id1[i] };
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

			//if (std::isinf(dr / vec3_len(v10_0)))
			//	std::cout << PRINT_TRIPLE(v10_0) << std::endl;

			//if (std::isinf(vec3_len(v10_1)))
			//{
			//	std::cout << "error:\n" << PRINT_TRIPLE(p1[0]) << std::endl;
			//	std::cout << "error:\n" << PRINT_TRIPLE(p1[1]) << std::endl;
			//	std::cout << "id:\n" << id[0] << '\t' << id[1] << std::endl;
			//}
		}
		//BOOST_LOG_TRIVIAL(debug) << "error b: " << error-snap;

		return error;
	}

//#define XRWY_DEBUG
	template<class Container>
	MatrixFactory<Container>::~MatrixFactory()
	{
		SAFE_DELETE_ARRAY(groups);
	}

	template<class Container>
	void MatrixFactory<Container>::update(Container & id0, Container & id1, Container & id0p, Container & id1p,
		Container & id0n, Container & id1n, float* pos, uint32_t npos, double dr)
	{
#ifdef XRWY_PROFILE
		XTIMER_HELPER(setClock("mf"));
#endif
		assert(npos == groupId.size());
		assert(id0p.size() == id1p.size());
		assert(id0n.size() == id1n.size());

		if (!isInit())
		{
			bInit = true;

			/** h is given explicitly */
			recomputeA(1, id0, id1);
		}
		else
		{
			//recomputeA(1, id0, id1);
			updateL(id0p, id1p, id0n, id1n);
			//updateL(id0, id1);
		}

		//for (int i = 0; i < nGroup; i++)
		//	if (std::isnan(cache[i].LBase.sum()))
		//		BOOST_LOG_TRIVIAL(error) << i;

		//BOOST_LOG_TRIVIAL(info) << "\tupdate cholesky: " << XTIMER_HELPER(milliseconds("solve"));

		XTIMER_HELPER(setClock("ap"));

		loadPosition(pos);

		float *de_pos = new float[npos * 3];
		memcpy(de_pos, pos, sizeof(float) * npos * 3);

#ifdef XRWY_DEBUG
		BOOST_LOG_TRIVIAL(info) << "error 0: " << reportError(id0, id1, pos, de_pos, dr);
#endif

		const int maxiter = 4;
		for (int i = 0; i < maxiter; i++)
		{
			computeb(id0, id1, i, de_pos, dr);

			for (int i = 0; i < nGroup; i++)
				if (std::isnan(cache[i].b.sum()))
					BOOST_LOG_TRIVIAL(warning) << i;

			for (int j = 0; j < nGroup; j++)
			{
				if (groups[j].empty()) continue;
				GroupCache &infos = cache[j];

				//if (std::isinf(infos.b.squaredNorm()) || infos.b.squaredNorm() > 1E10)
				//	std::cout << "!!!!!!2333!" << j << std::endl;

				infos.LBase.forwardSubstitutionWithPrunex3(infos.b, 1e-4f);
				infos.LBase.backwardSubstitutionx3(infos.b);

				//if (std::isinf(infos.b.squaredNorm()) || infos.b.squaredNorm() > 1E10)
				//	std::cout << "!!!!!!!" << j << std::endl;

				//infos.LBase.triangularView<Eigen::Lower>().solveInPlace(infos.b);
				//infos.LBase.transpose().triangularView<Eigen::Upper>().solveInPlace(infos.b);
			}
			cachePosition();
			dispatchPosition(de_pos);
#ifdef XRWY_PROFILE
			BOOST_LOG_TRIVIAL(debug) << XTIMER_HELPER(millisecondsAndReset("mf"));
#endif
#ifdef XRWY_DEBUG
			BOOST_LOG_TRIVIAL(info) << "error " << i + 1 << ": " << reportError(id0, id1, pos, de_pos, dr);
#endif
		}
		dispatchPosition(pos);

		delete[] de_pos;

		BOOST_LOG_TRIVIAL(info) << "\tsolve: " << XTIMER_HELPER(milliseconds("ap"));
	}

	template<class Container>
	void MatrixFactory<Container>::loadPosition(float* pts)
	{
		for (uint32_t i = 0; i < nGroup; i++)
		{
			auto &pool = groups[i];
			for (uint32_t j = 0; j < pool.size(); j++)
				memcpy(cache[i].buffer + 3 * j, pts + 3 * pool[j], sizeof(float) * 3);
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

			//for (int j = 0; j < infos.buffersize/sizeof(float); j++)
			//	if (std::isinf(infos.buffer[j]) || std::abs(infos.buffer[j]) > 2000)
			//		std::cout << "fasfadasdfadsf:  " << infos.buffer[j] << '\t' << i << j;
		}
	}


	template<class Container>
	void MatrixFactory<Container>::recomputeA(double h, Container & id0, Container & id1)
	{
		std::deque<Eigen::Triplet<float>> *assemble = new std::deque<Eigen::Triplet<float>>[nGroup];
		const uint32_t npair = id0.size();
		balance = h*h*k_; sqrtBalance = std::sqrt(balance);

		for (uint32_t i = 0; i < npair; i++)
		{
			uint32_t id[2] = { id0[i], id1[i] };
			assert(id[0] < id[1]);

			int mseq[2] = { pid2matrixSeq[id[0]],  pid2matrixSeq[id[1]] };
			if (mseq[0] < 0 && mseq[1] < 0) continue;

			uint32_t g[2] = { groupId[id[0]], groupId[id[1]] };
			for (int j = 0; j < 2; j++)
			{
				if (mseq[j] < 0) continue;
				//for (int k = 0; k < 3; k++)
				assemble[g[j]].push_back(Eigen::Triplet<float>(mseq[j], mseq[j], 1));
			}

			if (g[0] == g[1])
			{
				/** upper triangle, row < column */
				//for (int k = 0; k < 3; k++)
				assemble[g[0]].push_back(Eigen::Triplet<float>(mseq[0], mseq[1], -1));
			}
		}

		LLTSolver solver;
		SparseMatrix tmpSM;


		for (uint32_t i = 0; i < nGroup; i++)
		{
			auto &infos = cache[i];
			if (infos.buffersize == 0) continue;
			tmpSM.resize(infos.A.rows(), infos.A.cols());
			tmpSM.setFromTriplets(assemble[i].begin(), assemble[i].end());
			infos.A.setIdentity();
			infos.A += tmpSM * balance;

			solver.compute(infos.A);
			assert(solver.info() == Eigen::Success);

			infos.LBase = solver.matrixL();
			//for (int j = 0; j < infos.LBase.cols(); j++)
			//{
			//	for (auto itr = infos.LBase.getIterator(j); itr; itr++)
			//	{
			//		if (itr.index() != j)
			//			cout << itr.index() << '\t' << j << '\t' << itr.value() << std::endl;
			//	}
			//}

			BOOST_LOG_TRIVIAL(debug) << "nnz: " << infos.LBase.nonZeros() << " size: " << infos.LBase.rows() <<
				"\tratio: " << infos.LBase.nonZeros() / (float)infos.LBase.rows();
		}
	}

	template<class Container>
	void MatrixFactory<Container>::computeb(Container & id0, Container & id1, int pass, float* pts, float dr)
	{
		const float coef = balance * dr;
		const uint32_t npair = id0.size();

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
				uint32_t id[2] = { id0[i], id1[i] };
				assert(id[0] < id[1]);
				uint32_t g[2] = { groupId[id[0]], groupId[id[1]] };

				int mseq[2] = { 3 * pid2matrixSeq[id[0]],  3 * pid2matrixSeq[id[1]] };
				if (mseq[0] < 0 && mseq[1] < 0) continue;

				float* p[2] = { pts + 3 * id[0], pts + 3 * id[1] };

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
				uint32_t id[2] = { id0[i], id1[i] };
				assert(id[0] < id[1]);

				int mseq[2] = { 3 * pid2matrixSeq[id[0]],  3 * pid2matrixSeq[id[1]] };
				if (mseq[0] < 0 && mseq[1] < 0) continue;

				uint32_t g[2] = { groupId[id[0]], groupId[id[1]] };
				//float* p[2] = { cache[g[0]].buffer + mseq[0], cache[g[1]].buffer + mseq[1] };
				float* p[2] = { pts + 3 * id[0], pts + 3 * id[1] };

				if (g[0] != g[1])
				{
					if (mseq[1] >= 0)
					{
						for (int k = 0; k < 3; k++)
						{
							cache[g[1]].b[mseq[1] + k] += balance * p[0][k];
							//if (std::isinf(cache[g[1]].b[mseq[1] + k]) || std::abs(cache[g[1]].b[mseq[1] + k]) > 2000)
							//{
							//	std::cout << PRINT_TRIPLE(p[0]) << std::endl;
							//	std::cout << "???????" << p[0][k] << "\t" << mseq[0] + k << '\t' << g[0] << '\t' << cache[g[0]].buffersize / 4;
							//}
						}
					}

					if (mseq[0] >= 0)
					{
						for (int k = 0; k < 3; k++)
						{
							cache[g[0]].b[mseq[0] + k] += balance * p[1][k];
							//if (std::isinf(cache[g[0]].b[mseq[0] + k]) || std::abs(cache[g[0]].b[mseq[0] + k]) > 2000)
							//	std::cout << "???????" << p[1][k];
						}
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

	static bool order(uint32_t a[2], uint32_t b[2])
	{
		assert(a[0] < a[1] && b[0] < b[1]);
		if (a[0] < b[0]) return true;
		else
		{
			if (a[0] == b[0]) return a[1] < b[1];
			return false;
		}
	}

	static bool equal(uint32_t a[2], uint32_t b[2])
	{
		assert(a[0] < a[1] && b[0] < b[1]);
		return (a[0] == b[0]) && (a[1] == b[1]);
	}


	static int _comp_1(uint32_t a[2], uint32_t b[2])
	{
		if (a[0] < b[0]) return 1;
		if (a[0] == b[0])
		{
			if (a[1] < b[1]) return 1;
			if (a[1] == b[1]) return 0;
		}
		return -1;
	}

	template<class Container>
	void MatrixFactory<Container>::_update(SparseMatrix& m, int id, int id2)
	{
		SparseVector v(m.rows());
		v.setZero();
		v.coeffRef(id) = sqrtBalance;
		if (id2 > 0)
			v.coeffRef(id2) = -sqrtBalance;
		sparse_cholesky_update(m, v);
	}

	template<class Container>
	void MatrixFactory<Container>::_update(XSparseMatrix& m, int id, int id2)
	{
		Eigen::SparseVector<float> v(m.rows());
		if (id2 > 0)
		{
			v.setZero();
			v.coeffRef(id) = sqrtBalance;
			v.coeffRef(id2) = -sqrtBalance;
			sparse_cholesky_update(m, v);
		}
		else
		{
			v.setZero();
			v.coeffRef(id) = sqrtBalance;
			sparse_cholesky_update(m, v);
		}

	}

	template<class Container>
	void MatrixFactory<Container>::_downdate(GroupCache& cache, int id, int id2)
	{
		Eigen::SparseVector<float> v(cache.b.rows());

		if (id2 > 0)
		{
			v.setZero();
			v.coeffRef(id) = sqrtBalance;
			v.coeffRef(id2) = -sqrtBalance;
			sparse_cholesky_downdate(cache.LBase, v);
		}
		else
		{
			v.setZero();
			v.coeffRef(id) = sqrtBalance;
			sparse_cholesky_downdate(cache.LBase, v);
		}
	}

	template<class Container>
	void MatrixFactory<Container>::updateL(Container & id0, Container & id1)
	{
		const size_t sz = id0.size();
		size_t i = 0, i0 = 0, idx[2] = { id0[0], id1[0] }, idx0[2] = { id0_[0], id1_[0] };
		int mseq[2];
		uint32_t g[2];

		int counta = 0, countb = 0;

		while (i < id0.size() || i0 < id0_.size())
		{
			if (i < id0.size())
			{
				idx[0] = id0[i];
				idx[1] = id1[i];
			}

			if (i0 < id0_.size())
			{
				idx0[0] = id0_[i0];
				idx0[1] = id1_[i0];
			}

			int res;
			if (i == id0.size())
				res = -1;
			else if (i0 == id0_.size())
				res = 1;
			else
				res = _comp_1(idx, idx0);

			switch (res)
			{
			case 1: // update
				mseq[0] = pid2matrixSeq[idx[0]]; mseq[1] = pid2matrixSeq[idx[1]];
				g[0] = groupId[idx[0]]; g[1] = groupId[idx[1]];
				if (g[0] == g[1] && mseq[0] != -1 && mseq[1] != -1)
				{
					_update(cache[g[0]].LBase, mseq[0], mseq[1]);
				}
				else
				{
					if (mseq[0] != -1)
						_update(cache[g[0]].LBase, mseq[0]);
					if (mseq[1] != -1)
						_update(cache[g[1]].LBase, mseq[1]);
				}
				i++; counta++;
				break;
			case -1: // downdate
				mseq[0] = pid2matrixSeq[idx0[0]]; mseq[1] = pid2matrixSeq[idx0[1]];
				g[0] = groupId[idx0[0]]; g[1] = groupId[idx0[1]];
				if (g[0] == g[1] && mseq[0] != -1 && mseq[1] != -1)
				{
					_downdate(cache[g[0]], mseq[0], mseq[1]);
				}
				else
				{
					if (mseq[0] != -1)
						_downdate(cache[g[0]], mseq[0]);
					if (mseq[1] != -1)
						_downdate(cache[g[1]], mseq[1]);
				}
				i0++; countb++;
				break;
			default: // skip
				assert(res == 0);
				i++; i0++;
				break;
			}
		}

		int countc = 0;
		for (uint32_t i = 0; i < nGroup; i++)
		{
			countc += cache[i].LBase.nonZeros();
		}
		BOOST_LOG_TRIVIAL(info) << "Number of update " << counta << '/' << countb << '/' << id0.size() << '\t' << countc;

		id0_.swap(id0);
		id1_.swap(id1);
	}

	template<class Container>
	void MatrixFactory<Container>::updateL(Container & id0p, Container & id1p, Container & id0n, Container & id1n)
	{
		size_t idx[2];
		int mseq[2];
		uint32_t g[2];

		size_t n = id0p.size();
		for (int i = 0; i < n; i++)
		{
			idx[0] = id0p[i];
			idx[1] = id1p[i];

			mseq[0] = pid2matrixSeq[idx[0]];
			mseq[1] = pid2matrixSeq[idx[1]];

			g[0] = groupId[idx[0]];
			g[1] = groupId[idx[1]];

			if (g[0] == g[1] && mseq[0] != -1 && mseq[1] != -1)
			{
				_update(cache[g[0]].LBase, mseq[0], mseq[1]);
			}
			else
			{
				if (mseq[0] != -1)
					_update(cache[g[0]].LBase, mseq[0]);
				if (mseq[1] != -1)
					_update(cache[g[1]].LBase, mseq[1]);
			}
		}

		n = id0n.size();
		for (int i = 0; i < n; i++)
		{
			idx[0] = id0n[i];
			idx[1] = id1n[i];

			mseq[0] = pid2matrixSeq[idx[0]];
			mseq[1] = pid2matrixSeq[idx[1]];

			g[0] = groupId[idx[0]];
			g[1] = groupId[idx[1]];

			if (g[0] == g[1] && mseq[0] != -1 && mseq[1] != -1)
			{
				_downdate(cache[g[0]], mseq[0], mseq[1]);
			}
			else
			{
				if (mseq[0] != -1)
					_downdate(cache[g[0]], mseq[0]);
				if (mseq[1] != -1)
					_downdate(cache[g[1]], mseq[1]);
			}
		}
	}


	template<class Container>
	void MatrixFactory<Container>::choleskySolve(int gid)
	{
		GroupCache &infos = cache[gid];

		//infos.LBase.forwardSubstitution(infos.b);
		//infos.LBase.backwardSubstitution(infos.b);

		infos.LBase.triangularView<Eigen::Lower>().solveInPlace(infos.b);
		infos.LBase.transpose().triangularView<Eigen::Upper>().solveInPlace(infos.b);
	}


} /// Hair
} /// XRwy
