/******************************************************************************
 *   Copyright (C) 2006-2021 by the GIMLi development team                    *
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

#pragma once

#include "gimli.h"
#include "vector.h"
#include "matrix.h"

namespace GIMLI{

class FEAFunction;
class ElementMatrixMap;

template < class ValueType > class DLLEXPORT ElementMatrix {
public:
    /*! If dof != 0 then scalar field approximation is to be supposed.
    For vector field solution give a dof, means be the number of nodes of the current mesh. */
    ElementMatrix(Index dof=0);

    ~ElementMatrix() {}

    /*! Assignment operator.*/
    ElementMatrix < ValueType > & operator = (const ElementMatrix < ValueType > & E) {
        std::cout << "ElementMatrix::operator = (" << std::endl;
        THROW_TO_IMPL
        if (this != & E){
//             this->resize(E.size());
//             for (uint i = 0; i < E.size(); i ++) mat_[i] = E.row(i);
//             _ids = E.idx();
        } return *this;
    }

    const Vector< ValueType > & operator[](Index row) const;

    inline const ValueType & operator()(Index i, Index j) const { 
        return mat_(i,j); 
    }
    void resize(Index rows, Index cols=0);

    ElementMatrix < ValueType > & operator += (const ElementMatrix < ValueType > & E);

    #define DEFINE_ELEMENTMATRIX_UNARY_MOD_OPERATOR__(OP) \
        ElementMatrix < ValueType > & operator OP##= (ValueType val);

        DEFINE_ELEMENTMATRIX_UNARY_MOD_OPERATOR__(+)
        DEFINE_ELEMENTMATRIX_UNARY_MOD_OPERATOR__(-)
        DEFINE_ELEMENTMATRIX_UNARY_MOD_OPERATOR__(/)
        DEFINE_ELEMENTMATRIX_UNARY_MOD_OPERATOR__(*)

    #undef DEFINE_ELEMENTMATRIX_UNARY_MOD_OPERATOR__

    inline Index size() const { return mat_.rows(); }
    inline Index rows() const { return mat_.rows(); }
    inline Index cols() const { return mat_.cols(); }

    inline const ValueType & getVal(Index i, Index j) const {
        return mat_(i,j); 
    }
    inline void setVal(Index i, Index j, const ValueType & v) {mat_(i,j) = v; }
    inline void addVal(Index i, Index j, const ValueType & v) {mat_(i,j) += v; }

    /*! Set data matrix. */
    inline void setMat(const SmallMatrix & m) { mat_ = m; }
    /*! Return data matrix. */
    inline SmallMatrix * pMat() { return & mat_; }
    /*! Return data for row i. */
    inline const SmallMatrix & mat() const { return mat_; }
    /*! Return data for row i. */

    inline const Vector< ValueType > & row(Index i) const { 
        return (*this)[i];
    }
    Vector< ValueType > row_RM(Index i) const;

    /*! Return data for col i. */
    Vector< ValueType > col(Index i) const;

    /*! Fill the node ids with a number of coefficents.
    For vector field approximation give field dimension 2 or 3.
    Please note that you need to give the number of nodes to the ElementMatrix constructor.
    */
    void fillIds(const MeshEntity & ent, Index nC=1);

    /*! Set all node indices for row and columns. Can be unsymmetric.*/
    inline void setIds(const IndexArray & idsR, const IndexArray & idsC) {
        _idsR = idsR; _idsC = idsC; _ids = idsR;
    }

    /*! Set all node indices.*/
    inline void setIds(const IndexArray & ids) {
        _idsR = ids; _idsC = ids; _ids = ids;
    }
    /*! Return all node indices.*/
    inline const IndexArray & ids() const { return _ids; }
    /*! Return all row node indices.*/
    inline const IndexArray & rowIDs() const { return _idsR; }
    /*! Return all column node indices.*/
    inline const IndexArray & colIDs() const { return _idsC; }
    /*! Return the node index for node i.*/
    inline const Index idx(Index i) const {
        return _ids[i]; }

    /*! Fill this element matrix with int_boundary C * u */
    ElementMatrix < ValueType > & u(const MeshEntity & ent
                                    // const Matrix< ValueType > & C
                                    );

    /*! Fill this element matrix with int_domain C * u * u */
    ElementMatrix < ValueType > & u2(const MeshEntity & ent
                                    //  const Matrix< ValueType > & C
                                     );

    /*! Get integration weights and points for the entity. */
    void findWeightsAndPoints(const MeshEntity & ent,
                const RVector * &w, const PosVector * &x, int order);


    /*! Return the stress matrix for this entity.*/
    Vector < ValueType > stress(const MeshEntity & ent,
                                const Matrix< ValueType > & C,
                                const RVector & u, bool voigtNotation=false);

    /*! Return gradient base for the last entity, i.e., integrate over gradient u over the cell. */
    const Matrix < ValueType > & gradientBase() const { return this->_grad;}

    /*! Fill Element Gradients Matrix for all integration points. */
    void fillGradientBase(const MeshEntity & ent,
                          const RVector & w,
                          const PosVector & x,
                          Index nC,
                          bool voigtNotation);

    /*! Fill this element matrix with int_domain C * grad u*/
    ElementMatrix < ValueType > & gradU(const Cell & cell,
                                        Index nC,
                                        bool voigtNotation=false);

    /*! Fill this element matrix with int_domain C * grad u */
    ElementMatrix < ValueType > & gradU(const MeshEntity & ent,
                                        const RVector & w,
                                        const PosVector & x,
                                        Index nC,
                                        bool voigtNotation=false);


    /*! Fill this element matrix with int_domain C * grad u * grad u.
    For scalar field approximation define C.size() = (1x1) isotropic or anisotropic
    C.size() = (cell.dim() x cell.dim()) parameter. For vector field approximation
    create the ElementMatrix with appropriate dof and C can be of size = (1x1)
    for isotropic, (cell.dim() x cell.dim()) for anisotropic, or (3x3)
    for 2D elastic and (6x6) 3D elastic parameter.
    Notation for elastic parameters Kelvin notation as default and can be Voigt's notation if needed.
     */
    ElementMatrix < ValueType > & gradU2(const Cell & cell,
                                         const Matrix< ValueType > & C,
                                         bool voigtNotation=false);

    /*! Fill this element matrix with int_domain C * grad u * grad u.*/
    ElementMatrix < ValueType > & gradU2(const Cell & cell, ValueType c){
        Matrix < ValueType > C(1, 1);
        C[0][0] = c;
        return this->gradU2(cell, C, false);
    }

    /*! Fill this element matrix with int_domain C * grad u * grad u. */
    ElementMatrix < ValueType > & gradU2(const MeshEntity & ent,
                                         const Matrix< ValueType > & C,
                                         const RVector & w,
                                         const PosVector & x,
                                         bool voigtNotation=false);

    ElementMatrix < ValueType > & ux2uy2uz2(const Cell & cell,
                                            bool useCache=false);

    ElementMatrix < ValueType > & u(const MeshEntity & ent,
                                    const RVector & w,
                                    const PosVector & x,
                                    bool verbose=false);
    ElementMatrix < ValueType > & u2(const MeshEntity & ent,
                                     const RVector & w,
                                     const PosVector & x,
                                     bool verbose=false);
    ElementMatrix < ValueType > & ux2(const MeshEntity & ent,
                                      const RVector & w,
                                      const PosVector & x,
                                      bool verbose=false);
    ElementMatrix < ValueType > & ux2uy2(const MeshEntity & ent,
                                         const RVector & w,
                                         const PosVector & x,
                                         bool verbose=false);
    ElementMatrix < ValueType > & ux2uy2uz2(const MeshEntity & ent,
                                            const RVector & w,
                                            const PosVector & x,
                                            bool verbose=false);

    ElementMatrix < double > & dudi(const MeshEntity & ent,
                                  const RVector & w,
                                  const PosVector & x,
                                  Index i, bool verbose=false);

    ElementMatrix < double > & ux(const MeshEntity & ent,
                                  const RVector & w,
                                  const PosVector & x,
                                  bool verbose=false){
        return dudi(ent, w, x, 0, verbose);
    }
    ElementMatrix < double > & uy(const MeshEntity & ent,
                                  const RVector & w,
                                  const PosVector & x,
                                  bool verbose=false){
        return dudi(ent, w, x, 1, verbose);
    }
    ElementMatrix < double > & uz(const MeshEntity & ent,
                                  const RVector & w,
                                  const PosVector & x,
                                  bool verbose=false){
        return dudi(ent, w, x, 2, verbose);
    }

    Vector < ValueType > mult(const Vector < ValueType > & v){
        Vector < ValueType > ret(this->size());
        this->mult(v, ret);
        return ret;
    }

    /*! Return S * a */
    void mult(const Vector < ValueType > & a, Vector < ValueType > & ret){
    #if USE_EIGEN3
    __MS("Efficiency warning")
    #endif 
        ASSERT_EQUAL(size(), ret.size())
        for (Index i = 0; i < size(); i ++) {
            for (Index j = 0; j < size(); j ++) {
                ret[i] += mat_(i,j) * a[_ids[j]];
            }
        }
    }
    /*! Return (S * a) * b */
    ValueType mult(const Vector < ValueType > & a,
                   const Vector < ValueType > & b){
    #if USE_EIGEN3
    __MS("Efficiency warning")
    #endif 
        ValueType ret = 0;
        for (Index i = 0; i < size(); i ++) {
            ValueType t = 0;
            for (Index j = 0; j < size(); j ++) {
                t += mat_(i,j) * a[_ids[j]];
            }
            ret += t * b[_ids[i]];
        }
        return ret;
    }

    /*! Return (S * (a-b)) * (m-n) TODO Check if its the same like mult(a-b, m-n)*/
    template < class Val > Val mult_(const Vector < Val > & a,
                                     const Vector < Val > & b,
                                     const Vector < Val > & m,
                                     const Vector < Val > & n){
    #if USE_EIGEN3
    __MS("Efficiency warning")
    #endif 
        Val ret = 0;
        for (Index i = 0; i < size(); i ++) {
            Val t = 0;
            for (Index j = 0; j < size(); j ++) {
                t += mat_(i,j) * (a[_ids[j]]-b[_ids[j]]);
            }
            ret += t * (m[_ids[i]]-n[_ids[i]]);
        }
        return ret;
    }

    double mult(const RVector & a, const RVector & b,
                const RVector & m, const RVector & n){
        return mult_(a, b, m, n);
    }
    Complex mult(const CVector & a, const CVector & b,
                 const CVector & m, const CVector & n){
        return mult_(a, b, m, n);
    }
    //***********************************************************************//
    //***********************************************************************//
    //***********************************************************************//
    //** new interface starts here **//
    //***********************************************************************//
    //***********************************************************************//
    //***********************************************************************//
    ElementMatrix(Index nCoeff, Index dofPerCoeff, Index dofOffset);

    ElementMatrix(const ElementMatrix < ValueType > & E);

    ElementMatrix(const ElementMatrix < ValueType > & E, bool withMat);

    void copyFrom(const ElementMatrix < ValueType > & E, bool withMat=true);

    void init(Index nCoeff, Index dofPerCoeff, Index dofOffset);

    /*! Fill this ElementMatrix with value (u for scalar, v for vector values) basis. Cache the matrix in entity*/
    ElementMatrix < ValueType > & pot(const MeshEntity & ent, Index order,
                                      bool sum,
                                      Index nCoeff, Index dof, Index dofOffset);

    /*! Fill this ElementMatrix with value (u for scalar, v for vector values) basis.*/
    ElementMatrix < ValueType > & pot(const MeshEntity & ent, Index order,
                                      bool sum=false);

    /*! Fill this ElementMatrix with value (u for scalar, v for vector values) basis. Cache the matrix in entity*/
    ElementMatrix < ValueType > & grad(const MeshEntity & ent, Index order,
                                       bool elastic, bool sum, bool div,
                                      Index nCoeff, Index dof, Index dofOffset, bool kelvin=false
                                      );

    /*! Fill this ElementMatrix with gradient of ent.*/
    ElementMatrix < ValueType > & grad(const MeshEntity & ent, Index order,
                                       bool elastic=false, bool sum=false,
                                       bool div=false, bool kelvin=false);

    ElementMatrix < ValueType > & identity(const MeshEntity & ent, Index order, 
                                           Index nCoeff, Index dofPerCoeff, Index dofOffset);


    /*! Add B to this ElementMatrix depending on requested dimension.
        Usual needed for expression (A+B)*u(dim==1) or (A+B)*v(dim!=1)
        for dim == 1 (A + B) ## A or B is grad and need to be summed (div)
        for dim == 0 (A + B) ## A or B is grad and need add per dimension
    */
    ElementMatrix < ValueType > & add(const ElementMatrix< double > & B, 
                                      Index dim=0, double b=1.0);

    /*! Integrate, i.e., sum over quadrature matrices.*/
    void integrate() const;

    /*! Set submatrices. matx of (nWeight, shape(mat.T))*/
    void setMatXI(Index i, const SmallMatrix & mat);

    /*! PG temp hack .. Convert RMatrix into EigenMatrix for pg. */
    void setMatXI_RM(Index i, const RMatrix & mat);

    /*! Set data matrix. */
    void setMat_RM(const RMatrix & m);
        
    /*! Return copy of mat */
    RMatrix mat_RM() const;


    /*! Return reference to all matrices per quadrature point.*/
    const std::vector < SmallMatrix > & matX() const { return _matX; }

    std::vector < SmallMatrix > * pMatX() { return &_matX; }

    /*! Set const reference to the current entity.*/
    void setEntity(const MeshEntity & ent) { _ent = &ent; }


    // /*! Return const reference to the last active entity.*/
    // const MeshEntity & entity() const { ASSERT_PTR(_ent); return *_ent; }

    /*! Return const reference to the last active entity.*/
    const MeshEntity * entity() const { return _ent; }

    // /*! Return const reference to the last active entity.*/
    // MeshEntity & rEntity() const { return (*const_cast< MeshEntity *>(_ent)); }

    /*! Return const reference to quadrature points.*/
    void setX(const PosVector & p);

    /*! Return const reference to quadrature points.*/
    const PosVector * x() const { return _x; }
    // /*! Return const reference to quadrature points.*/
    // const PosVector & x() const { ASSERT_PTR(_x); return *_x; }

    void setW(const RVector & w);

    /*! Return const reference to quadrature weights.*/
    const RVector * w() const { return _w; }
    // /*! Return const reference to quadrature weights.*/
    // const RVector & w() const { ASSERT_PTR(_w); return *_w; }

    Index order() const { return _order; }
    Index nCoeff() const { return _nCoeff; }
    Index dofPerCoeff() const { return _dofPerCoeff; }
    Index dofOffset() const { return _dofOffset; }

    void setDiv(bool div){ _div = div;}
    bool isDiv() const { return _div;}

    bool isIntegrated() const { return _integrated; }
    void integrated(bool i) { _integrated = i; }

    bool valid() const { return _valid; }
    void setValid(bool v) { _valid = v; }

    bool elastic() const { return _elastic;}

    bool oldStyle() const { return !this->_newStyle; }

    #define DEFINE_INTEGRATOR(A_TYPE) \
        /*! Integrate linear form: r = \int_entity this * f \d entity \
        with r = RVector(final form) and f = A_TYPE */ \
        void integrate(A_TYPE f, RVector & r, double scale) const; \
        
    DEFINE_INTEGRATOR(double)   // const scalar
    DEFINE_INTEGRATOR(const RMatrix &)  // const Matrix
    DEFINE_INTEGRATOR(const RVector &)  // scalar for each quadr
    DEFINE_INTEGRATOR(const Pos &)      // const vector //!calling order!
    DEFINE_INTEGRATOR(const PosVector &)  // vector for each quadr
    DEFINE_INTEGRATOR(const std::vector< RMatrix > &) // matrix for each quadrs
    DEFINE_INTEGRATOR(const FEAFunction &) // matrix for each quadrs
    
    #undef DEFINE_INTEGRATOR

    #define DEFINE_INTEGRATOR(A_TYPE) \
        /*! Integrate bilinear form A = \int_mesh this * f * R \d entity \
        with A = SparseMatrix(final form, final form) and f = A_TYPE */ \
        void integrate(const ElementMatrix < double > & R, \
                       A_TYPE f, SparseMatrixBase & A, double scale) const; \
        
    DEFINE_INTEGRATOR(double)   // const scalar
    DEFINE_INTEGRATOR(const RMatrix &)  // const Matrix
    DEFINE_INTEGRATOR(const RVector &)  // scalar for each quadr
    DEFINE_INTEGRATOR(const std::vector< RMatrix > &) // matrix for each quadrs
    DEFINE_INTEGRATOR(const FEAFunction &) // matrix for each quadrs
    
    #undef DEFINE_INTEGRATOR

