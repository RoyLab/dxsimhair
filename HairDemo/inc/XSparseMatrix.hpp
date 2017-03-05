#pragma once
#include <Eigen/Core>
#include <Eigen/Sparse>

#include <map>
#include <cmath>

#include "macros.h"
#include "XException.hpp"

namespace XR
{
	template <class T>
	class SparseVector
	{
	public:
		typedef size_t Index;
		typedef std::map<Index, T> Storage;

		class ConstInnerIterator
		{
		private:
			typedef typename Storage::const_iterator Iterator;
			Iterator data, end;

		public:
			ConstInnerIterator(const SparseVector& m) :
				end(m.m_data.cend()), data(m.m_data.cbegin())
			{}
			ConstInnerIterator(const SparseVector& m, Index row) :
				end(m.m_data.cend()), data(m.m_data.upper_bound(row))
			{}

			operator bool() const { return data != end; }
			void operator++() { data++; }
			void operator++(int) { ++data; }

			Index row() const { return data->first; }
			Index index() const { return data->first; }
			T value() const { return data->second; }
			Iterator pos() const { return data; }
		};

		class InnerIterator
		{
		private:
			typedef typename Storage::iterator Iterator;
			Iterator data, end;

		public:
			InnerIterator(SparseVector& m) :
				end(m.m_data.end()), data(m.m_data.begin())
			{}
			InnerIterator(SparseVector& m,  Index row) :
				end(m.m_data.end()), data(m.m_data.upper_bound(row))
			{}

			operator bool() const { return data != end; }
			void operator++() { data++; }
			void operator++(int) { ++data; }

			Index row() const { return data->first; }
			Index index() const { return data->first; }
			T& value() { return data->second; }
			T value() const { return data->second; }
			Iterator pos() const { return data; }
		};

	private:
		Storage m_data;
		size_t mn_rows = 0;

	public:
		SparseVector() {}
		SparseVector(size_t n):mn_rows(n) {}
		SparseVector(const Eigen::SparseVector<T>& matrix) { *this = matrix; }

		size_t rows() const { return mn_rows; }
		size_t nonZeros() const { return m_data.size(); }
		void setZero() { m_data.clear(); }
		T& coeffRef(Index i) { return m_data[i]; }
		Storage& data() { return m_data; }
		SparseVector& operator=(const Eigen::SparseVector<T>& matrix);
		T sum() const { T res(0); for (auto & itr : m_data) res += m_data.second; return res; }

		T squaredNorm() const;
		InnerIterator getIterator() { return InnerIterator(*this); }
		InnerIterator getIterator(Index col) { return InnerIterator(*this, col); }
		ConstInnerIterator getConstIterator() const { return ConstInnerIterator(*this); }
		ConstInnerIterator getConstIterator(Index after) const { return ConstInnerIterator(*this, after); }
	};

	template <class T>
	class SPDLowerMatrix
	{
	public:
		typedef size_t Index;
		typedef std::map<Index, T> ColStorage;

		class InnerIterator
		{
		private:
			typedef typename ColStorage::iterator Iterator;
			Iterator data, end;

		public:
			InnerIterator(SPDLowerMatrix& m, Index col) :
				end(m.m_data[col].end()), data(m.m_data[col].begin())
			{}
			InnerIterator(SPDLowerMatrix& m, Index col, Index row) :
				end(m.m_data[col].end()), data(m.m_data[col].upper_bound(row))
			{}

			operator bool() const { return data != end; }
			void operator++() { data++; }
			void operator++(int) { ++data; }
			void increAndPrune(ColStorage& c, T thresh)
			{ 
				++data;
				while (data != end && data->second < thresh && data->second > -thresh)
					data = c.erase(data);
			}

			Index row() const { return data->first; }
			Index index() const { return data->first; }
			T& value() { return data->second; }
			T value() const { return data->second; }
			Iterator pos() const { return data; }
		};

		class ConstInnerIterator
		{
		private:
			typedef typename ColStorage::const_iterator Iterator;
			Iterator data, end;

		public:
			ConstInnerIterator(const SPDLowerMatrix& m, Index col) :
				end(m.m_data[col].cend()), data(m.m_data[col].cbegin())
			{}
			ConstInnerIterator(const SPDLowerMatrix& m, Index col, Index row) :
				end(m.m_data[col].cend()), data(m.m_data[col].upper_bound(row))
			{}

			operator bool() const { return data != end; }
			void operator++() { data++; }
			void operator++(int) { ++data; }

			Index row() const { return data->first; }
			Index index() const { return data->first; }
			T value() const { return data->second; }
			Iterator pos() const { return data; }
		};

		class ConstReverseInnerIterator
		{
		private:
			typedef typename ColStorage::const_reverse_iterator Iterator;
			Iterator data, end;

		public:
			ConstReverseInnerIterator() {}
			ConstReverseInnerIterator(const SPDLowerMatrix& m, Index col) :
				end(m.m_data[col].crend()), data(m.m_data[col].crbegin())
			{}

