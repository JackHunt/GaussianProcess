#include "GPRegressor.h"

using namespace GaussianProcess;

GPRegressor::GPRegressor(KernelType kernType){
	switch(kernType){
	case SQUARED_EXPONENTIAL:
		kernel.reset(new SquaredExponential());
		break;
	default:
		throw std::runtime_error("Invalid kernel choice.");
	}
}

GPRegressor::~GPRegressor(){
	//
}

#ifdef WITH_PYTHON_BINDINGS
double GPRegressor::runRegression(const double *trainData, int trainCols, int trainRows, const double *trainTruth,
								  int trainTruthRows, const double *testData, int testCols, int testRows,
								  const double *testTruth, int testTruthRows, const ParamaterSet &params){
	runRegression(trainData, trainTruth, trainRows, trainCols, testData, testTruth, testRows, testCols, params);
}
#endif

double GPRegressor::runRegression(const double *trainData, const double *trainTruth, int trainRows, int trainCols,
								  const double *testData, const double *testTruth, int testRows, int testCols,
								  const ParamaterSet &params){
	if(trainCols != testCols){
		throw std::runtime_error("Train and test sets must have the same number of columns.");
	}
	
	//Wrap data in Eigen matrices and vectors.
	const Eigen::Map<const Matrix> X(trainData, trainRows, trainCols);
	const Eigen::Map<const Matrix> X_s(testData, testRows, testCols);
	const Eigen::Map<const Vector> Y(trainTruth, trainRows);
	const Eigen::Map<const Vector> Y_s(testTruth, testRows);

	//Compute covariance matrices.
    K.resize(trainRows, trainRows);
	K_s.resize(trainRows, testRows);
	K_ss.resize(testRows, testRows);
    buildCovarianceMatrix(X, X, K, params);
	buildCovarianceMatrix(X, X_s, K_s, params);
	buildCovarianceMatrix(X_s, X_s, K_ss, params);

	//Solve for alpha.
    L.resize(trainRows, trainRows);
	jitterChol(K, L);
	tmp = L.triangularView<Eigen::Lower>().solve(Y);
	alpha = L.transpose().triangularView<Eigen::Lower>().solve(tmp);

	//Solve for test means and train variances.
	f_s = K_s.transpose() * alpha;
	v = L.triangularView<Eigen::Lower>().solve(K_s);
	v_s = K_ss - v.transpose() * v;

	//Get the MSE.
	auto sq = [](auto a){return a*a;};
	predDiff = Y_s - f_s;
	predDiff = predDiff.unaryExpr(sq);
	return predDiff.mean();
}

const double *GPRegressor::getMeans() const {
	return f_s.data();
}

const double *GPRegressor::getCovariances() const {
	return v_s.data();
}

const double *GPRegressor::getStdDev() {
	predSD.resizeLike(f_s);

	const size_t len = predSD.rows();
#ifdef WITH_OPENMP
#pragma omp parallel for schedule(dynamic)
#endif
	for(int i = 0; i < len; i++) {
		predSD(i) = sqrt(v_s(i, i));
	}

	return predSD.data();
}

void GPRegressor::buildCovarianceMatrix(const Eigen::Map<const Matrix> &A, const Eigen::Map<const Matrix> &B,
										Matrix &C, const ParamaterSet &params){
	const size_t rowsA = A.rows();
	const size_t rowsB = B.rows();
#ifdef WITH_OPENMP
#pragma omp parallel for schedule(dynamic) collapse(2)
#endif
	for(size_t i = 0; i < rowsA; i++){
		for(size_t j = 0; j < rowsB; j++){
			C(i, j) = kernel->f(A.row(i), B.row(j), params);
		}
	}
}

void GPRegressor::jitterChol(const Matrix &A, Matrix &C){
	const size_t rowsA = A.rows();
	const size_t colsA = A.cols();
	if(rowsA != colsA){
		throw std::runtime_error("Cannot take Cholesky Decomposition of non square matrix.");
	}
	
	Matrix jitter = Matrix::Identity(rowsA, colsA);
	jitter *= 1e-8;
	
	bool passed = false;

	while(!passed && jitter(0,0) < 1e4){
		Eigen::LLT<Matrix> chol(A + jitter);
		if(chol.info() == Eigen::NumericalIssue){
			jitter *= 1.1;
		}else{
			passed = true;
			C = chol.matrixL();
		}
	}

	if(!passed){
		throw std::runtime_error("Unable to make matrix positive semidefinite.");
	}
}