protected:
    mutable SmallMatrix mat_;
    IndexArray _ids;
    IndexArray _idsC;
    IndexArray _idsR;

    std::map< uint, RVector > uCache_;
    std::map< uint, Matrix < ValueType > > u2Cache_;

    std::vector< Matrix < ValueType > > _B;
    Matrix < ValueType > _grad;

    // number of single dof
    Index _nDof;
    // number of coefficients: 1, 2, 3 for scalar(dim), 1, 3, 6 for vector(dim)
    // Index _nC;

    RMatrix dNdr_;
    RMatrix dNds_;
    RMatrix dNdt_;

    RMatrix dNdx_; // (nRules, nVerts)
    RMatrix dNdy_; // (nRules, nVerts)
    RMatrix dNdz_; // (nRules, nVerts)

    RMatrix _abaTmp; // temp workspace

    //** new interface starts here **//
    // const Mesh * _mesh;
    Index _order;
    Index _nCoeff;
    Index _dofPerCoeff;
    Index _dofOffset;

    const MeshEntity * _ent;
    const RVector * _w;
    const PosVector * _x;

    // matrices per quadrature point
    std::vector < SmallMatrix > _matX;

    bool _newStyle;
    bool _div;
    bool _valid;
    bool _elastic;
    mutable bool _integrated;

