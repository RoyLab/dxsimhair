/*
* cholesky.hpp
*
*  Created on: 2011-08-11
*      Author: roshan
*/

#ifndef SPARSE_CHOLESKY_HPP_
#define SPARSE_CHOLESKY_HPP_

#include <cmath>

#include <Eigen/Sparse>
#include <Eigen/Jacobi>
#include "XSparseMatrix.h"

typedef Eigen::SparseMatrix<float> SparseMatrix;
typedef Eigen::SparseVector<float> SparseVector;

template <class Index = int, class T = float>
struct XTriplet { XTriplet(const Index& a, const T& b) :r(a), val(b) {} Index r; T val; };

typedef XTriplet<> Triplet2;


#define SPARSE_CHOLESKY_UPDATE_PRUNE
#ifdef SPARSE_CHOLESKY_UPDATE_PRUNE
#define PRUNE_THRESH 1e-5
#endif

/* NOTE: This function is a hack; in particular the first two parameters are
* modified even though
* they are passed by const reference. See
* 	http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html#TopicPlainFunctionsFailing
* for why this hack is required.
*
* Computes [a b]' := rot * [a b]' in-place, where a and b are column vectors
* and rot is a Jacobi
* rotation, i.e. rot = |  cos(x)  sin(x) |
* 	                   | -sin(x)  cos(x) |
*/

template<class IteratorX, class IteratorY>
void rot_compute(std::vector<Triplet2>& xs, std::vector<Triplet2>& ys, IteratorX& itrx,
	IteratorY& itry, const Eigen::JacobiRotation<float>& rot)
{
	float c = rot.c();
	float s = rot.s();

	while (itrx || itry)
	{
		if (itrx)
		{
			if (itry)
			{
				if (itrx.row() < itry.row())
				{
					xs.emplace_back(itrx.row(), c * itrx.value());
					ys.emplace_back(itrx.row(), -s*itrx.value());
					++itrx;
				}
				else if (itrx.row() > itry.row())
				{
					xs.emplace_back(itry.row(), s * itry.value());
					ys.emplace_back(itry.row(), c * itry.value());
					++itry;
				}
				else
				{
					xs.emplace_back(itrx.row(), c * itrx.value() + s * itry.value());
					ys.emplace_back(itrx.row(), -s*itrx.value() + c * itry.value());
					++itrx; ++itry;
				}
			}
			else
			{
				xs.emplace_back(itrx.row(), c * itrx.value());
				ys.emplace_back(itrx.row(), -s*itrx.value());
				++itrx;
			}
		}
		else
		{
			xs.emplace_back(itry.row(), s * itry.value());
			ys.emplace_back(itry.row(), c * itry.value());
			++itry;
		}
	}
}

void apply_jacobi_rotation(int after, SparseMatrix& x, size_t col, SparseVector& y, const Eigen::JacobiRotation<float>& rot)
{
	static std::vector<Triplet2> xs;
	static std::vector<Triplet2> ys;
	xs.clear();
	ys.clear();

	auto itrx = SparseMatrix::InnerIterator(x, col);
	auto itry = SparseVector::InnerIterator(y);

	while (itrx && itrx.row() <= after) ++itrx;
	while (itry && itry.row() <= after) ++itry;

	rot_compute(xs, ys, itrx, itry, rot);

	for (auto& triplet : xs)
		x.coeffRef(triplet.r, col) = triplet.val;

	for (auto& triplet : ys)
		y.coeffRef(triplet.r) = triplet.val;
}

void apply_jacobi_rotation2(SparseVector& x, SparseMatrix& y, size_t col, const Eigen::JacobiRotation<float>& rot)
{
	static std::vector<Triplet2> xs;
	static std::vector<Triplet2> ys;
	xs.clear();
	ys.clear();

	auto itrx = SparseVector::InnerIterator(x);
	auto itry = SparseMatrix::InnerIterator(y, col);

	rot_compute(xs,  ys, itrx, itry, rot);

	for (auto& triplet : xs)
		x.coeffRef(triplet.r) = triplet.val;

	for (auto& triplet : ys)
		y.coeffRef(triplet.r, col) = triplet.val;
}

