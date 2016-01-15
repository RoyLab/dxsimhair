#pragma once
#include <Eigen\Sparse>
#include <Eigen\Dense>

namespace WR
{
	typedef Eigen::Matrix3f Mat3;
	typedef Eigen::Matrix4f Mat4;

	typedef Eigen::Vector3f Vec3;
	typedef Eigen::Vector4f Vec4;

	typedef Eigen::SparseMatrix<float> SparseMat;
	typedef Eigen::SparseVector<float> SparseVec;
	typedef Eigen::MatrixXf	MatX;
	typedef Eigen::VectorXf	VecX;

	template <class Derived>
	Eigen::Block<Derived, 3, 1> triple(Eigen::MatrixBase<Derived>& m, int i)
	{
		return Eigen::Block<Derived, 3, 1>(m.derived(), 3 * i, 0);
	}

	template <class Derived>
	const Eigen::Block<const Derived, 3, 1> triple(const Eigen::MatrixBase<Derived>& m, int i)
	{
		return Eigen::Block<const Derived, 3, 1>(m.derived(), 3 * i, 0);
	}

	template <class Derived>
	Eigen::Block<Derived, 3, 3> squared_triple(Eigen::MatrixBase<Derived>& m, int i, int j)
	{
		return Eigen::Block<Derived, 3, 3>(m.derived(), 3 * i, 3 * j);
	}

	template <class Derived>
	const Eigen::Block<const Derived, 3, 3> squared_triple(const Eigen::MatrixBase<Derived>& m, int i, int j)
	{
		return Eigen::Block<const Derived, 3, 3>(m.derived(), 3 * i, 3 * j);
	}

	template <class Derived>
	Eigen::Block<Derived, 3, 3> squared_triple(Eigen::MatrixBase<Derived>& m, int i)
	{
		return Eigen::Block<Derived, 3, 3>(m.derived(), 3 * i, 3 * i);
	}

	template <class Derived>
	const Eigen::Block<const Derived, 3, 3> squared_triple(const Eigen::MatrixBase<Derived>& m, int i)
	{
		return Eigen::Block<const Derived, 3, 3>(m.derived(), 3 * i, 3 * i);
	}

	template <class Derived>
	Eigen::Block<Derived, 3, 3> squared_triple(Eigen::SparseMatrixBase<Derived>& m, int i, int j)
	{
		return Eigen::Block<Derived, 3, 3>(m.derived(), 3 * i, 3 * j);
	}

	template <class Derived>
	const Eigen::Block<const Derived, 3, 3> squared_triple(const Eigen::SparseMatrixBase<Derived>& m, int i, int j)
	{
		return Eigen::Block<const Derived, 3, 3>(m.derived(), 3 * i, 3 * j);
	}

	template <class Derived>
	Eigen::Block<Derived, 3, 3> squared_triple(Eigen::SparseMatrixBase<Derived>& m, int i)
	{
		return Eigen::Block<Derived, 3, 3>(m.derived(), 3 * i, 3 * i);
	}

	template <class Derived>
	const Eigen::Block<const Derived, 3, 3> squared_triple(const Eigen::SparseMatrixBase<Derived>& m, int i)
	{
		return Eigen::Block<const Derived, 3, 3>(m.derived(), 3 * i, 3 * i);
	}

	
	template <size_t _row, size_t _col>
	class Mat:
		public Eigen::Matrix<float, _row, _col>
	{};

	template <size_t _row>
	class Vec :
		public Eigen::Matrix<float, _row, 1>
	{};

	template <class _matA, class _matB>
	void convert3x3(_matA& a, const _matB& b)
	{
		for (size_t i = 0; i < 3; i++)
			for (size_t j = 0; j < 3; j++)
				a(i, j) = b(i, j);
	}

	template <class _vecA, class _vecB>
	void convert3(_vecA& a, const _vecB& b)
	{
		for (size_t i = 0; i < 3; i++)
			a[i] = b[i];
	}
}
