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

typedef Eigen::SparseMatrix<float> SparseMatrix;
typedef Eigen::SparseVector<float> SparseVector;

struct Triplet2 { Triplet2(int a, float b) :r(a), val(b) {} int r; float val; };

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

	//while (itrx && itrx.row() <= after) ++itrx;
	//while (itry && itry.row() <= after) ++itry;

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
	for (int i = 0; i < N; ++i) {
		if (v.coeff(i) != 0.0f)
		{
			rot.makeGivens(L.coeff(i, i), -v.coeff(i), &L.coeffRef(i, i));
			if (i < N - 1)
				apply_jacobi_rotation(i, L, i, v, rot);
		}
	}
}

///* See M. Seeger, "Low Rank Updates for the Cholesky Decomposition", 2008 at
//* 	http://lapmal.epfl.ch/papers/cholupdate.pdf
//* for more an explanation of the algorithm used here.
//*/
template <class T>
void sparse_cholesky_downdate(Eigen::SparseMatrix<T>& L,
	Eigen::SparseVector<T>& p) {

	//L.template triangularView<Eigen::Lower>().solveInPlace(p);
	L.triangularView<Eigen::Lower>().solveInPlace(p);
	const size_t N = p.rows();

	assert(p.squaredNorm()
		< 1); // otherwise the downdate would destroy positive definiteness.
	float rho = std::sqrt(1 - p.squaredNorm());

	Eigen::JacobiRotation<float> rot;
	Eigen::SparseVector<T> temp(N);

	for (int i = N - 1; i >= 0; --i) {
		if (p.coeff(i) != 0.0f)
		{
			rot.makeGivens(rho, p.coeff(i), &rho);
			apply_jacobi_rotation2(temp, L, i, rot);
		}
	}
}

#endif /* SPARSE_CHOLESKY_HPP_ */
