/*
   This file is part of HPDDM.

   Author(s): Pierre Jolivet <jolivet@ann.jussieu.fr>
        Date: 2012-10-07

   Copyright (C) 2011-2014 Université de Grenoble

   HPDDM is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published
   by the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   HPDDM is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with HPDDM.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MKL_PARDISO_
#define _MKL_PARDISO_

#ifdef DMKL_PARDISO
#include <mkl_cluster_sparse_solver.h>
#endif
#ifdef MKL_PARDISOSUB
#include <mkl_pardiso.h>
#endif

namespace HPDDM {
template<class>
struct prds {
};
template<>
struct prds<float> {
    static constexpr int SPD =  2;
    static constexpr int SYM = -2;
    static constexpr int UNS =  1;
};
template<>
struct prds<double> {
    static constexpr int SPD =  2;
    static constexpr int SYM = -2;
    static constexpr int UNS =  1;
};
template<>
struct prds<std::complex<float>> {
    static constexpr int SPD =  4;
    static constexpr int SYM = -4;
    static constexpr int UNS =  3;
};
template<>
struct prds<std::complex<double>> {
    static constexpr int SPD =  4;
    static constexpr int SYM = -4;
    static constexpr int UNS =  3;
};

#ifdef DMKL_PARDISO
#define COARSEOPERATOR HPDDM::MklPardiso
/* Class: MKL Pardiso
 *
 *  A class inheriting from <DMatrix> to use <MKL Pardiso>.
 *
 * Template Parameter:
 *    K              - Scalar type. */