private:
    /*! No copy operator. */
};

template < > DLLEXPORT
ElementMatrix < double >::ElementMatrix(Index dof);

template < > DLLEXPORT
ElementMatrix < double >::ElementMatrix(const ElementMatrix < double > &);

template < > DLLEXPORT 
void ElementMatrix < double >::fillIds(const MeshEntity & ent, Index nC);
template < > DLLEXPORT
void ElementMatrix < double >::resize(Index rows, Index cols);


template < > DLLEXPORT
void ElementMatrix < double >::copyFrom(const ElementMatrix < double > & E,
                                        bool withMat);

template < > DLLEXPORT
void ElementMatrix < double >::init(Index nCoeff, Index dofPerCoeff,
                                    Index dofOffset);

#define DEFINE_ELEMENTMATRIX_UNARY_MOD_OPERATOR__(OP) \
template < > DLLEXPORT ElementMatrix < double > & \
ElementMatrix < double >::operator OP##= (double val); \

DEFINE_ELEMENTMATRIX_UNARY_MOD_OPERATOR__(+)
DEFINE_ELEMENTMATRIX_UNARY_MOD_OPERATOR__(-)
DEFINE_ELEMENTMATRIX_UNARY_MOD_OPERATOR__(/)
DEFINE_ELEMENTMATRIX_UNARY_MOD_OPERATOR__(*)