			operator bool() const { return data != end; }
			void operator++() { data++; }
			void operator++(int) { ++data; }

			Index row() const { return data->first; }
			Index index() const { return data->first; }
			T value() const { return data->second; }
			Iterator pos() const { return data; }
		};

	private:
		ColStorage			*m_data = 0;
		size_t				mn_row = 0;

	public:
		SPDLowerMatrix() {}
		SPDLowerMatrix(size_t i) { mn_row = i; m_data = new ColStorage[i]; }
		SPDLowerMatrix(const Eigen::SparseMatrix<T, Eigen::ColMajor>& matrix) { *this = matrix; }
		~SPDLowerMatrix() { clear(); }

		// manip
		void clear() { SAFE_DELETE_ARRAY(m_data); }
		void setZero() { for (int i = 0; i < mn_row; i++) m_data[i].clear(); }
		SPDLowerMatrix& operator=(const Eigen::SparseMatrix<T, Eigen::ColMajor>& matrix);

		// access
		T& coeffRef(Index r, Index c) { return m_data[c][r]; }
		T coeff(Index r, Index c) const { auto res = m_data[c].find(r); if (res == m_data[c].end()) return T(0); else return res->second; }
		T& diag(Index i) { return m_data[i].begin()->second; }
		const T& diag(Index i) const { return m_data[i].begin()->second; }

		// iterator
		InnerIterator getIterator(Index col) { return InnerIterator(*this, col); }
		InnerIterator getIterator(Index col, Index row) { return InnerIterator(*this, col, row); }
		ConstInnerIterator getConstIterator(Index col) const { return ConstInnerIterator(*this, col); }
		ConstInnerIterator getConstIterator(Index col, Index row) const { return ConstInnerIterator(*this, col, row); }

		ConstReverseInnerIterator getReverseConstIterator(Index col) const { return ConstReverseInnerIterator(*this, col); }
		
		// attributes
		size_t nonZeros() const;
		size_t cols() const { return mn_row; }
		size_t rows() const { return mn_row; }
		const ColStorage& col(Index i) const { return m_data[i]; }
		ColStorage& col(Index i) { return m_data[i]; }
		T sum() const;

		// solve
		// forward is lower triangle (default), backward is upper triangle
		// http://mathfaculty.fullerton.edu/mathews/n2003/BackSubstitutionMod.html
		void forwardSubstitutionWithPrune(Eigen::Matrix<T, -1, 1>& b, T thresh = T(0));
		void forwardSubstitutionWithPrunex3(Eigen::Matrix<T, -1, 1>& b, T thresh = T(0));

		void forwardSubstitution(Eigen::Matrix<T, -1, 1>& b) const;
		void backwardSubstitution(Eigen::Matrix<T, -1, 1>& b) const;
		void backwardSubstitutionx3(Eigen::Matrix<T, -1, 1>& b) const;

		void forwardSubstitution(Eigen::SparseVector<T>& b) const;
		void backwardSubstitution(Eigen::SparseVector<T>& b) const;

		void forwardSubstitution(SparseVector<T>& b) const;
	};

	template <class T>
	void assign(Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& a, const SPDLowerMatrix<T> & b)
	{
		a.resize(b.rows(), b.cols());
		a.setZero();

		for (int i = 0; i < b.cols(); i++)
		{
			auto itr = b.getConstIterator(i);
			for (; itr; ++itr)
				a(itr.index(), i) = itr.value();
		}
	}

	template <class T>
	void assign(Eigen::Matrix<T, Eigen::Dynamic, 1>& a, const SparseVector<T>& b)
	{
		a.resize(b.rows());
		a.setZero();

		auto itr = b.getConstIterator();
		for (; itr; ++itr)
			a(itr.index()) = itr.value();
	}


	template<class T>
	inline T SparseVector<T>::squaredNorm() const
	{
		T res = T(0);
		for (auto& pair : m_data)
		{
			res += std::pow(pair.second, 2);
		}
		return res;
	}


	template<class T>
	SparseVector<T>&  SparseVector<T>::operator=(const Eigen::SparseVector<T>& matrix)
	{
		mn_rows = matrix.rows();
		m_data.clear();
		for (auto itr = std::remove_reference<decltype(matrix)>::type::InnerIterator(matrix);
			itr; ++itr)
		{
			m_data[itr.index()] = itr.value();
		}
		return *this;
	}

	template<class T>
	SPDLowerMatrix<T>& SPDLowerMatrix<T>::operator=(const Eigen::SparseMatrix<T, Eigen::ColMajor>& matrix)
	{
		const size_t cols = matrix.cols();
		mn_row = cols;
		SAFE_DELETE_ARRAY(m_data);
		m_data = new std::remove_pointer<decltype(m_data)>::type[cols];

		for (size_t i = 0; i < cols; i++)
		{
			for (auto itr = std::remove_reference<decltype(matrix)>::type::InnerIterator(matrix, i);
				itr; ++itr)
			{
				assert(i <= itr.index()); // lower triangle
				m_data[i][itr.index()] = itr.value();
			}
		}
		return *this;
	}

