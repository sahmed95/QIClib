/*
 * QIC_lib (Quantum information and computation library)
 *
 * Copyright (c) 2015 - 2016  Titas Chanda (titas.chanda@gmail.com)
 *
 * This file is part of QIC_lib.
 *
 * QIC_lib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * QIC_lib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QIC_lib.  If not, see <http://www.gnu.org/licenses/>.
 */



namespace qic {

template< typename T1, typename TR =
          typename std::enable_if< is_arma_type_var<T1>::value,
                                   arma::Mat< eT<T1> >
                                   >::type >
inline
TR TrX(const T1& rho1,
       arma::uvec sys,
       arma::uvec dim
       ) {
  const auto& p = as_Mat(rho1);

  bool checkV = true;
  if ( p.n_cols == 1 )
    checkV = false;


#ifndef QIC_LIB_NO_DEBUG
  if ( p.n_elem == 0 )
    throw Exception("qic::TrX", Exception::type::ZERO_SIZE);

  if ( checkV )
    if ( p.n_rows != p.n_cols )
      throw Exception("qic::TrX",
                      Exception::type::MATRIX_NOT_SQUARE_OR_CVECTOR);

  if ( dim.n_elem == 0 || arma::any(dim == 0) )
    throw Exception("qic::TrX", Exception::type::INVALID_DIMS);

  if ( arma::prod(dim) != p.n_rows )
    throw Exception("qic::TrX", Exception::type::DIMS_MISMATCH_MATRIX);

  if ( dim.n_elem < sys.n_elem || arma::any(sys == 0)
       || arma::any(sys > dim.n_elem)
       || sys.n_elem != arma::find_unique(sys, false).eval().n_elem)
    throw Exception("qic::TrX", Exception::type::INVALID_SUBSYS);
#endif

  sys = arma::sort(sys);
  _internal::protect_subs::dim_collapse_sys(dim, sys);
  const arma::uword n = dim.n_elem;
  const arma::uword m = sys.n_elem;


  arma::uvec keep(n);
  for ( arma::uword run = 0 ; run < n ; ++run )
    keep.at(run) = run + 1;
  for ( arma::uword run = 0 ; run < m ; ++run )
    keep.shed_row(sys.at(run)-1-run);

  arma::uvec dimr(n-m);
  for ( arma::uword run = 0 ; run < (n-m) ; ++run )
    dimr.at(run) = dim(keep.at(run)-1);

  arma::uword dimtrace = prod(dim(sys-1));
  arma::uword dimkeep = p.n_rows/dimtrace;


  arma::uvec product = arma::ones<arma::uvec>(n);
  for ( arma::sword i = n-2 ; i >- 1 ; --i )
    product.at(i) = product.at(i+1) * dim.at(i+1);

  arma::uvec productr = arma::ones<arma::uvec>(n-m);
  for ( arma::sword i = n-m-2 ; i >- 1 ; --i )
    productr.at(i) = productr.at(i+1) * dimr.at(i+1);

  arma::Mat< eT<T1> > tr_p = arma::zeros< arma::Mat< eT<T1> >
                                          >(dimkeep, dimkeep);

  const arma::uword loop_no = 2 * n;
  arma::uword* loop_counter = new arma::uword[loop_no+1];
  arma::uword* MAX = new arma::uword[loop_no+1];

  for ( arma::uword i = 0 ; i < n ; ++i ) {
    MAX[i] = dim.at(i);

    if ( arma::any(sys == (i+1)) )
      MAX[i+n] = 1;
    else
      MAX[i+n] = dim.at(i);
  }
  MAX[loop_no] = 2;

  for ( arma::uword i = 0 ; i < loop_no+1 ; ++i )
    loop_counter[i] = 0;

  arma::uword p1 = 0;

  while ( loop_counter[loop_no] == 0 ) {
    arma::uword I(0), J(0), K(0), L(0), n_to_k(0);
    for ( arma::uword i = 0 ; i < n ; ++i ) {
      if ( arma::any(sys == i+1) ) {
        I += product.at(i)*loop_counter[i];
        J += product.at(i)*loop_counter[i];

      } else {
        I += product.at(i)*loop_counter[i];
        J += product.at(i)*loop_counter[i+n];
      }

      if ( arma::any(keep == i+1) ) {
        K += productr.at(n_to_k)*loop_counter[i];
        L += productr.at(n_to_k)*loop_counter[i+n];
        ++n_to_k;
      }
    }

    tr_p.at(K, L) += checkV ? p.at(I, J) : p.at(I) * std::conj(p.at(J));

    ++loop_counter[0];
    while ( loop_counter[p1] == MAX[p1] ) {
      loop_counter[p1] = 0;
      loop_counter[++p1]++;
      if ( loop_counter[p1] != MAX[p1] )
        p1 = 0;
    }
  }
  delete [] loop_counter;
  delete [] MAX;
  return tr_p;
}

template< typename T1, typename TR =
          typename std::enable_if< is_arma_type_var<T1>::value,
                                   arma::Mat< eT<T1> >
                                   >::type >
inline
TR TrX(const T1& rho1,
       arma::uvec sys,
       arma::uword dim = 2
       ) {
  const auto& p = as_Mat(rho1);

  bool checkV = true;
  if ( p.n_cols == 1 )
    checkV = false;

#ifndef QIC_LIB_NO_DEBUG
  if ( p.n_elem == 0 )
    throw Exception("qic::TrX", Exception::type::ZERO_SIZE);

  if ( checkV )
    if ( p.n_rows != p.n_cols )
      throw Exception("qic::TrX",
                      Exception::type::MATRIX_NOT_SQUARE_OR_CVECTOR);

  if ( dim == 0)
    throw Exception("qic::TrX", Exception::type::INVALID_DIMS);
#endif

  arma::uword n = static_cast<arma::uword>
      (std::llround(std::log(p.n_rows) / std::log(dim)));

  arma::uvec dim2 = dim*arma::ones<arma::uvec>(n);
  return TrX(p, std::move(sys), std::move(dim2));
}



}  // namespace qic