#undef DEFINE_ELEMENTMATRIX_UNARY_MOD_OPERATOR__


template < > DLLEXPORT ElementMatrix < double > & 
ElementMatrix < double >::operator += (const ElementMatrix < double > & E);


template < > DLLEXPORT ElementMatrix < double > &
ElementMatrix < double >::add(const ElementMatrix < double > & B, 
                              Index dim, double b);


DLLEXPORT void dot(const ElementMatrix < double > & A,
                   const ElementMatrix < double > & B,
                   ElementMatrix < double > & ret);

DLLEXPORT const ElementMatrix < double > dot(const ElementMatrix < double > & A,
                                            const ElementMatrix < double > & B);


// // declare this before mult(.., pos, C) to avoid ambiguities
// /*! scalar per quadrature point */
// DLLEXPORT void mult(const ElementMatrix < double > & A, const RVector & b,
//                     ElementMatrix < double > & C);
// /*! vector per quadrature point */
// DLLEXPORT void mult(const ElementMatrix < double > & A, const PosVector & b,
//                     ElementMatrix < double > & C);
// /*! Matrix per quadrature point */
// DLLEXPORT void mult(const ElementMatrix < double > & A,
//                     const std::vector < RMatrix > & b,
//                     ElementMatrix < double > & C);


#define DEFINE_DOT_MULT(A_TYPE) \
DLLEXPORT void dot(const ElementMatrix < double > & A, \
                   const ElementMatrix < double > & B, \
                   A_TYPE c, ElementMatrix < double > & ret); \