	template<class T>
	size_t SPDLowerMatrix<T>::nonZeros() const
	{
		size_t res = 0;
		for (size_t i = 0; i < mn_row; i++)
		{
			res += m_data[i].size();
		}
		return res;
	}

	template<class T>
	T SPDLowerMatrix<T>::sum() const
	{
		T res(0);
		for (int i = 0; i < cols(); i++)
		{
			for (auto& pair : m_data[i])
				res += pair.second;
		}
		return res;
	}

	template<class T>
	void SPDLowerMatrix<T>::forwardSubstitutionWithPrune(Eigen::Matrix<T, -1, 1>& p, T thresh)
	{
		const size_t sz = p.rows();
		for (int i = 0; i < sz; i++)
		{
			auto itrL = getIterator(i);
			assert(itrL.index() == i);
			T fix = p(i) / itrL.value();
			p(i) = fix;

			// substitution
			while (1)
			{
				itrL.increAndPrune(m_data[i], thresh);
				if (!itrL) break;
				p(itrL.index()) -= fix*itrL.value();
			}
		}
	}

	template<class T>
	void SPDLowerMatrix<T>::forwardSubstitutionWithPrunex3(Eigen::Matrix<T, -1, 1>& p, T thresh)
	{
		const size_t sz = rows();
		for (int i = 0; i < sz; i++)
		{
			auto itrL = getIterator(i);
			assert(itrL.index() == i);
			T fix[3];
			for (int j = 0; j < 3; j++)
			{
				p(3 * i + j) /= itrL.value();
			}

			// substitution
			while (1)
			{
				itrL.increAndPrune(m_data[i], thresh);
				if (!itrL) break;
				for (int j = 0; j < 3; j++)
				{
					p(3 * itrL.index() + j) -= p(3 * i + j)*itrL.value();
				}
			}
		}
	}

	template<class T>
	void SPDLowerMatrix<T>::forwardSubstitution(Eigen::Matrix<T, -1, 1>& p) const
	{
		const size_t sz = p.rows();
		for (int i = 0; i < sz; i++)
		{
			auto itrL = getConstIterator(i);
			assert(itrL.index() == i);
			T fix = p(i) / itrL.value();
			p(i) = fix;

			// substitution
			for (++itrL; itrL; ++itrL)
				p(itrL.index()) -= fix*itrL.value();
		}
	}

	template<class T>
	void SPDLowerMatrix<T>::backwardSubstitution(Eigen::Matrix<T, -1, 1>& p) const
	{
		const size_t sz = p.rows();
		T div;
		for (int i = sz - 1; i >= 0; i--)
		{
			auto itrL = getConstIterator(i);
			assert(itrL.index() == i);
			div = itrL.value();

			for (itrL++; itrL; itrL++)
				p(i) -= p(itrL.index()) * itrL.value();

			p(i) /= div;
		}
	}

	template<class T>
	void SPDLowerMatrix<T>::backwardSubstitutionx3(Eigen::Matrix<T, -1, 1>& p) const
	{
		const size_t sz = rows();
		T div;
		for (int i = sz - 1; i >= 0; i--)
		{
			auto itrL = getConstIterator(i);
			assert(itrL.index() == i);
			div = itrL.value();

			for (itrL++; itrL; itrL++)
			{
				for (int j = 0; j < 3; j++)
					p(3 * i + j) -= p(3 * itrL.index() + j) * itrL.value();
			}

			for (int j = 0; j < 3; j++)
			{
				p(3 * i + j) /= div;
			}
		}
	}

	template<class T>
	void SPDLowerMatrix<T>::forwardSubstitution(Eigen::SparseVector<T>& p) const
	{
		std::remove_reference<decltype(p)>::type::Storage &data = p.data();
		for (int i = 0; i < p.nonZeros(); i++)
		{
			auto idx = data.index(i);
			auto val = data.value(i);
			auto itrL = getConstIterator(idx);
			assert(itrL.index() == idx);
			T fix = val / itrL.value();
			data.value(i) = fix;

			// substitution
			for (++itrL; itrL; ++itrL)
				p.coeffRef(itrL.index()) -= fix*itrL.value();
		}
	}

	template<class T>
	void SPDLowerMatrix<T>::forwardSubstitution(SparseVector<T>& p) const
	{
		std::remove_reference<decltype(p)>::type::Storage &data = p.data();
		for (auto &pair : data)
		{
			auto idx = pair.first;
			auto val = pair.second;
			auto itrL = getConstIterator(idx);
			assert(itrL.index() == idx);
			T fix = val / itrL.value();
			pair.second = fix;

			// substitution
			for (++itrL; itrL; ++itrL)
				p.coeffRef(itrL.index()) -= fix*itrL.value();
		}
	}

	template<class T>
	void SPDLowerMatrix<T>::backwardSubstitution(Eigen::SparseVector<T>& b) const
	{
		throw XR::NotImplementedException();
	}


}
