#pragma once
#include"wrTypes.h"


namespace WR
{
	class SparseMatAssemble :
		public Eigen::SparseMatrix < float >
	{
		typedef long long IndexPair;
		typedef std::unordered_map<IndexPair, size_t> HashMap;
		typedef std::pair<IndexPair, size_t> HashPair;

		class Triplet :
			public Eigen::Triplet < float >
		{
		public:
			Triplet() : Eigen::Triplet<float>(){}

			Triplet(const Index& i, const Index& j, const Scalar& v = Scalar(0))
				:Eigen::Triplet<float>(i, j, v){}

			Triplet& operator += (float val){ m_value += val; return *this; }
		};

	public:
		SparseMatAssemble() :
			Eigen::SparseMatrix<float>(){}

		SparseMatAssemble(size_t i, size_t j) :
			Eigen::SparseMatrix<float>(i, j){}

		~SparseMatAssemble(){}

		void add_triple(int mi, int mj, const Mat3& c)
		{
			IndexPair idx = make_index(mi, mj);
			auto loc = m_map.find(idx);
			if (loc == m_map.end())
			{
				m_map.emplace(idx, m_buffer.size());
				for (size_t i = 0; i < 3; i++)
					for (size_t j = 0; j < 3; j++)
						m_buffer.emplace_back(3 * mi + i, 3 * mj + j, c(i, j));
			}
			else
			{
				size_t start = loc->second;
				for (size_t i = 0; i < 3; i++)
					for (size_t j = 0; j < 3; j++)
						m_buffer[start + 3 * i + j] += c(i, j);
			}
		}

		void add_triple_without_check(int mi, int mj, const Mat3& c)
		{
			IndexPair idx = make_index(mi, mj);
			m_map.emplace(idx, m_buffer.size());
			for (size_t i = 0; i < 3; i++)
				for (size_t j = 0; j < 3; j++)
					m_buffer.emplace_back(3 * mi + i, 3 * mj + j, c(i, j));
		}

		void add_triple_diag(int mi, float val)
		{
			IndexPair idx = make_index(mi, mi);
			auto loc = m_map.find(idx);
			assert(loc != m_map.end());

			size_t start = loc->second;
			for (size_t i = 0; i < 3; i++)
				m_buffer[start + 3 * i + i] += val;
		}

		void flush()
		{
			setFromTriplets(m_buffer.begin(), m_buffer.end());
			m_buffer.clear();
			m_map.clear();
		}

		void reserve_hash_map(size_t n){ m_map.reserve(n); m_buffer.reserve(9 * n); }

	protected:

		IndexPair make_index(int id0, int id1)
		{
			IndexPair indexPair;
			indexPair = id1;
			indexPair = indexPair << 32;
			indexPair |= id0;
			return indexPair;
		}

		HashMap					m_map;
		std::vector<Triplet>		m_buffer;
	};

}