DLLEXPORT void mult(const ElementMatrix < double > & A, \
                    A_TYPE b, ElementMatrix < double > & C); \
DLLEXPORT ElementMatrix < double > dot(const ElementMatrix < double > & A, \
                                       const ElementMatrix < double > & B, \
                                       A_TYPE c); \
DLLEXPORT ElementMatrix < double > mult(const ElementMatrix < double > & A, \
                                        A_TYPE b); \
template < > DLLEXPORT \
void ElementMatrix < double >::integrate(A_TYPE f, \
                                         RVector & R, double scale) const; \

DEFINE_DOT_MULT(double)
DEFINE_DOT_MULT(const RVector &)
DEFINE_DOT_MULT(const Pos &) // check Pos before RVector
DEFINE_DOT_MULT(const PosVector &)
DEFINE_DOT_MULT(const RMatrix &)
DEFINE_DOT_MULT(const std::vector < RMatrix > &)
DEFINE_DOT_MULT(const FEAFunction &)
#undef DEFINE_DOT_MULT


#define DEFINE_INTEGRATE(A_TYPE) \
template < > DLLEXPORT \
void ElementMatrix < double >::integrate(const ElementMatrix < double > & R, \
                                         A_TYPE f, \
                                    SparseMatrixBase & A, double scale) const; \

