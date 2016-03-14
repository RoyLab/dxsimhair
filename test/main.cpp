#include <Eigen\Core>
#include <Eigen\Sparse>
#include <iostream>

using namespace Eigen;

int main(int argc, char* argv[])
{
    SparseMatrix<float> A(10, 10);
    A.setIdentity();
    A.coeffRef(0, 0) = 0;

    MatrixXf b, dv;
    b.setOnes();

    Eigen::SparseLU<SparseMatrix<float>> solver;
    solver.analyzePattern(A);
    if (Success != solver.info())
    {
        std::cout << 1 << solver.lastErrorMessage() << std::endl;
        system("pause");
        return -1;
    }

    solver.factorize(A);
    if (Success != solver.info())
    {
        std::cout << 2 << solver.lastErrorMessage() << std::endl;
        system("pause");
        return -1;
    }

    dv = solver.solve(b);
    if (Success != solver.info())
    {
        std::cout << 3 << solver.lastErrorMessage() << std::endl;
        system("pause");
        return -1;
    }

    return 0;
}
