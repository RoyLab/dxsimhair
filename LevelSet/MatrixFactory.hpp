#pragma once
#include <Eigen\Sparse>
#include "HairStructs.h"
#include "EigenTypes.h"

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

		struct GroupCache
		{
			SparseMatrix L, A;
			Vector b, b0;
			float *buffer = 0;
			int buffersize = 0;
			///Eigen::SimplicialLLT<WR::SparseMat, Eigen::Upper> solver;
		};
	public:
		MatrixFactory(const char*, double k=1, int skipFactor=0);
		~MatrixFactory();

		void update(Container & id0, Container & id1, float* &pos, uint32_t npos, double dr, double h);

	private:
		void initCache(double h);
		void compute_b(int pass, int gId, float* pts, float dr);
		void updateL(Container & id0, Container & id1);
		bool isInit() const { return bInit; }
		void loadPosition(float*& pts);
		void dispatchPosition(float*& pts);

		bool bInit = false;
		double k_;
		Container id0_, id1_;
		uint32_t nGroup;
		std::vector<gid_t> groupId; // group id of each particle
		std::vector<uint32_t>* groups = nullptr; // particles in each group
		std::vector<GroupCache> cache; // A, L, b, buffer
		std::vector<uint32_t> pid2matrixSeq; // from particle id to matrix position
	};

	template<class Container>
	MatrixFactory<Container>::MatrixFactory(const char* fgroup, double k, int skipFactor)
	{
		k_ = k;

		std::ifstream file(fgroup, std::ios::binary);
		int ngi;
		Read4Bytes(file, ngi);

		int *gid = new int[ngi];
		ReadNBytes(file, gid, ngi * sizeof(4));
		file.close();

		groupId.resize(ngi);
		for (int i = 0; i < ngi; i++)
			groupId[i] = static_cast<gid_t>(gid[i]);

		auto minmaxpair = std::minmax_element(groupId.begin(), groupId.end());
		nGroup = *minmaxpair.second - *minmaxpair.first + 1;

		//groupMatrixMapping = new std::deque<int>[nGroup];
		groups = new std::remove_reference<decltype(groups[0])>::type[nGroup];
		pid2matrixSeq.resize(ngi);
		int *counter = new int[nGroup];
		memset(counter, 0, sizeof(int) * nGroup);

		for (size_t i = 0; i < ngi; i++)
		{
			int gid = groupId[i];
			if (skipFactor < 1 || i % skipFactor != 0)
			{
				pid2matrixSeq[i] = 3*counter[gid]++;
				groups[gid].push_back(i);
				//groupMatrixMapping[groupId].push_back(i);
			}
			else pid2matrixSeq[i] = -1;
		}

		cache.resize(nGroup);
		for (uint32_t i = 0; i < nGroup; i++)
		{
			auto dim = counter[i] * 3;
			cache[i].A.resize(dim, dim);
		}

		delete[]counter;
	}

	template<class Container>
	MatrixFactory<Container>::~MatrixFactory()
	{
		SAFE_DELETE_ARRAY(groups);
	}

	template<class Container>
	void MatrixFactory<Container>::update(Container & id0, Container & id1, float* &pos, uint32_t npos, double dr, double h)
	{
		assert(npos == groupId.size());
		if (!isInit())
		{
			bInit = true;

			id0_.swap(id0);
			id1_.swap(id1);
			initCache(h);
		}
		else
		{
			updateL(id0, id1);
			id0_.swap(id0);
			id1_.swap(id1);
		}

		loadPosition(pos);

		const int maxiter = 4;
		for (int i = 0; i < maxiter; i++)
		{
			for (int j = 0; j < nGroup; j++)
			{
				if (groups[j].empty()) continue;

				compute_b(i, j, pos, dr);
				//choleskySolve(j);
			}
		}
		dispatchPosition(pos);
	}

	template<class Container>
	void MatrixFactory<Container>::loadPosition(float*& pts)
	{

	}

	template<class Container>
	void MatrixFactory<Container>::dispatchPosition(float*& pts)
	{

	}

	template<class Container>
	void MatrixFactory<Container>::initCache(double h)
	{
		std::deque<Eigen::Triplet<float>> *assemble = new std::deque<Eigen::Triplet<float>>[nGroup];
		const uint32_t npair = id0_.size();
		const float balance = h*h*k_;

		for (uint32_t i = 0; i < npair; i++)
		{
			uint32_t id[2] = { id0_[i], id1_[i] };
			assert(id[0] < id[1]);
			uint32_t g[2] = { groupId[id[0]], groupId[id[1]] };
			uint32_t mseq[2] = { pid2matrixSeq[id[0]],  pid2matrixSeq[id[1]] };

			// assemble A
			for (int j = 0; j < 2; j++)
			{
				if (mseq[j] < 0) continue;
				for (int k = 0; k < 3; k++)
					assemble[g[j]].push_back(Eigen::Triplet<float>(mseq[j] +k, mseq[j] +k, 1));
			}

			if (g[0] == g[1])
			{
				// upper triangle, row < column
				for (int k = 0; k < 3; k++)
					assemble[g[0]].push_back(Eigen::Triplet<float>(mseq[0] + k, mseq[1] + k, -1));
			}
		}

		Eigen::SimplicialLLT<SparseMatrix, Eigen::Upper> solver;
		SparseMatrix tmpSM;
		for (uint32_t i = 0; i < nGroup; i++)
		{
			const int dim = cache[i].A.rows();
			tmpSM.resize(dim, dim);
			tmpSM.setFromTriplets(assemble[i].begin(), assemble[i].end());
			cache[i].A.setIdentity();
			cache[i].A += tmpSM * balance;

			solver.compute(cache[i].A);
			cache[i].L = solver.matrixL();
			cache[i].b.resize(dim);
			cache[i].b0.resize(dim);

			if (groups[i].size())
			{
				cache[i].buffer = new float[3 * groups[i].size()];
				cache[i].buffersize = 3 * groups[i].size() * sizeof(float);
			}
		}
	}

	template<class Container>
	void MatrixFactory<Container>::compute_b(int pass, int gId, float* pts, float dr)
	{
		auto& particleList = groups[gId];
		auto& infos = cache[gId];
		//auto n = particleList.size();
		//const float balance = this->balance;

		memcpy(infos.b0.data(), infos.buffer, infos.buffersize);

		//for (size_t i = 0; i < n; i++)
		//{
		//	auto& point = pts[particleList[i]];
		//	b[3 * i] = point.x;
		//	b[3 * i + 1] = point.y;
		//	b[3 * i + 2] = point.z;
		//}

		//for (auto& item : infos.l)
		//{
		//	auto &pi = pts[item.i], &pj = pts[item.number];

		//	KernelPBD::Point_3 p0(pi.x, pi.y, pi.z);
		//	KernelPBD::Point_3 p1(pj.x, pj.y, pj.z);
		//	int id0 = matrixSeq[item.i];
		//	int id1 = matrixSeq[item.number];

		//	auto diff = (p0 - p1);
		//	auto v3 = balance * dr * diff / sqrt(diff.squared_length());

		//	b[3 * id0] += v3.x();
		//	b[3 * id0 + 1] += v3.y();
		//	b[3 * id0 + 2] += v3.z();

		//	b[3 * id1] -= v3.x();
		//	b[3 * id1 + 1] -= v3.y();
		//	b[3 * id1 + 2] -= v3.z();
		//}

		//for (auto& item : infos.l2)
		//{
		//	auto &pi = pts[item.i], &pj = pts[item.number];

		//	KernelPBD::Point_3 p0(pi.x, pi.y, pi.z);
		//	KernelPBD::Point_3 p1(pj.x, pj.y, pj.z);
		//	int id1 = matrixSeq[item.number];

		//	auto diff = (p0 - p1);
		//	auto v3 = balance * dr * diff / sqrt(diff.squared_length());

		//	b[3 * id1] -= (v3.x() - balance * pi.x);
		//	b[3 * id1 + 1] -= (v3.y() - balance * pi.y);
		//	b[3 * id1 + 2] -= (v3.z() - balance * pi.z);
		//}
	}

	template<class Container>
	void MatrixFactory<Container>::updateL(Container & id0, Container & id1)
	{

	}


	//void GroupPBD::solveSingleGroup(int gId, const Tree* pTree, XMFLOAT3* p0, WR::VecX& x, int pps, bool pass0)
	//{
	//	Eigen::SimplicialLLT<WR::SparseMat, Eigen::Upper>* pSolver = nullptr;
	//	size_t dim = 3 * groupIds[gId].size();
	//	if (!bMatrixInited)
	//	{
	//		void(0);
	//	}
	//	auto &solverCache = solverCacheBuffer[gId];

	//	if (pass0)
	//	{
	//		auto& particleList = groupIds[gId];
	//		std::list<IntPair> l, l2;

	//		for (size_t i = 0; i < particleList.size(); i++)
	//		{
	//			int particleId = particleList[i]; //global id
	//			if (particleId % pps == 0) continue; // do not check follicle

	//			std::list<IntWrapper> tmps;
	//			auto &v = p0[particleId];
	//			Fuzzy_sphere fs(Point_3(v.x, v.y, v.z, particleId), dr, 0.5f * dr);
	//			pTree->search(std::back_inserter(tmps), fs);
	//			for (auto item : tmps)
	//			{
	//				if (item.i % pps != 0 && belongTo(item.i, gId)) // not follicle and belong to group
	//				{
	//					if (item.i < particleId)
	//						l.emplace_back(item.i, particleId);
	//				}
	//				else l2.emplace_back(item.i, particleId);
	//			}
	//		}

	//		solverCache.l.swap(l);
	//		solverCache.l2.swap(l2);
	//		assembleMatA(l, l2, gId);

	//		//solver.analyzePattern(A);
	//		//solver.factorize(A);
	//		solverCache.solver.compute(solverCache.A);//????????? TODO
	//	}
	//	pSolver = &solverCache.solver;

	//	WR::VecX b;
	//	b.resize(dim);
	//	computeVecb(gId, p0, dr, b);

	//	if (Eigen::Success != pSolver->info())
	//	{
	//		// TODO thread lock needed
	//		std::cout << "Matrix A is not factorizable." << std::endl;
	//		system("pause");
	//		exit(0);
	//	}
	//	x = pSolver->solve(b);
	//}




} /// Hair
} /// XRwy