DEFINE_INTEGRATE(double)
DEFINE_INTEGRATE(const RMatrix &)
DEFINE_INTEGRATE(const RVector &)
DEFINE_INTEGRATE(const std::vector < RMatrix > &)
DEFINE_INTEGRATE(const FEAFunction &)

#undef DEFINE_INTEGRATE

/*!Evaluate scalars per cell.*/
DLLEXPORT void evaluateQuadraturePoints(const MeshEntity & ent,
                                        const PosVector & x,
                                        const FEAFunction & f, RVector & ret);
/*!Evaluate vectors per cell.*/
DLLEXPORT void evaluateQuadraturePoints(const MeshEntity & ent,
                                        const PosVector & x,
                                        const FEAFunction & f, PosVector & ret);
/*!Evaluate matrices per cell.*/
DLLEXPORT void evaluateQuadraturePoints(const MeshEntity & ent,
                                        const PosVector & x,
                                        const FEAFunction & f,
                                        std::vector < RMatrix > & ret);
/*!Evaluate scalar for each cell.*/
DLLEXPORT void evaluateQuadraturePoints(const Mesh & mesh, Index order,
                                        const FEAFunction & f,
                                        std::vector< RVector > & ret);
/*!Evaluate vectors for each cell.*/
DLLEXPORT void evaluateQuadraturePoints(const Mesh & mesh, Index order,
                                        const FEAFunction & f,
                                        std::vector< PosVector > & ret);
/*!Evaluate matrices for each cell.*/
DLLEXPORT void evaluateQuadraturePoints(const Mesh & mesh, Index order,
                                        const FEAFunction & f,
                                        std::vector< std::vector< RMatrix > > & ret);

