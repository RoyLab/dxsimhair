#define XRWY_EXPORTS
#include <XR_Exception.hpp>
#include <boost\log\trivial.hpp>
#include <iostream>

#include "XSparseMatrix.h"

namespace XRwy
{
namespace core
{
	SPDLowerMatrix& SPDLowerMatrix::operator=(const Eigen::SparseMatrix<T, Eigen::ColMajor>& matrix)
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

	size_t SPDLowerMatrix::nonZeros() const
	{
		size_t res = 0;
		for (size_t i = 0; i < mn_row; i++)
		{
			res += m_data[i].size();
		}
		return res;
	}

	SPDLowerMatrix::T SPDLowerMatrix::sum() const
	{
		T res(0);
		for (int i = 0; i < cols(); i++)
		{
			for (auto& pair : m_data[i])
				res += pair.second;
		}
		return res;
	}

	void SPDLowerMatrix::forwardSubstitutionWithPrune(Eigen::Matrix<T, -1, 1>& p, T thresh)
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

	void SPDLowerMatrix::forwardSubstitutionWithPrunex3(Eigen::Matrix<T, -1, 1>& p, T thresh)
	{
		const size_t sz = rows();
		for (int i = 0; i < sz; i++)
		{
			auto itrL = getIterator(i);
			assert(itrL.index() == i);
			T fix[3];
			for (int j = 0; j < 3; j++)
			{
				//if (std::isinf(p(3 * i + j)) || std::abs(p(3 * i + j)) > 2000)
				//	std::cout << "!!!!!!!!!!!!" << itrL.value();
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

	void SPDLowerMatrix::forwardSubstitution(Eigen::Matrix<T, -1, 1>& p) const
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

	void SPDLowerMatrix::backwardSubstitution(Eigen::Matrix<T, -1, 1>& p) const
	{
		const size_t sz = p.rows();
		T div;
		for (int i = sz-1; i >=0; i--)
		{
			auto itrL = getConstIterator(i);
			assert(itrL.index() == i);
			div = itrL.value();

			for (itrL++; itrL; itrL++)
				p(i) -= p(itrL.index()) * itrL.value();

			p(i) /= div;
		}
	}

	void SPDLowerMatrix::backwardSubstitutionx3(Eigen::Matrix<T, -1, 1>& p) const
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
					p(3*i+j) -= p(3*itrL.index()+j) * itrL.value();
			}

			for (int j = 0; j < 3; j++)
			{
				//if (std::isinf(p(3 * i + j)) || std::abs(p(3 * i + j)) > 2000)
				//	std::cout << "!!!!!!!!!!!!" << div;
				p(3 * i + j) /= div;
			}
		}
	}

	//void SPDLowerMatrix::backwardSubstitution(Eigen::Matrix<T, -1, 1>& p) const
	//{
	//	//std::vector<std::vector<Index>> slot(mn_row);
	//	//std::vector<ConstReverseInnerIterator> iters(mn_row);
	//	const unsigned char ERROR = 0xfe;
	//	const Index ERRORX4 = 0xfefefefe;

	//	Index(*slot)[2] = new Index[mn_row][2];
	//	memset(slot, ERROR, sizeof(Index)*mn_row * 2);

	//	ConstReverseInnerIterator* iters = new ConstReverseInnerIterator[mn_row];

	//	Index* linkedList = new Index[mn_row];
	//	memset(linkedList, ERROR, sizeof(Index)*mn_row);

	//	for (size_t i = 0; i < mn_row; i++)
	//	{
	//		iters[i] = getReverseConstIterator(i);
	//		auto j = iters[i].index();
	//		if (j != i) // diag is not added;
	//		{
	//			if (slot[j][0] == ERRORX4)
	//			{
	//				assert(slot[j][1] == ERRORX4);
	//				slot[j][0] = i;
	//				slot[j][1] = i;
	//			}
	//			else
	//			{
	//				linkedList[slot[j][1]] = i;
	//				slot[j][1] = i;
	//			}
	//		}
	//	}

	//	const size_t sz = p.rows();
	//	for (int i = sz-1; i >= 0; i--)
	//	{
	//		auto val = p(i);
	//		T fix = val / diag(i);
	//		p(i) = fix;

	//		// substitution
	//		auto cur = slot[i][0];
	//		while (cur != ERRORX4)
	//		{
	//			auto item = cur;
	//			p(item) -= fix*iters[item].value();

	//			iters[item]++;
	//			auto k = iters[item].index();
	//			if (k != item) // diag is not added;
	//			{
	//				if (slot[k][0] == ERRORX4)
	//				{
	//					assert(slot[k][1] == ERRORX4);
	//					slot[k][0] = item;
	//					slot[k][1] = item;
	//				}
	//				else
	//				{
	//					linkedList[slot[k][1]] = item;
	//					slot[k][1] = item;
	//				}
	//			}
	//			auto tmp = linkedList[cur];
	//			linkedList[cur] = ERRORX4;
	//			cur = tmp;
	//		}
	//	}

	//	delete[]slot;
	//	delete[]iters;
	//	delete[]linkedList;
	//}


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

	void SPDLowerMatrix::forwardSubstitution(SparseVector<T>& p) const
	{
		std::remove_reference<decltype(p)>::type::Storage &data = p.data();
		for (auto &pair: data)
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


	void SPDLowerMatrix::backwardSubstitution(Eigen::SparseVector<T>& b) const
	{
		throw XR::NotImplementedException();
	}

	void XRWY_DLL assign(Eigen::MatrixXf & a, const SPDLowerMatrix & b)
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

	void XRWY_DLL assign(Eigen::VectorXf& a, const SparseVector<float>& b)
	{
		a.resize(b.rows());
		a.setZero();

		auto itr = b.getConstIterator();
		for (; itr; ++itr)
			a(itr.index()) = itr.value();
	}
}
}