/******************************************************************************
 *   Copyright (C) 2007-2021 by the GIMLi development team                    *
 *   Carsten Rücker carsten@resistivity.net                                   *
 *                                                                            *
 *   Licensed under the Apache License, Version 2.0 (the "License");          *
 *   you may not use this file except in compliance with the License.         *
 *   You may obtain a copy of the License at                                  *
 *                                                                            *
 *       http://www.apache.org/licenses/LICENSE-2.0                           *
 *                                                                            *
 *   Unless required by applicable law or agreed to in writing, software      *
 *   distributed under the License is distributed on an "AS IS" BASIS,        *
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *   See the License for the specific language governing permissions and      *
 *   limitations under the License.                                           *
 *                                                                            *
 ******************************************************************************/

#include "gimli.h"
#include "vector.h"
#include "elementmatrix.h"

#include "matrix.h"
// #include "vector.h"
#include "stopwatch.h"

#if OPENBLAS_CBLAS_FOUND
    #include <cblas.h>
#endif


namespace GIMLI{


void toEigenMatrix(const RMatrix & m, SmallMatrix & r){
    r.resize(m.rows(), m.cols());

    for (Index i=0; i < r.rows(); i ++){
#if USE_EIGEN3
        r(i, Eigen::all) = Eigen::Map <const Eigen::VectorXd>(&m[i][0],
                                                              r.cols());
#else
    for (Index j=0; j < r.cols(); j ++){
        r(i,j) = m(i,j);
    }
#endif
    }
}
void toRMatrix(const SmallMatrix & m, RMatrix & r){
    r.resize(m.rows(), m.cols());

    // optimize if in use
    for (Index i=0; i < r.rows(); i ++){
        for (Index j=0; j < r.cols(); j ++){
            r(i,j) = m(i,j);
        }
    }
}

#if USE_EIGEN3
void toRVector(const Eigen::VectorXd & m, RVector & r, double b){
    r.resize(m.size());

    // optimize if in use
    if (b == 0.0){
        for (Index i=0; i < m.size(); i ++){
            r[i] = m(i);
        }
    } else if (b == 1.0){
        for (Index i=0; i < m.size(); i ++){
            r[i] += m(i);
        }
    } else if (b == -1.0){
        for (Index i=0; i < m.size(); i ++){
            r[i] -= m(i);
        }
    } else if (b == -1.0){
        for (Index i=0; i < m.size(); i ++){
            r[i] = m(i) + r[i] * b;
        }
    }
}
#endif

template < class ValueType > Vector < ValueType >
_mult(const Matrix< ValueType > & M, const Vector < ValueType > & b) {
    Index cols = M.cols();
    Index rows = M.rows();

    Vector < ValueType > ret(rows, 0.0);

    //ValueType tmpval = 0;
    if (b.size() == cols){
        for (Index i = 0; i < rows; ++i){
            ret[i] = sum(M.mat_[i] * b);
            // for (Index j = 0; j < cols; j++){
            //     ret[i] += M.mat_[i][j] * b[j];
            // }
        }
    } else {
        throwLengthError(WHERE_AM_I + " " + str(cols) + " != " + str(b.size()));
    }
    return ret;
}

template<> Vector < double >
Matrix< double >::mult(const Vector < double > & b) const { return _mult((*this), b); }
template<> Vector < Complex >
Matrix< Complex >::mult(const Vector < Complex > & b) const { return _mult((*this), b); }

template < class ValueType > Vector < ValueType >
_mult(const Matrix< ValueType > & M, const Vector < ValueType > & b, Index startI, Index endI) {
    Index cols = M.cols();
    Index rows = M.rows();
    Index bsize = Index(endI - startI);

    if (bsize != cols) {
        throwLengthError(WHERE_AM_I + " " + str(cols) + " < " + str(endI) + "-" + str(startI));
    }
    Vector < ValueType > ret(rows, 0.0);
    for (Index i = 0; i < rows; ++i){
        for (Index j = startI; j < endI; j++) {
            ret[i] += M.mat_[i][j] * b[j];
        }
    }
    return ret;
}

template<> Vector < double >
Matrix< double >::mult(const Vector < double > & b, Index startI, Index endI) const {
    return _mult((*this), b, startI, endI);
}
template<> Vector < Complex >
Matrix< Complex >::mult(const Vector < Complex > & b, Index startI, Index endI) const {
    return _mult((*this), b, startI, endI);
}

template < class ValueType > Vector < ValueType >
_transMult(const Matrix< ValueType > & M, const Vector < ValueType > & b) {
    Index cols = M.cols();
    Index rows = M.rows();
    Vector < ValueType > ret(cols, 0.0);

    if (b.size() == rows){
        for (Index i = 0; i < rows; i++){
            // ret += M.mat_[i] * b[i];
            for (Index j = 0; j < cols; j++){
                ret[j] += M.mat_[i][j] * b[i];
            }
        }
    } else {
        throwLengthError(WHERE_AM_I + " " + str(rows) + " != " + str(b.size()));
    }
    return ret;
}

template<> Vector< double >
Matrix< double >::transMult(const Vector < double > & b) const {
    return _transMult((*this), b);
}
template<> Vector< Complex >
Matrix< Complex >::transMult(const Vector < Complex > & b) const {
    return _transMult((*this), b);
}

template < class ValueType > Matrix < ValueType > &
_transAdd(Matrix < ValueType > * a, const Matrix < ValueType > & b){
    // a+=b.T
    if (a->rows() != b.cols() || a->cols() != b.rows()){
        __MS(a->rows(), a->cols(), ":",  b.rows(), b.cols())
        log(Error, "Matrix _transAdd with wrong dimensions");
        return *a;
    }

    for (Index i = 0; i < a->rows(); i ++ ){
        for (Index j = 0; j < a->cols(); j ++ ){
            a->mat_[i][j] += b.mat_[j][i];
        }
    }
    return *a;
}
template <> Matrix < double > &
Matrix<double>::transAdd(const Matrix < double > & a){
    return _transAdd(this, a);
}
template <> Matrix < Complex > &
Matrix<Complex>::transAdd(const Matrix < Complex > & a){
    return _transAdd(this, a);
}


void matMultABA(const SmallMatrix & A, const SmallMatrix & B, 
                SmallMatrix & C, SmallMatrix & AtB, double a, double b){

#if USE_EIGEN3
    THROW_TO_IMPL
#else
    return matMultABA_RM(A, B, C, AtB, a, b);
#endif
}
void matMultABA_RM(const RMatrix & A, const RMatrix & B, 
                   RMatrix & C, RMatrix & AtB, double a, double b){
    // C = a A.T * B * A + b * C
    // __MS("matMultABA: ", A.rows(), A.cols(), B.rows(), B.cols())

    if (A.rows() != B.rows()){
        log(Error, "matMultABA B sizes mismatch.", A.rows(), "!=", B.rows());
        return;
    }
    AtB.resize(A.cols(), B.rows());
    matTransMult_RM(A, B, AtB, 1.0, 0.0);
    matMult_RM(AtB, A, C, a, b);
}
void matMult(const SmallMatrix & A, const SmallMatrix & B, 
             SmallMatrix & C, double a, double b){
#if USE_EIGEN3
    if (A.cols() == B.rows()){
        
        if (C.rows() != A.rows() || C.cols() != B.cols()){
            C.resize(A.rows(), B.cols());
        }
        if (b == 0.0){
            C = a*(A*B);
        } else if (b == 1.0){
            C += a*(A*B);
        } else if (b == -1.0){
            C += a*(A*B);
        } else {
            C = b*C + a*(A*B);
        }
    } else if (A.cols() == B.cols()){
        if (C.rows() != A.rows() || C.cols() != B.rows()){
            C.resize(A.rows(), B.rows());
        }
        if (b == 0.0){
            C = a*(A*B.transpose());
        } else if (b == 1.0){
            C += a*(A*B.transpose());
        } else if (b == -1.0){
            C += a*(A*B.transpose());
        } else {
            C = b*C + a*(A*B.transpose());
        }
    } else {
        log(Error, "matMult sizes mismatch. ", A.cols(), "!=", B.rows());
    }

#else
    return matMult_RM(A, B, C, a, b);
#endif
}
void matMult_RM(const RMatrix & A, const RMatrix & B, 
                RMatrix & C, double a, double b){
    // C = a * A*B + b*C || C = a * A*B.T + b*C
    
    // __MS("matMult: ", A.rows(), A.cols(), B.rows(), B.cols())

    Index m = A.rows(); // C.rows()
    Index k = A.cols(); 
    
    // B.rows()
    Index n = B.cols(); // C.cols()
    bool bIsTrans = false;    
    Index bRows = n;

    if (k == B.rows()){ // A * B (k == k)
    
    } else if (k == B.cols()){ // A * B.T (k == k)
        bIsTrans = true;
        n = B.rows();
        bRows = k;
    } else {
        log(Error, "matMult sizes mismatch. ", 
            A.cols(), "!=", B.rows());
    }
    C.resize(m, n);

#if OPENBLAS_CBLAS_FOUND

    CBLAS_TRANSPOSE aTrans = CblasNoTrans;
    CBLAS_TRANSPOSE bTrans = CblasNoTrans;

    if (bIsTrans) bTrans = CblasTrans;
    

    double *A2 = new double[m * k];
    double *B2 = new double[k * n];
    double *C2 = new double[m * n];

    A.dumpData(A2);
    B.dumpData(B2);
    C.dumpData(C2);

    // cblas_dgemm(CblasRowMajor, CblasTrans, CblasNoTrans, m, n, k,
    //             a, A2, m, B2, n, b, C2, n);
                
    // lda ## leading dimension for a, means column for CblasRowMajor
    cblas_dgemm(CblasRowMajor, aTrans, bTrans, 
                m, n, k,
                a, A2, k, B2, bRows,
                b, C2, n);

    C.fromData(C2, m, n);

    delete [] A2;
    delete [] B2;
    delete [] C2;
#else
    
    // __MS("\t: ", C.rows(), C.cols(), bIsTrans)

    for (Index i = 0; i < A.rows(); i ++){

        for (Index j = 0; j < n; j ++){
            double c = 0;
            for (Index k = 0; k < A.cols(); k ++){
                if (bIsTrans){
                    c += A[i][k] * B[j][k];
                } else {
                    c += A[i][k] * B[k][j];
                }
            }
            if (b == 0.0){
                C[i][j] = a * c;
            } else if (b == 1.0){
                C[i][j] += a * c;
            } else if (b == -1.0){
                C[i][j] -= a * c;
            } else {
                C[i][j] = b * C[i][j] + a * c;
            }
        }
    }
#endif
}

void matTransMult(const SmallMatrix & A, const SmallMatrix & B, 
                  SmallMatrix & C, double a, double b){
#if USE_EIGEN3
    if (A.rows() == B.rows()){
        
        if (C.rows() != A.cols() || C.cols() != B.cols()){
            C.resize(A.cols(), B.cols());
        }
        if (b == 0.0){
            C = a*(A.transpose()*B);
        } else if (b == 1.0){
            C += a*(A.transpose()*B);
        } else if (b == -1.0){
            C += a*(A.transpose()*B);
        } else {
            C = b*C + a*(A.transpose()*B);
        }
    } else if (A.rows() == B.cols()){
        if (C.rows() != A.cols() || C.cols() != B.rows()){
            C.resize(A.cols(), B.rows());
        }
        if (b == 0.0){
            C = a*(A.transpose()*B.transpose());
        } else if (b == 1.0){
            C += a*(A.transpose()*B.transpose());
        } else if (b == -1.0){
            C += a*(A.transpose()*B.transpose());
        } else {
            C = b*C + a*(A.transpose()*B.transpose());
        }
    } else {
        log(Error, "matTransMult sizes mismatch. ", A.rows(), "!=", B.rows());
    }

#else
    matTransMult_RM(A, B, C, a, b);
#endif
}
void matTransMult_RM(const RMatrix & A, const RMatrix & B, 
                     RMatrix & C, double a, double b){

    // __MS("matTransMult: ", A.rows(), A.cols(), B.rows(), B.cols())
    
    // Mxk * kxN == MxN
    // (kxM).T * kxN == MxN
    // A (k,M), B(k,N) == C(M,N)
    //** C = a * A.T*B + b*C || C = a * A.T*B.T + b*C

    //** **MEH C = (a * A.T*B).T + b*C if C has the right dimension **  MEH
    // c-transpose check only for b != 0.0, else c is resized
    
    bool retTrans = false;

    // A(k, m).T * B(k, n) = C(m, n)

    Index k = A.rows(); // B.rows()
    Index m = A.cols(); // C.rows()
    Index n = B.cols(); // C.cols()

    bool bIsTrans = false;    
    Index bRows = n;

    if (k == B.rows()){ // A.T * B
            
        if (b == 0.0) C.resize(m, n);

        if (C.rows() != A.cols() || C.cols() != B.cols()){
            __MS("isinuse?")
            //__MS(C.rows(), C.cols(), A.cols(), B.cols())
            //** Target array have wrong dimensions
            if (C.rows() == B.cols() && C.cols() == A.cols()){
                // C = a * B.T*A + b*C
                //** Target array seems needed to be transposed
                //** C = a*(A.T*B).T + b * C
                // __MS("ret transmult")
                return matTransMult_RM(B, A, C, a, b);

                retTrans = true;
            } else {
                //** resize target array
                C.resize(m, n);
            }
        }
    } else if(k == B.cols()){
        bIsTrans = true;
        n = B.rows();
        bRows = k;

    } else {
        // if not A.T * B or A.T * B.T
        __MS(A.rows(), A.cols(), '\n', A)
        __MS(B.rows(), B.cols(), '\n', B)
        __MS(C.rows(), C.cols())
        throwLengthError("matTransMult sizes mismatch.");
    }

#if OPENBLAS_CBLAS_FOUND
    CBLAS_TRANSPOSE aTrans = CblasTrans;
    CBLAS_TRANSPOSE bTrans = CblasNoTrans;

    if (bIsTrans) bTrans = CblasTrans;
        
    double *A2 = new double[k * m];
    double *B2 = new double[k * n];
    double *C2 = new double[m * n];

    A.dumpData(A2);
    B.dumpData(B2);
    C.dumpData(C2);

    // std::cout << "A" << std::endl;
    // for (Index i = 0; i < m*k; i ++ ){std::cout << A2[i] << " ";} std::cout << std::endl;
    // std::cout << "b" << std::endl;
    // for (Index i = 0; i < n*k; i ++ ){std::cout << B2[i] << " ";} std::cout << std::endl;
    // std::cout << "C1" << std::endl;
    // for (Index i = 0; i < m*n; i ++ ){std::cout << C2[i] << " ";} std::cout << std::endl;

    cblas_dgemm(CblasRowMajor, aTrans, bTrans, m, n, k,
                a, A2, m, B2, bRows, 
                b, C2, n);

    // std::cout << "C2" << std::endl;
    // for (Index i = 0; i < m*n; i ++ ){std::cout << C2[i] << " ";} std::cout << std::endl;

    C.fromData(C2, m, n);

    // std::cout << "C3" << std::endl;
    // std::cout << C << std::endl;

    delete [] A2;
    delete [] B2;
    delete [] C2;

#else

    for (Index i = 0; i < A.cols(); i ++){
        for (Index j = 0; j < n; j ++){
            double c = 0;

            for (Index k = 0; k < A.rows(); k ++){

                if (bIsTrans){
                    c += A[k][i] * B[j][k];
                } else {
                    c += A[k][i] * B[k][j];
                }
            }
            if (retTrans){
                THROW_TO_IMPL
                // C[j][i] = a * c + C[j][i]*b;
            } else {

                if (b == 0.0){
                    C[i][j] = a * c;
                } else if (b == 1.0){
                    C[i][j] += a * c;
                } else {
                    C[i][j] = b * C[i][j] + a * c;
                }
            }
        }
    }
#endif
    
}



} // namespace GIMLI{