template<class K>
class MklPardiso : public DMatrix {
    private:
        /* Variable: pt
         *  Internal data pointer. */
        void*      _pt[64];
        /* Variable: a
         *  Array of data. */
        K*              _C;
        /* Variable: I
         *  Array of row pointers. */
        int*            _I;
        /* Variable: J
         *  Array of column indices. */
        int*            _J;
        /* Variable: w
         *  Workspace array. */
        K*              _w;
        /* Variable: mtype
         *  Matrix type. */
        int         _mtype;
        /* Variable: iparm
         *  Array of parameters. */
        int     _iparm[64];
        /* Variable: comm
         *  MPI communicator. */
        int          _comm;
    protected:
        /* Variable: numbering
         *  0-based indexing. */
        static constexpr char _numbering = 'C';
    public:
        MklPardiso() : _pt(), _C(), _I(), _J(), _w(), _comm(-1) { }
        ~MklPardiso() {
            delete [] _w;
            int phase = -1;
            int error;
            K ddum;
            int idum;
            if(_comm != -1)
                CLUSTER_SPARSE_SOLVER(_pt, const_cast<int*>(&i__1), const_cast<int*>(&i__1), &_mtype, &phase, &(DMatrix::_n), &ddum, &idum, &idum, const_cast<int*>(&i__1), const_cast<int*>(&i__1), _iparm, const_cast<int*>(&i__0), &ddum, &ddum, const_cast<int*>(&_comm), &error);
            delete [] _I;
            delete [] _J;
            if(DMatrix::_communicator != MPI_COMM_NULL && DMatrix::_n == _iparm[41] - _iparm[40] + 1 && _mtype != prds<K>::SPD)
                delete [] _C;
        }
        /* Function: numfact
         *
         *  Initializes <MKL Pardiso::pt> and <MKL Pardiso::iparm>, and factorizes the supplied matrix.
         *
         * Template Parameter:
         *    S              - 'S'ymmetric or 'G'eneral factorization.
         *
         * Parameters:
         *    ncol           - Number of local rows.
         *    I              - Array of row pointers.
         *    loc2glob       - Lower and upper bounds of the local domain.
         *    J              - Array of column indices.
         *    C              - Array of data. */
        template<char S>
        inline void numfact(unsigned int ncol, int* I, int* loc2glob, int* J, K* C) {
            _I = I;
            _J = J;
            _C = C;
            if(S == 'S')
                _mtype = prds<K>::SPD;
            else
                _mtype = prds<K>::UNS;
            int phase, error;
            K ddum;
            std::fill(_iparm, _iparm + 64, 0);
            _iparm[0] = 1;
#ifdef _OPENMP
            _iparm[1] = omp_get_num_threads() > 1 ? 3 : 2;
#else
            _iparm[1] = 2;
#endif
            _iparm[2] = 1;
            _iparm[5] = 1;
            _iparm[9] = 13;
            _iparm[10] = 1;
            _iparm[17] = -1;
            _iparm[18] = -1;
            _iparm[27] = std::is_same<double, typename Wrapper<K>::ul_type>::value ? 0 : 1;
            _iparm[34] = (_numbering == 'C');
            _iparm[39] = DMatrix::_distribution == DMatrix::NON_DISTRIBUTED ? 1 : 2;
            _iparm[40] = loc2glob[0];
            _iparm[41] = loc2glob[1];
            delete [] loc2glob;
            phase = 12;
            CLUSTER_SPARSE_SOLVER(_pt, const_cast<int*>(&i__1), const_cast<int*>(&i__1), &_mtype, &phase, &(DMatrix::_n), C, _I, _J, const_cast<int*>(&i__1), const_cast<int*>(&i__1), _iparm, const_cast<int*>(&i__1), &ddum, &ddum, const_cast<int*>(&_comm), &error);
            if(DMatrix::_distribution == DMatrix::NON_DISTRIBUTED && DMatrix::_rank == 0)
                _w = new K[DMatrix::_n];
            else
                _w = new K[_iparm[41] - _iparm[40] + 1];
        }
        /* Function: solve
         *
         *  Solves the system in-place.
         *
         * Template Parameter:
         *    D              - Distribution of right-hand sides and solution vectors.
         *
         * Parameters:
         *    rhs            - Input right-hand side, solution vector is stored in-place.
         *    fuse           - Number of fused reductions (optional). */
        template<DMatrix::Distribution D>
        inline void solve(K* rhs, const unsigned short& fuse = 0) {
            int error;
            int phase = 33;
            K ddum;
            CLUSTER_SPARSE_SOLVER(_pt, const_cast<int*>(&i__1), const_cast<int*>(&i__1), &_mtype, &phase, &(DMatrix::_n), _C, _I, _J, const_cast<int*>(&i__1), const_cast<int*>(&i__1), _iparm, const_cast<int*>(&i__0), rhs, _w, const_cast<int*>(&_comm), &error);
        }
        /* Function: initialize
         *
         *  Initializes <MKL Pardiso::comm>,  <DMatrix::rank>, and <DMatrix::distribution>.
         *
         * Parameter:
         *    parm           - Vector of parameters. */
        template<class Container>
        inline void initialize(Container& parm) {
            if(DMatrix::_communicator != MPI_COMM_NULL) {
                _comm = MPI_Comm_c2f((DMatrix::_communicator));
                MPI_Comm_rank(DMatrix::_communicator, &(DMatrix::_rank));
            }
            if(parm[TOPOLOGY] == 1)
                parm[TOPOLOGY] = 0;
            if(parm[DISTRIBUTION] != DMatrix::DISTRIBUTED_SOL_AND_RHS && parm[DISTRIBUTION] != DMatrix::NON_DISTRIBUTED) {
                if(DMatrix::_communicator != MPI_COMM_NULL && DMatrix::_rank == 0)
                    std::cout << "WARNING -- only distributed solution and RHS and non distributed solution and RHS supported by the PARDISO interface, forcing the distribution to DISTRIBUTED_SOL_AND_RHS" << std::endl;
                DMatrix::_distribution = DMatrix::DISTRIBUTED_SOL_AND_RHS;
                parm[DISTRIBUTION] = DMatrix::DISTRIBUTED_SOL_AND_RHS;
            }
            else
                DMatrix::_distribution = static_cast<DMatrix::Distribution>(parm[DISTRIBUTION]);
        }
};
#endif // DMKL_PARDISO

