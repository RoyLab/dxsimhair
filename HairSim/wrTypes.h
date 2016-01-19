#pragma once
#include <Eigen\Sparse>
#include <Eigen\Dense>
#include <unordered_map>

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
	inline Eigen::Block<Derived, 3, 1> triple(Eigen::MatrixBase<Derived>& m, int i)
	{
		return Eigen::Block<Derived, 3, 1>(m.derived(), 3 * i, 0);
	}

	template <class Derived>
	inline const Eigen::Block<const Derived, 3, 1> triple(const Eigen::MatrixBase<Derived>& m, int i)
	{
		return Eigen::Block<const Derived, 3, 1>(m.derived(), 3 * i, 0);
	}

	template <class Derived>
	inline Eigen::Block<Derived, 3, 3> squared_triple(Eigen::MatrixBase<Derived>& m, int i, int j)
	{
		return Eigen::Block<Derived, 3, 3>(m.derived(), 3 * i, 3 * j);
	}

	template <class Derived>
	inline const Eigen::Block<const Derived, 3, 3> squared_triple(const Eigen::MatrixBase<Derived>& m, int i, int j)
	{
		return Eigen::Block<const Derived, 3, 3>(m.derived(), 3 * i, 3 * j);
	}

	template <class Derived>
	inline Eigen::Block<Derived, 3, 3> squared_triple(Eigen::MatrixBase<Derived>& m, int i)
	{
		return Eigen::Block<Derived, 3, 3>(m.derived(), 3 * i, 3 * i);
	}

	template <class Derived>
	inline const Eigen::Block<const Derived, 3, 3> squared_triple(const Eigen::MatrixBase<Derived>& m, int i)
	{
		return Eigen::Block<const Derived, 3, 3>(m.derived(), 3 * i, 3 * i);
	}

//#define ss_triple(m, i, j)\
//	((m).block(3, 3, 3 * (i), 3 * (j)))

	inline void add_mat_triple(SparseMat& mat, int mi, int mj, const Mat3& c)
	{
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
				mat.coeffRef(3 * mi + i, 3 * mj + j) += c(i, j);
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

	template<class _MAT>
	void write(char* fileName, _MAT& mat)
	{
		std::ofstream f(fileName);
		f << mat;
		f.close();
	}
}
