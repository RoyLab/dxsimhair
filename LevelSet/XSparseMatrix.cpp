#define XRWY_EXPORTS
#include "XSparseMatrix.h"
#include <XR_Exception.hpp>

namespace XRwy
{
namespace core
{
	SPDLowerMatrix& SPDLowerMatrix::operator=(const Eigen::SparseMatrix<T, Eigen::ColMajor>& matrix)
	{
		const size_t cols = matrix.cols();
		mn_row = cols;
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

	size_t SPDLowerMatrix::nonZeros() const
	{
		size_t res = 0;
		for (size_t i = 0; i < mn_row; i++)
		{
			res += m_data[i].size();
		}
		return res;
	}

	void SPDLowerMatrix::forwardSubstitution(Eigen::Matrix<T, -1, 1>& p) const
	{
		const size_t sz = p.rows();
		for (int i = 0; i < sz; i++)
		{
			auto idx = i;
			auto val = p(i);
			auto itrL = getConstIterator(idx);
			assert(itrL.index() == idx);
			T fix = val / itrL.value();
			p(i) = fix;

			// substitution
			for (++itrL; itrL; ++itrL)
				p(itrL.index()) -= fix*itrL.value();
		}
	}

	void SPDLowerMatrix::backwardSubstitution(Eigen::Matrix<T, -1, 1>& p) const
	{
		const size_t sz = p.rows();
		for (int i = sz-1; i >= 0; i--)
		{
			auto idx = i;
			auto val = p(i);
			auto itrL = getReverseConstIterator(idx);
			assert(itrL.index() == idx);
			T fix = val / itrL.value();
			p(i) = fix;

			// substitution
			for (++itrL; itrL; ++itrL)
				p(itrL.index()) -= fix*itrL.value();
		}
	}

	void SPDLowerMatrix::forwardSubstitution(Eigen::SparseVector<T>& p) const
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
	void SPDLowerMatrix::backwardSubstitution(Eigen::SparseVector<T>& b) const
	{
		throw XR::NotImplementedException();
	}
}
}