#ifdef MKL_PARDISOSUB
#define SUBDOMAIN HPDDM::MklPardisoSub
template<class K>
class MklPardisoSub {
    private:
        mutable void*  _pt[64];
        K*                  _C;
        int*                _I;
        int*                _J;
        K*                  _w;
        int             _mtype;
        mutable int _iparm[64];
        int                 _n;
    public:
        MklPardisoSub() : _pt(), _C(), _I(), _J(), _w() { }
        MklPardisoSub(const MklPardisoSub&) = delete;
        ~MklPardisoSub() {
            delete [] _w;
            int phase = -1;
            int error;
            int idum;
            K ddum;
            _n = 1;
            PARDISO(_pt, const_cast<int*>(&i__1), const_cast<int*>(&i__1), &_mtype, &phase, &_n, &ddum, &idum, &idum, const_cast<int*>(&i__1), const_cast<int*>(&i__1), _iparm, const_cast<int*>(&i__0), &ddum, &ddum, &error);
            if(_mtype == prds<K>::SPD || _mtype == prds<K>::SYM) {
                delete [] _I;
                delete [] _J;
            }
            if(_mtype == prds<K>::SYM)
                delete [] _C;
        }
        inline void numfact(MatrixCSR<K>* const& A, bool detection = false, K* const& schur = nullptr) {
            int* perm = nullptr;
            int phase, error;
            K ddum;
            if(!_w) {
                _n = A->_n;
                std::fill(_iparm, _iparm + 64, 0);
                _iparm[0] = 1;
#ifdef _OPENMP
                _iparm[1] = omp_get_num_threads() > 1 ? 3 : 2;
#else
                _iparm[1] = 2;
#endif
                _iparm[2] = 1;
                _iparm[9] = 13;
                _iparm[10] = 1;
                _iparm[17] = -1;
                _iparm[18] = -1;
                _iparm[27] = std::is_same<double, typename Wrapper<K>::ul_type>::value ? 0 : 1;
                _iparm[34] = 1;
                phase = 12;
                if(A->_sym) {
                    _I = new int[_n + 1];
                    _J = new int[A->_nnz];
                    _C = new K[A->_nnz];
                }
                else
                    _mtype = prds<K>::UNS;
                if(schur != nullptr) {
                    _iparm[35] = 2;
                    perm = new int[_n];
                    std::fill(perm, perm + static_cast<int>(std::real(schur[1])), 0);
                    std::fill(perm + static_cast<int>(std::real(schur[1])), perm + _n, 1);
                }
                _w = new K[_n];
            }
            else {
                if(_mtype == prds<K>::SPD)
                    _C = new K[A->_nnz];
                phase = 22;
            }
            if(A->_sym) {
                _mtype = detection ? prds<K>::SYM : prds<K>::SPD;
                Wrapper<K>::template csrcsc<'C'>(&_n, A->_a, A->_ja, A->_ia, _C, _J, _I);
            }
            else {
                _I = A->_ia;
                _J = A->_ja;
                _C = A->_a;
            }
            PARDISO(_pt, const_cast<int*>(&i__1), const_cast<int*>(&i__1), &_mtype, &phase,
                    const_cast<int*>(&_n), _C, _I, _J, perm, const_cast<int*>(&i__1), _iparm, const_cast<int*>(&i__0), &ddum, schur, &error);
            delete [] perm;
            if(_mtype == prds<K>::SPD)
                delete [] _C;
        }
        inline void solve(K* x) const {
            int error;
            int phase = 33;
            _iparm[5] = 1;
            PARDISO(_pt, const_cast<int*>(&i__1), const_cast<int*>(&i__1), const_cast<int*>(&_mtype), &phase, const_cast<int*>(&_n), _C, _I, _J, const_cast<int*>(&i__1), const_cast<int*>(&i__1), _iparm, const_cast<int*>(&i__0), x, const_cast<K*>(_w), &error);
        }
        inline void solve(K* const x, const unsigned short& n) const {
            int error;
            int phase = 33;
            int nrhs = n;
            _iparm[5] = 1;
            K* w = new K[_n * n];
            PARDISO(_pt, const_cast<int*>(&i__1), const_cast<int*>(&i__1), const_cast<int*>(&_mtype), &phase, const_cast<int*>(&_n), _C, _I, _J, const_cast<int*>(&i__1), &nrhs, _iparm, const_cast<int*>(&i__0), x, w, &error);
            delete [] w;
        }
        inline void solve(const K* const b, K* const x) const {
            int error;
            int phase = 33;
            _iparm[5] = 0;
            PARDISO(_pt, const_cast<int*>(&i__1), const_cast<int*>(&i__1), const_cast<int*>(&_mtype), &phase, const_cast<int*>(&_n), _C, _I, _J, const_cast<int*>(&i__1), const_cast<int*>(&i__1), _iparm, const_cast<int*>(&i__0), const_cast<K*>(b), x, &error);
        }
};
#endif // MKL_PARDISOSUB
} // HPDDM
#endif // _MKL_PARDISO_
