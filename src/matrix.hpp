/*
   This file is part of HPDDM.

   Author(s): Pierre Jolivet <jolivet@ann.jussieu.fr>
        Date: 2013-03-12

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

#ifndef _MATRIX_
#define _MATRIX_

#include <iostream>
#include <iomanip>

namespace HPDDM {
/* Class: MatrixCSR
 *
 *  A class for storing sparse matrices in Compressed Sparse Row format.
 *
 * Template Parameters:
 *    K              - Scalar type.
 *    N              - 0- or 1-based indexing. */
template<class K, char N = 'C'>
class MatrixCSR {
    static_assert(N == 'F' || N == 'C', "Unknown numbering");
    private:
        /* Variable: free
         *  Sentinel value for knowing if the pointers <MatrixCSR::a>, <MatrixCSR::ia>, <MatrixCSR::ja> have to be freed. */
        bool _free;
    public:
        /* Variable: a
         *  Array of data. */
        K*      _a;
        /* Variable: ia
         *  Array of row pointers. */
        int*   _ia;
        /* Variable: ja
         *  Array of column indices. */
        int*   _ja;
        /* Variable: n
         *  Number of rows. */
        int     _n;
        /* Variable: m
         *  Number of columns. */
        int     _m;
        /* Variable: nnz
         *  Number of nonzero entries. */
        int   _nnz;
        /* Variable: sym
         *  Symmetry of the matrix. */
        bool  _sym;
        MatrixCSR() : _free(true), _a(), _ia(), _ja(), _n(0), _m(0), _nnz(0), _sym(true) { }
        MatrixCSR(const int& n, const int& m, const bool& sym) : _free(true), _a(), _ia(new int[n + 1]), _ja(), _n(n), _m(m), _nnz(0),  _sym(sym) { }
        MatrixCSR(const int& n, const int& m, const int& nnz, const bool& sym) : _free(true), _a(new K[nnz]), _ia(new int[n + 1]), _ja(new int[nnz]), _n(n), _m(m), _nnz(nnz), _sym(sym) { }
        MatrixCSR(const int& n, const int& m, const int& nnz, K* const& a, int* const& ia, int* const& ja, const bool& sym, const bool& takeOwnership = false) : _free(takeOwnership), _a(a), _ia(ia), _ja(ja), _n(n), _m(m), _nnz(nnz), _sym(sym) { }
        ~MatrixCSR() {
            if(_free) {
                delete [] _a;
                delete [] _ia;
                delete [] _ja;
            }
        }
        /* Function: sameSparsity
         *
         *  Checks whether the input matrix can be modified to have the same sparsity pattern as the calling object.
         *
         * Parameter:
         *    A              - Input matrix. */
        inline bool sameSparsity(MatrixCSR<K>* const& A) const {
            if(A->_sym == _sym && A->_nnz >= _nnz) {
                if(A->_ia == _ia && A->_ja == _ja)
                    return true;
                else {
                    bool same = true;
                    K* a = new K[_nnz];
                    for(int i = 0; i < _n && same; ++i) {
                        for(int j = A->_ia[i], k = _ia[i]; j < A->_ia[i + 1]; ++j) {
                            while(k < _ia[i + 1] && _ja[k] < A->_ja[j])
                                a[k++] = K();
                            if(_ja[k] != A->_ja[j]) {
                                if(std::abs(A->_a[j]) > HPDDM_EPS)
                                    same = false;
                            }
                            else
                                a[k++] = A->_a[j];
                        }
                    }
                    if(same) {
                        A->_nnz = _nnz;
                        delete [] A->_ja;
                        delete [] A->_ia;
                        delete [] A->_a;
                        A->_a = a;
                        A->_ia = _ia;
                        A->_ja = _ja;
                        A->_free = false;
                    }
                    else
                        delete [] a;
                    return same;
                }
            }
            else
                return false;
        }
        /* Function: dump
         *  Outputs the matrix to an output stream. */
        std::ostream& dump(std::ostream& f) const {
            f << "# First line: n m (is symmetric) nnz indexing" << std::endl;
            f << "# For each nonzero coefficient: i j a_ij such that (i, j) \\in  {1, ..., n} x {1, ..., m}" << std::endl;
            f << _n << " " << _m << " " << _sym << "  " << _nnz << " " << N << std::endl;
            unsigned int k = _ia[0] - (N == 'F');
            int old = f.precision();
            for(unsigned int i = 0; i < _n; ++i) {
                unsigned int ke = _ia[i + 1] - (N == 'F');
                for( ; k < ke; ++k)
                    f << std::setw(9) << i + 1 << " " << std::setw(9) << _ja[k] + (N == 'C') << " " << std::setprecision(20) << _a[k] << std::endl;
            }
            f.precision(old);
            return f;
        }
};
} // HPDDM
#endif // _MATRIX_
