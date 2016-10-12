#pragma once
#include <map>
#include <Eigen/Sparse>
#include "macros.h"

namespace XRwy
{
namespace core
{
	class XRWY_DLL SPDLowerMatrix
	{
	public:
		typedef float T;
		typedef size_t Index;
		typedef std::map<Index, T> ColStorage;


		class InnerIterator
		{
		private:
			ColStorage::iterator data, end;

		public:
			InnerIterator(SPDLowerMatrix& m, Index col, Index row = 0) :
				end(m.m_data[col].end()), data(m.m_data[col].lower_bound(row)) 
			{}

			operator bool() const { return data != end; }
			void operator++() { data++; }
			void operator++(int) { ++data; }

			Index row() const { return data->first; }
			Index index() const { return data->first; }
			T value() const { return data->second; }
		};

		class ConstInnerIterator
		{
		private:
			ColStorage::const_iterator data, end;

		public:
			ConstInnerIterator(const SPDLowerMatrix& m, Index col) :
				end(m.m_data[col].cend()), data(m.m_data[col].begin())
			{}
			ConstInnerIterator(const SPDLowerMatrix& m, Index col, Index row) :
				end(m.m_data[col].cend()), data(m.m_data[col].lower_bound(row))
			{}

			operator bool() const { return data != end; }
			void operator++() { data++; }
			void operator++(int) { ++data; }

			Index row() const { return data->first; }
			Index index() const { return data->first; }
			T value() const { return data->second; }
		};

		class ConstReverseInnerIterator
		{
		private:
			ColStorage::const_reverse_iterator data, end;

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

		// solve
		// forward is lower triangle (default), backward is upper triangle
		// http://mathfaculty.fullerton.edu/mathews/n2003/BackSubstitutionMod.html
		void forwardSubstitution(Eigen::Matrix<T, -1, 1>& b) const;
		void backwardSubstitution(Eigen::Matrix<T, -1, 1>& b) const;
		void forwardSubstitution(Eigen::SparseVector<T>& b) const;
		void backwardSubstitution(Eigen::SparseVector<T>& b) const;

	};
}
}