/*! Return symmetrized copy of A as 0.5*(A + A.T). Only for gradients without Voigt or Kelvin notation. */
DLLEXPORT ElementMatrix < double > sym(const ElementMatrix < double > & A);

/*! copy symmetrized A as 0.5*(A + A.T) into B.*/
DLLEXPORT void sym(const ElementMatrix < double > & A, ElementMatrix < double > & B);


#define DEFINE_CREATE_FORCE_VECTOR(A_TYPE) \
DLLEXPORT void createForceVector(const Mesh & mesh, Index order, \
                                 RVector & ret, A_TYPE a, \
                                 Index nCoeff, Index dofOffset); \
DLLEXPORT void createMassMatrix(const Mesh & mesh, Index order, \
                                RSparseMapMatrix & ret, A_TYPE a, \
                                Index nCoeff, Index dofOffset); \
DLLEXPORT void createStiffnessMatrix(const Mesh & mesh, Index order, \
                                     RSparseMapMatrix & ret, A_TYPE a, \
                                     Index nCoeff, Index dofOffset, \
                                     bool elastic=false, bool kelvin=false); \

DEFINE_CREATE_FORCE_VECTOR(double)             // const scalar for all cells
DEFINE_CREATE_FORCE_VECTOR(const Pos &)   // const vector for all cells
DEFINE_CREATE_FORCE_VECTOR(const RVector &)    // scalar for each cell
DEFINE_CREATE_FORCE_VECTOR(const PosVector &)   // vector for each cell
DEFINE_CREATE_FORCE_VECTOR(const RMatrix &)    // const matrix for all cells
// matrix for each cell
DEFINE_CREATE_FORCE_VECTOR(const std::vector< RVector > &)
// vector at each quadrature point for each cell
DEFINE_CREATE_FORCE_VECTOR(const std::vector< PosVector > &)
// matrix at each quadrature point for each cell
DEFINE_CREATE_FORCE_VECTOR(const std::vector< RMatrix > &)
// scalar at each quadrature point for each cell
DEFINE_CREATE_FORCE_VECTOR(const std::vector< std::vector < RMatrix > > &)
// generic function for each point
DEFINE_CREATE_FORCE_VECTOR(const FEAFunction &)

#undef DEFINE_CREATE_FORCE_VECTOR

/*!Interface to function q=f(p, ent) with q, p = Pos() in R1, R2, R 3
and ent assiated mesh entity.*/
class DLLEXPORT FEAFunction {
public:
    FEAFunction(Index valueSize)
        : _valueSize(valueSize), _evalOnCellCenter(false){ }

    virtual ~FEAFunction() { }

    virtual double evalR1(const Pos & arg, const MeshEntity * ent=0) const{
        log(Warning, "FEAFunction.evalR1 should be overloaded.");
        return 0.0;
    }
    virtual Pos evalR3(const Pos & arg, const MeshEntity * ent=0) const{
        log(Warning, "FEAFunction.evalR3 should be overloaded.");
        return Pos(0.0, 0.0, 0.0);
    }
    virtual RMatrix evalRM(const Pos & arg, const MeshEntity * ent=0) const{
        log(Warning, "FEAFunction.evalRM should be overloaded.");
        return RMatrix(0, 0);
    }

    /*!Return expected value size for evaluation */
    Index valueSize() const { return _valueSize; }

    /*!Set expected value size for evaluation */
    void setValueSize(Index s) { _valueSize = s; }

    /*!Return if the function is marked for value evaluation on cell centers or quadrature points. */
    bool evalOnCellCenter() const { return _evalOnCellCenter; }

    /*!Mark the function to evaluate on cell centers instead of quadrature points. */
    void setEvalOnCellCenter(bool e) { _evalOnCellCenter = e; }

protected:
    Index _valueSize;
    bool _evalOnCellCenter;
};

template < class ValueType > std::ostream & operator << (std::ostream & str,
                                                         const ElementMatrix< ValueType > & e);

template < > DLLEXPORT std::ostream & operator << (std::ostream & str,
                                            const ElementMatrix< double > & e);

} // namespace GIMLI{