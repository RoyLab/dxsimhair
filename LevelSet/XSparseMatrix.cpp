#define XRWY_EXPORTS
#include "XSparseMatrix.h"
#include <XR_Exception.hpp>
#include <boost\log\trivial.hpp>

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
		std::vector<std::vector<Index>> slot(mn_row);
		std::vector<ConstReverseInnerIterator> iters(mn_row);

		for (size_t i = 0; i < mn_row; i++)
		{
			iters[i] = getReverseConstIterator(i);
			auto j = iters[i].index();
			if (j != i) // diag is not added;
				slot[j].push_back(i);
		}

		const size_t sz = p.rows();
		for (int i = sz-1; i >= 0; i--)
		{
			auto val = p(i);
			T fix = val / diag(i);
			p(i) = fix;

			// substitution
			for (size_t j = 0; j < slot[i].size(); j++)
			{
				auto item = slot[i][j];
				p(item) -= fix*iters[item].value();

				iters[item]++;
				auto k = iters[item].index();
				if (k != item) // diag is not added;
					slot[k].push_back(item);
			}
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