void apply_jacobi_rotation(int after, XRwy::core::SPDLowerMatrix& x, size_t col, 
	XRwy::core::SparseVector<XRwy::core::SPDLowerMatrix::T>& y, 
	const Eigen::JacobiRotation<XRwy::core::SPDLowerMatrix::T>& rot)
{
	static std::vector<Triplet2> xs;
	static std::vector<Triplet2> ys;
	xs.clear();
	ys.clear();

	auto itrx = x.getIterator(col, after);
	auto itry = y.getConstIterator(after);

	rot_compute(xs, ys, itrx, itry, rot);

	for (auto& triplet : xs)
		x.coeffRef(triplet.r, col) = triplet.val;

	for (auto& triplet : ys)
		y.coeffRef(triplet.r) = triplet.val;
}


void apply_jacobi_rotation2(XRwy::core::SparseVector<XRwy::core::SPDLowerMatrix::T>& x,
	XRwy::core::SPDLowerMatrix& y, size_t col, const Eigen::JacobiRotation<XRwy::core::SPDLowerMatrix::T>& rot)
{
	static std::vector<Triplet2> xs;
	static std::vector<Triplet2> ys;
	xs.clear();
	ys.clear();

	auto itrx = x.getConstIterator();
	auto itry = y.getIterator(col);

	rot_compute(xs, ys, itrx, itry, rot);

	for (auto& triplet : xs)
		x.coeffRef(triplet.r) = triplet.val;

	for (auto& triplet : ys)
		y.coeffRef(triplet.r, col) = triplet.val;
}



/* See M. Seeger, "Low Rank Updates for the Cholesky Decomposition", 2008 at
* 	http://lapmal.epfl.ch/papers/cholupdate.pdf
* for more an explanation of the algorithm used here.
*/
template <class T>
void sparse_cholesky_update(Eigen::SparseMatrix<T>& L,
	Eigen::SparseVector<T>& v) {

	Eigen::JacobiRotation<T> rot;
	const size_t N = v.rows();
	Eigen::SparseVector<T>::Storage& data = v.data();
	for (int i = 0; i < v.nonZeros(); ++i) {
		auto idx = data.index(i);
		rot.makeGivens(L.coeff(idx, idx), -data.value(i), &L.coeffRef(idx, idx));
		if (i < N - 1)
			apply_jacobi_rotation(idx, L, idx, v, rot);
	}
}



template<class T>
void lower_sparse_matrix_solve_in_place_sparse_vector(Eigen::SparseMatrix<T>& L, Eigen::SparseVector<T>& p)
{
	std::remove_reference<decltype(p)>::type::Storage &data = p.data();
	for (int i = 0; i < p.nonZeros(); i++)
	{
		auto idx = data.index(i);
		auto val = data.value(i);
		auto itrL = std::remove_reference<decltype(L)>::type::InnerIterator(L, idx);
		assert(itrL.index() == idx);
		T fix = val / itrL.value();
		data.value(i) = fix;

		// substitution
		for (++itrL; itrL; ++itrL)
			p.coeffRef(itrL.index()) -= fix*itrL.value();
	}
}

///* See M. Seeger, "Low Rank Updates for the Cholesky Decomposition", 2008 at
//* 	http://lapmal.epfl.ch/papers/cholupdate.pdf
//* for more an explanation of the algorithm used here.
//*/
template <class T>
void sparse_cholesky_downdate(Eigen::SparseMatrix<T>& L,
	Eigen::SparseVector<T>& p) {

	lower_sparse_matrix_solve_in_place_sparse_vector(L, p);
	const size_t N = p.rows();

	assert(p.squaredNorm()
		< 1); // otherwise the downdate would destroy positive definiteness.

	float rho = std::sqrt(1 - p.squaredNorm());

	Eigen::JacobiRotation<float> rot;
	Eigen::SparseVector<T> temp(N);

	std::remove_reference<decltype(p)>::type::Storage &data = p.data();
	const size_t sz = data.size();
	for (int i = sz - 1; i >= 0; --i)
	{
		auto idx = data.index(i);
		auto value = data.value(i);
		rot.makeGivens(rho, value, &rho);
		apply_jacobi_rotation2(temp, L, idx, rot);
	}
}

