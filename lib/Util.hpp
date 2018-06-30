/*
BSD 3-Clause License

Copyright (c) 2017, Jack Miles Hunt
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GAUSSIAN_PROCESS_UTIL_HEADER
#define GAUSSIAN_PROCESS_UTIL_HEADER

#include <memory>
#include <string>
#include "Kernels.hpp"

namespace GPLib {

    /**
     * @brief jitterChol Performs a cholesky decomposition. Deals with non Pos-Semidef matrices by adding jitter successively to diagonal.
     * @param A Matrix to compute cholesky decomposition of.
     * @param C Matrix to write cholesky decomposition to.
     */
    inline void jitterChol(const Matrix &A, Matrix &C) {
        const size_t rowsA = A.rows();
        const size_t colsA = A.cols();
        if (rowsA != colsA) {
            throw std::runtime_error("Cannot take Cholesky Decomposition of non square matrix.");
        }

        Matrix jitter = Matrix::Identity(rowsA, colsA);
        jitter *= 1e-8;

        bool passed = false;

        //Successively add jitter to make positive semi-definite.
        while (!passed && jitter(0, 0) < 1e4) {
            Eigen::LLT<Matrix> chol(A + jitter);
            if (chol.info() == Eigen::NumericalIssue) {//Not pos-semidefinite.
                jitter *= 1.1;
            }
            else {
                passed = true;
                C = chol.matrixL();
            }
        }

        //If Matrix is still not positive semi-definite.
        if (!passed) {
            throw std::runtime_error("Unable to make matrix positive semidefinite.");
        }
    }

    /**
     * @brief buildCovarianceMatrix Builds a Covariance Matrix between two data matrices using a given kernel.
     * @param A Data matrix A.
     * @param B Data matrix B.
     * @param C Output Covariance Matrix.
     * @param params Parameters for kernel(must match kernel choice)
     * @param kernel Kernel(must match Parameters)
     * @param var Variable to differentiate w.r.t if derivative matrix is required.
     */
    inline void buildCovarianceMatrix(const Eigen::Map<const Matrix> &A, const Eigen::Map<const Matrix> &B,
                                      Matrix &C, const ParameterSet &params, const std::shared_ptr<Kernel> &kernel,
        const std::string &var = std::string("")) {
        const size_t rowsA = A.rows();
        const size_t rowsB = B.rows();
#ifdef WITH_OPENMP
#pragma omp parallel for
#endif
        for (int i = 0; i < rowsA; i++) {
            for (int j = 0; j < rowsB; j++) {
                if (var.compare("") != 0) {
                    C(i, j) = kernel->df(A.row(i), B.row(j), params, var);
                }
                else {
                    C(i, j) = kernel->f(A.row(i), B.row(j), params);
                }
            }
        }
    }
}

#endif