template<class MA, class MB, class IteratorX, class IteratorY>
void applyRotInPlace(MA& va, MB& vb, IteratorX& itrx,
	IteratorY& itry, const Eigen::JacobiRotation<float>& rot)
{
	typedef typename MA::value_type PairA;
	typedef typename MB::value_type PairB;
	float c = rot.c();
	float s = rot.s();

	while (itrx || itry)
	{
		if (itrx)
		{
			if (itry)
			{
				if (itrx.row() < itry.row())
				{
					vb.insert(PairB(itrx.row(), -s*itrx.value()));
					itrx.value() = c * itrx.value();
					++itrx;
				}
				else if (itrx.row() > itry.row())
				{
					va.insert(PairA(itry.row(), s * itry.value()));
					itry.value() = c * itry.value();
					++itry;
				}
				else
				{
					float tmp = c * itrx.value() + s * itry.value();
					itry.value() = -s*itrx.value() + c * itry.value();
					itrx.value() = tmp;
					++itrx; ++itry;
				}
			}
			else
			{
				vb.insert(PairB(itrx.row(), -s*itrx.value()));
				itrx.value() = c * itrx.value();
				++itrx;
			}
		}
		else
		{
			va.insert(PairA(itry.row(), s * itry.value()));
			itry.value() = c * itry.value();
			++itry;
		}
	}
}

void sparse_cholesky_update(XRwy::core::SPDLowerMatrix& L,
	XRwy::core::SparseVector<XRwy::core::SPDLowerMatrix::T>& v) {

	typedef XRwy::core::SPDLowerMatrix::T T;
	Eigen::JacobiRotation<T> rot;
	const size_t N = v.rows();
	std::remove_reference<decltype(v)>::type::Storage& data = v.data();
	for (auto &pair : data) {
		auto idx = pair.first;
		auto &ref = L.diag(idx);
		rot.makeGivens(ref, -pair.second, &ref);
		if (idx < N - 1)
		{
			auto itrx = L.getIterator(idx, idx);
			auto itry = v.getIterator(idx);

			applyRotInPlace(L.col(idx), v.data(), itrx, itry, rot);
		}
	}
}

void sparse_cholesky_downdate(XRwy::core::SPDLowerMatrix& L,
	XRwy::core::SparseVector<typename XRwy::core::SPDLowerMatrix::T>& p) {

	typedef std::remove_reference<decltype(L)>::type SparseMatrix;
	typedef std::remove_reference<decltype(p)>::type SparseVector;
	typedef typename SparseMatrix::T T;

	L.forwardSubstitution(p);
	const size_t N = p.rows();

	assert(p.squaredNorm()
		< 1); // otherwise the downdate would destroy positive definiteness.

	float rho = std::sqrt(1 - p.squaredNorm());

	Eigen::JacobiRotation<float> rot;
	SparseVector temp(N);

	SparseVector::Storage &data = p.data();
	const size_t sz = data.size();
	for (auto itr = data.crbegin(); itr != data.crend(); ++itr)
	{
		auto idx = itr->first;
		auto value = itr->second;
		rot.makeGivens(rho, value, &rho);

		//apply_jacobi_rotation2(temp, L, idx, rot);

		auto itrx = temp.getIterator();
		auto itry = L.getIterator(idx);

		applyRotInPlace(temp.data(), L.col(idx), itrx, itry, rot);
	}
}


#endif /* SPARSE_CHOLESKY_HPP_ */
