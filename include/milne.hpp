#ifndef MILNE_H
#define MILNE_H

#include <Kokkos_Core.hpp>

namespace ccake{
namespace milne{

/// @brief A class representing a vector in D-dimensional space.
/// @tparam T The type of the vector components.
/// @tparam D The dimension of the vector.
template <class T, int D>
class Vector
{
private:
  T x[D];         ///< The array storing the components of the vector.
  int status = 1; ///< Stores if the vector is covariant or contravariant. Contravariant by default.

public:

   ///@brief Default constructor. Initializes all components to zero.
  KOKKOS_FUNCTION Vector<T,D>(){for (int i = 0; i < D; i++) x[i] = (T)0.0;}

   /// @brief Constructor with same initial value for all components
   /// @param x0 The initial value for all elements of the matrix
  KOKKOS_FUNCTION Vector<T,D>(T x0){for (int i = 0; i < D; i++) x[i] = x0;}

  /// @brief Constructor that initializes the components using an initializer list.
  /// @param x0 The initializer list containing the values for the components.
  KOKKOS_FUNCTION Vector<T,D>(std::initializer_list<T> x0){
    #ifdef DEBUG
    assert(x0.size() == D);
    #endif
    std::copy(x0.begin(), x0.end(), x);
  }

  /// @brief Overload of the call operator to access the components of the vector.
  /// @param i The index of the component to access.
  /// @return A reference to the component at the given index.
  KOKKOS_FUNCTION T& operator()(const int i){ return x[i]; };

  /// @brief Overload of the call operator to access the components of the vector.
  /// @param i The index of the component to access.
  /// @return The value of the component at the given index.
  KOKKOS_FUNCTION T  operator()(const int i) const { return x[i]; };

  /// @brief Assignment operator.
  /// @param other The vector to assign from.
  /// @return A reference to the assigned vector.
  KOKKOS_FUNCTION Vector<T,D>& operator=(const Vector<T,D>& other);

  /// @brief Assignment operator with type conversion.
  /// @param other The value to assign from.
  /// @tparam U The type to convert from.
  /// @return A reference to the assigned vector.
  template <class U> KOKKOS_FUNCTION
  Vector<T,D>& operator=(U other);

  /// @brief Addition assignment operator.
  /// @param other The vector to add.
  /// @tparam U The type of the vector to add.
  /// @return A reference to the modified vector.
  template <class U> KOKKOS_FUNCTION
  Vector<T,D>& operator+=(const Vector<U,D>& other);

  /// @brief Subtraction assignment operator.
  /// @param other The vector to subtract.
  /// @tparam U The type of the vector to subtract.
  /// @return Vector<T,D>& A reference to the modified vector.
  template <class U> KOKKOS_FUNCTION
  Vector<T,D>& operator-=(const Vector<U,D>& other);

  /// @brief Scalar multiplication assignment operator.
  /// @param scalar The scalar to multiply by.
  /// @tparam U The type of the scalar.
  /// @return A reference to the modified vector.
  template <class U> KOKKOS_FUNCTION
  Vector<T,D>& operator*=(U scalar);

  /// @brief Make the vector covariant.
  /// @details In this class, the components do not carry the signal
  /// information. If we are working with (2+1)D, a specialization
  /// is provided which does not need to take into account Milne components.
  /// In all other cases, the last component is assumed to carry the metric
  /// factor $\f\tau^2\f$.
  /// @param t2 The scaling factor.
  KOKKOS_FUNCTION void make_covariant(double t2) {x[D-1] *= t2;status-=2;};

  /// Make the vector contravariant.
  /// @details This performs the inverse operation of make_covariant.
  /// @see make_covariant
  /// @param t2 The scaling factor.
  KOKKOS_FUNCTION void make_contravariant(double t2){x[D-1] /= t2;status+=2;};

  /// @brief Set the covariance of the vector.
  /// @details A value of 1 means the vector is contravariant, while a value
  /// of -1 means the vector is covariant.
  /// @param c The covariance value to set.
  KOKKOS_FUNCTION void set_covariance(int c){ status = c;}; //Adjust the covariance of the object

  ///@brief Default destructor.
  KOKKOS_FUNCTION ~Vector<T,D>(){};
};

//==============================================================================
// overloaded operations +, - and * between scalars and vectors
template <class T, int D> KOKKOS_FUNCTION
Vector<T,D> operator+ ( const Vector<T,D>&, const Vector<T,D>& );
template <class T, int D> KOKKOS_FUNCTION
Vector<T,D> operator- ( const Vector<T,D>&, const Vector<T,D>& );
template <class T, int D> KOKKOS_FUNCTION
Vector<T,D> operator* ( T, const Vector<T,D>& );


template <class T, int D1, int D2>
class Matrix{
/// @class Matrix
/// @brief Stores data as a D1 by D2 matrix
/// @details We assume that D1 represents the number of rows and D2 represents the number
/// of columns. When we invoke D(i,j), we retrieve the element
/// in the i-th row and j-th column. The fast running index is the last
/// one (the j index). We will refer to the first index as occuping the first
/// index slot and the second index as occupying the second index slot.
private:
  T x[D1*D2];            ///< Linearized storage for the matrix elements
  int status[2] = {1,1}; ///< Stores if the matrix indices are covariant or contravariant. All contravariant by default.
  KOKKOS_INLINE_FUNCTION int index(const int i, const int j) const { return i*D2+j; };
public:
  /// @brief Default constructor. Initializes all elements to zero
  KOKKOS_FUNCTION Matrix<T,D1,D2>() {for (int i=0; i<D1*D2; i++) x[i] = 0;};

  /// @brief Constructor with same initial value for all components
  /// @param x0 The initial value for all elements of the matrix
  KOKKOS_FUNCTION Matrix<T,D1,D2>(T x0) {for (int i = 0; i < D1*D2; i++) x[i] = x0;};

  /// @brief Overload of the call operator to access the components of the matrix.
  /// @param i The row index
  /// @param j The column index
  /// @return The element at the specified indices
  KOKKOS_FUNCTION T operator()(const int i, const int j) const { return x[index(i,j)]; };

  /// @brief Overload of the call operator to access the components of the matrix.
  /// @param i The row index
  /// @param j The column index
  /// @return A reference to the element at the specified indices
  KOKKOS_FUNCTION T& operator()(const int i, const int j) { return x[index(i,j)];};

  /// @brief Assignment operator
  /// @param value The value to assign to all elements of the matrix
  /// @return A reference to the modified matrix
  KOKKOS_FUNCTION Matrix<T,D1,D2>& operator=(T value);

  /// @brief Assignment operator for matrices of different types
  /// @tparam U The type of the input matrix elements
  /// @param a The input matrix to assign from
  /// @return A reference to the modified matrix
  template <class U>
  KOKKOS_FUNCTION Matrix<T,D1,D2>& operator=(const Matrix<U,D1,D2>& a);

  /// @brief Addition assignment operator
  /// @param a The matrix to add
  /// @return A reference to the modified matrix
  KOKKOS_FUNCTION Matrix<T,D1,D2>& operator+=(const Matrix<T,D1,D2>& a);

  /// @brief Subtraction assignment operator
  /// @param a The matrix to subtract
  /// @return A reference to the modified matrix
  KOKKOS_FUNCTION Matrix<T,D1,D2>& operator-=(const Matrix<T,D1,D2>& a);

  /// @brief Scalar multiplication assignment operator
  /// @param scalar The scalar value to multiply with
  /// @return A reference to the modified matrix
  KOKKOS_FUNCTION Matrix<T,D1,D2>& operator*=(T scalar);

  /// @brief Make the matrix covariant in a specific slot
  /// @details In this class, the components do not carry the signal
  /// information. If we are working with (2+1)D, a specialization
  /// is provided which does not need to take into account Milne components.
  /// In all other cases, the last component is assumed to carry the metric
  /// factor $\f\tau^2\f$. Also, this only lowers one of the indexes, chosen
  /// by the slot parameter. Slot values can be 0 or 1. Zero means we are lowering
  /// the first index, i.e. the row index. 1 is lowering the second index
  /// (the column index).
  /// @param slot The slot index
  /// @param t2 The value to set in the slot
  KOKKOS_FUNCTION void make_covariant(int slot, double t2);

  /// @brief Make the matrix contravariant in a specific slot
  /// @details This performs the inverse operation of make_covariant.
  /// @see make_covariant
  /// @param slot The slot index
  /// @param t2 The value to set in the slot
  KOKKOS_FUNCTION void make_contravariant(int slot, double t2);

  /// @brief Set the covariance of the matrix
  /// @details The input is an array of two integers, representing the
  /// covariance of the two indexes. A value of 1 means the index is
  /// contravariant (upper index), while a value of -1 means the index is
  /// covariant (lower index).
  /// @param c An array of two integers representing the covariance
  KOKKOS_FUNCTION void set_covariance(int c[2]);

  /// @brief Destructor
  KOKKOS_FUNCTION ~Matrix<T,D1,D2>(){};
};

//----------------------------------
//Overload operators +, - and * between scalars, vectors and matrices

//==============================
// sum two matrices
template <class T, int D1, int D2> KOKKOS_FUNCTION
Matrix<T,D1,D2> operator+ ( const Matrix<T,D1,D2>&, const Matrix<T,D1,D2>& );
//==============================
// negative of Matrix
template <class T, int D1, int D2> KOKKOS_FUNCTION
Matrix<T,D1,D2> operator- ( const Matrix<T,D1,D2>& );
//==============================
// subtract two matrices
template <class T, int D1, int D2> KOKKOS_FUNCTION
Matrix<T,D1,D2> operator- ( const Matrix<T,D1,D2>&, const Matrix<T,D1,D2>& );
//==============================
// multiply Matrix by scalar
template <class T, int D1, int D2> KOKKOS_FUNCTION
Matrix<T,D1,D2> operator* ( T, const Matrix<T,D1,D2>& );
//==============================
// standard Matrix multiplication
template <class T, int D1, int D2, int Da2, int Db1> KOKKOS_FUNCTION
Matrix<T,D1,D2> operator* ( const Matrix<T,D1,Da2>& a, const Matrix<T,Db1,D2>& b );
//==============================
// right-multiply Matrix a on vector b: a.b
template <class T, int D1, int D2> KOKKOS_FUNCTION
Vector<T,D1>    operator* ( const Matrix<T,D1,D2>& a, const Vector<T,D2>& b );
//==============================
// left-multiply Matrix a on vector b: b.a
template <class T, int D1, int D2> KOKKOS_FUNCTION
Vector<T,D2>    operator* ( const Vector<T,D1>& a, const Matrix<T,D1,D2>& b );

//==============================================================================
// Auxiliary operations
/// @brief Calculate the inner product of two vectors
/// @details The inner product is calculated as the sum of the products of the
/// corresponding components of the two vectors, i.e.
/// \f$a\cdot b = \sum_i a_i b_i\f$.
/// @tparam T The type of the vector components
/// @tparam D The dimension of the vectors
/// @return The inner product of the two vectors
template <class T, int D> KOKKOS_FUNCTION
double inner( const Vector<T,D>& a , const Vector<T,D>& b);

/// @brief Calculate the transpose of a matrix
/// @tparam T The type of the matrix elements
/// @tparam D1 The number of rows of the matrix
/// @tparam D2 The number of columns of the matrix
/// @return The transpose of the input matrix
template <class T, int D1, int D2> KOKKOS_FUNCTION
Matrix<T,D2,D1> transpose( const Matrix<T, D1, D2>& a );

/// @brief Computes the inverse of a matrix.
/// @details We explicitly provide the implementation for 1x1, 2x2 and 3x3
/// matrices. Higher dimensions are not supported.
/// @tparam T The type of the matrix elements
/// @tparam D The dimension of the matrix
/// @param a The input matrix
/// @return The inverse of the input matrix
template <class T, int D> KOKKOS_FUNCTION
Matrix<T,D,D> inverse(const Matrix<T, D, D>& a);

/// @brief Extracts the space components of a matrix column.
/// @tparam T The type of the matrix elements
/// @tparam D1 The number of rows of the matrix
/// @tparam D2 The number of columns of the matrix
/// @param l The index of the column to extract
/// @param a The input matrix
/// @return A Vector with the (D1-1)th column of the input matrix
template <class T, int D1, int D2> KOKKOS_FUNCTION
Vector<T,(D1-1)> colp1( int l, const Matrix<T, D1, D2>& a );

/// @brief Extracts the (D1-1)x(D2-1) submatrix of a matrix
/// @details The submatrix is obtained by removing the first row and column
/// of the input matrix, i.e. we are picking the space components of the matrix.
/// @tparam T The type of the matrix elements
/// @tparam D1 The number of rows of the input matrix
/// @tparam D2 The number of columns of the input matrix
/// @param b The output matrix
template <class T, int D1, int D2> KOKKOS_FUNCTION
void mini(Matrix<T,D1-1,D2-1> &b, const Matrix<T,D1,D2>& a );


//================================================================
//Implementations of the Vector class
//================================================================

template <class T, int D> KOKKOS_FUNCTION
Vector<T,D>& Vector<T,D>::operator=(const Vector<T,D> & a)
{
  for (int i = 0; i < D; i++) x[i] = a(i);
  return *this;
}

template <class T, int D> template <class U> KOKKOS_FUNCTION
Vector<T,D>& Vector<T,D>::operator=(U a)
{
  for (int i = 0; i < D; i++) x[i] = (T)a;
  return *this;
}


template <class T, int D> template <class U> KOKKOS_FUNCTION
Vector<T,D>& Vector<T,D>::operator+=(const Vector<U,D> & a)
{
  for (int i = 0; i < D; i++) x[i] += (T)a(i);
  return *this;
}

template <class T, int D> template <class U> KOKKOS_FUNCTION
Vector<T,D>& Vector<T,D>::operator-=(const Vector<U,D> & a)
{
  for (int i = 0; i < D; i++) x[i] -= (T)a(i);
  return *this;
}

template <class T, int D> template <class U> KOKKOS_FUNCTION
Vector<T,D>& Vector<T,D>::operator*=(U a)
{
  for (int i = 0; i < D; i++) x[i] *= (T)a;
  return *this;
}

//Dimension 2 is special in the sense we do not distinguish
//between covariant and contravariant components
template <> KOKKOS_INLINE_FUNCTION
void Vector<double,2>::make_covariant(double t2) {status-=2;};
template <> KOKKOS_INLINE_FUNCTION
void Vector<float,2>::make_covariant(double t2) {status-=2;};
template <> KOKKOS_INLINE_FUNCTION
void Vector<int,2>::make_covariant(double t2) {status-=2;};
template <> KOKKOS_INLINE_FUNCTION
void Vector<double,2>::make_contravariant(double t2) {status+=2;};
template <> KOKKOS_INLINE_FUNCTION
void Vector<float,2>::make_contravariant(double t2) {status+=2;};
template <> KOKKOS_INLINE_FUNCTION
void Vector<int,2>::make_contravariant(double t2) {status+=2;};

//================================================================
//Implementations of the Matrix class
//================================================================

template <class T, int D1, int D2> KOKKOS_FUNCTION
Matrix<T,D1,D2>& Matrix<T,D1,D2>::operator=(T a)
{
  int k = 0;
  for (int i=0; i<D1; i++)
  for (int j=0; j<D2; j++)
    x[k++] = (T)a;
  return *this; // AOK
}

template <class T, int D1, int D2> template<class U> KOKKOS_FUNCTION
Matrix<T,D1,D2>& Matrix<T,D1,D2>::operator=(const Matrix<U,D1,D2>& a)
{
  int k = 0;
  for (int i=0; i<D1; i++)
  for (int j=0; j<D2; j++)
    x[k++] = (T)a(i,j);
  return *this; // AOK
}


template <class T, int D1, int D2> KOKKOS_FUNCTION
Matrix<T,D1,D2>& Matrix<T,D1,D2>::operator+=(const Matrix<T,D1,D2>& a)
{
  int k = 0;
  for(int i=0; i<D1; i++)
  for(int j=0; j<D2; j++)
    x[k++] += a(i,j);
  return *this; // AOK
}


template <class T, int D1, int D2> KOKKOS_FUNCTION
Matrix<T,D1,D2>& Matrix<T,D1,D2>::operator-=(const Matrix<T,D1,D2>& a)
{
  int k = 0;
  for(int i=0; i<D1; i++)
  for(int j=0; j<D2; j++)
    x[k++] -= a(i,j);
  return *this; // AOK
}


template <class T, int D1, int D2> KOKKOS_FUNCTION
Matrix<T,D1,D2>& Matrix<T,D1,D2>::operator*=(T l)
{
  for (int i=0; i<D1*D2; i++) x[i] *= l;
  return *this; // AOK
}

template <class T, int D1, int D2> KOKKOS_FUNCTION
void Matrix<T,D1,D2>::make_covariant(int slot, double t2){
  status[slot]-=2;
  switch (slot){
    case 0: ///Lower first index. Array becomes M_i^{\,j} = g_ik M^{kj} = tau^2 M^{3j}\delta_i^3
      for (int i=0; i<D1; ++i) x[index(D1-1,i)] *= t2;
      break;
    case 1:
      for (int i=0; i<D2; ++i) x[index(i,D2-1)] *= t2;
      break;
    default:
      break;
  }
}

template <class T, int D1, int D2> KOKKOS_FUNCTION
void Matrix<T,D1,D2>::make_contravariant(int slot, double t2){
  status[slot]+=2;
  switch (slot){
    case 0:
      for (int i=0; i<D1; ++i) x[index(D1-1,i)] /= t2;
      break;
    case 1:
      for (int i=0; i<D2; ++i) x[index(i,D2-1)] /= t2;
      break;
    default:
      break;
  }
}

/// 2x2 matrices are special. We do not distinguish covariant and contravariant ones

template <> KOKKOS_INLINE_FUNCTION
void Matrix<double,2,2>::make_contravariant(int slot, double t2){
  status[slot]+=2;
}
template <> KOKKOS_INLINE_FUNCTION
void Matrix<float,2,2>::make_contravariant(int slot, double t2){
  status[slot]+=2;
}
template <> KOKKOS_INLINE_FUNCTION
void Matrix<int,2,2>::make_contravariant(int slot, double t2){
  status[slot]+=2;
}
template <> KOKKOS_INLINE_FUNCTION
void Matrix<double,2,2>::make_covariant(int slot, double t2){
  status[slot]-=2;
}
template <> KOKKOS_INLINE_FUNCTION
void Matrix<float,2,2>::make_covariant(int slot, double t2){
  status[slot]-=2;
}
template <> KOKKOS_INLINE_FUNCTION
void Matrix<int,2,2>::make_covariant(int slot, double t2){
  status[slot]-=2;
}

//================================================================
//Implementations of vector overload operations
//================================================================
template <class T, int D> KOKKOS_FUNCTION
Vector<T,D> operator+(const Vector<T,D>& a, const Vector<T,D>& b)
{
  Vector<T,D> t;
  for (int i = 0; i < D; i++) t(i) = a(i) + b(i);
  return t;
}

template <class T, int D> KOKKOS_FUNCTION
Vector<T,D> operator-(const Vector<T,D>& a, const Vector<T,D>& b)
{
  Vector<T,D> t;
  for (int i = 0; i < D; i++) t(i) = a(i) - b(i);
  return t;
}

template <class T, int D> KOKKOS_FUNCTION
Vector<T,D> operator*(T l, const Vector<T,D>& a)
{
  Vector<T,D> t;
  for (int i = 0; i < D; i++) t(i) = l*a(i);
  return t;
}

//================================================================
//Implementations of Matrix overload operations
//================================================================

template <class T, int D1, int D2> KOKKOS_FUNCTION
Matrix<T,D1,D2> operator+(const Matrix<T,D1,D2>& a, const Matrix<T,D1,D2>& b)
{
  Matrix<T,D1,D2> t;
  for (int i=0; i<D1; i++)
  for (int j=0; j<D2; j++)
    t(i,j) = a(i,j)+b(i,j);
  return t;
} // AOK


template <class T, int D1, int D2> KOKKOS_FUNCTION
Matrix<T,D1,D2> operator-(const Matrix<T,D1,D2>& a)
{
  return (-1.0)*a;
} // AOK


template <class T, int D1, int D2> KOKKOS_FUNCTION
Matrix<T,D1,D2> operator-(const Matrix<T,D1,D2>& a, const Matrix<T,D1,D2>& b)
{
  Matrix<T,D1,D2> t;
  for (int i = 0; i < D1; i++)
  for (int j = 0; j < D2; j++)
    t(i,j) = a(i,j) - b(i,j);
  return t;
} // AOK

template <class T, int D1, int D2, int Dk> KOKKOS_FUNCTION
Matrix<T,D1,D2> operator*(const Matrix<T,D1,Dk>& a, const Matrix<T,Dk,D2>& b)
{

  Matrix<T,D1,D2> t;
  for (int i=0; i<D1; i++)
  for (int j=0; j<D2; j++)
  for (int k=0; k<Dk; k++)
    t(i,j) += a(i,k) * b(k,j);

  return t;
} // AOK


template <class T, int D1, int D2> KOKKOS_FUNCTION
Matrix<T,D1,D2> operator*(T l, const Matrix<T,D1,D2>& a)
{
  Matrix<T,D1,D2> t;
  for (int i = 0; i < D1; i++)
  for (int j = 0; j < D2; j++)
    t(i,j) = l*a(i,j);

  return t;
} // AOK

template <class T, int D1, int D2> KOKKOS_FUNCTION
Vector<T,D1> operator*(const Matrix<T,D1,D2>& a, const Vector<T,D2>& b)
{
  Vector<T,D1> t;
  for (int i=0; i<D1; i++)
  for (int j=0; j<D2; j++)
    t(i) += a(i,j) * b(j);

  return t;
} // AOK

template <class T, int D1, int D2> KOKKOS_FUNCTION
Vector<T,D2> operator*(const Vector<T,D1>& b, const Matrix<T,D1,D2>& a)
{
  Vector<T,D1> t;
  for (int j=0; j<D2; j++)
  for (int i=0; i<D1; i++)
    t(j) += b(i)*a(i,j) ;

  return t;
} // AOK



template <class T, int D1, int D2> KOKKOS_FUNCTION
Matrix<T,D1,D2> operator* ( const Vector<T,D1>& a, const Vector<T,D2>& b )
{
  Matrix<T,D1,D2> t;
  for (int i = 0; i < D1; i++)
  for (int j = 0; j < D2; j++)
    t(i,j) = a(i)*b(j);
  return t;

} // AOK


//================================================================
//Implementations of auxiliary operations
//================================================================
template <class T, int D> KOKKOS_FUNCTION
double inner (const Vector<T,D>& a, const Vector<T,D>& b)
{
  double t = 0;
  for (int i = 0; i < D; i++) t += a(i)*b(i);
  return t;
}

template <class T, int D1, int D2> KOKKOS_FUNCTION
Matrix<T,D2,D1> transpose(const Matrix<T,D1,D2>& a)
{
  Matrix<T,D2,D1> t;

  for (int i=0; i<D2; i++)
  for (int j=0; j<D1; j++)
    t(i,j) = a(j,i);

  return t;
} // AOK

template<> KOKKOS_INLINE_FUNCTION
Matrix<double,1,1> inverse<double, 1>(const Matrix<double,1,1> &A)
{
  Matrix<double,1,1> Ainv;
  Ainv(0,0) = 1./A(0,0);
  return Ainv;
}

template<>
KOKKOS_INLINE_FUNCTION
Matrix<double,2,2> inverse<double, 2>(const Matrix<double,2,2> &A)
{
  Matrix<double,2,2> Ainv;
  double det = A(0,0)*A(1,1) - A(0,1)*A(1,0);
  Ainv(0,0) = A(1,1)/det;
  Ainv(0,1) = -A(0,1)/det;
  Ainv(1,0) = -A(1,0)/det;
  Ainv(1,1) = A(0,0)/det;
  return Ainv;
}

template<>
KOKKOS_INLINE_FUNCTION
Matrix<double,3,3> inverse<double, 3>(const Matrix<double,3,3> &A)
{
    Matrix<double,3,3> Ainv;
    double det = A(0,0)*A(1,1)*A(2,2) + A(0,1)*A(1,2)*A(2,0) +
                 A(0,2)*A(1,0)*A(2,1) - A(0,2)*A(1,1)*A(2,0) -
                 A(0,1)*A(1,0)*A(2,2) - A(0,0)*A(1,2)*A(2,1);
    Ainv(0,0) = (A(1,1)*A(2,2) - A(1,2)*A(2,1))/det;
    Ainv(0,1) = (A(0,2)*A(2,1) - A(0,1)*A(2,2))/det;
    Ainv(0,2) = (A(0,1)*A(1,2) - A(0,2)*A(1,1))/det;
    Ainv(1,0) = (A(1,2)*A(2,0) - A(1,0)*A(2,2))/det;
    Ainv(1,1) = (A(0,0)*A(2,2) - A(0,2)*A(2,0))/det;
    Ainv(1,2) = (A(0,2)*A(1,0) - A(0,0)*A(1,2))/det;
    Ainv(2,0) = (A(1,0)*A(2,1) - A(1,1)*A(2,0))/det;
    Ainv(2,1) = (A(0,1)*A(2,0) - A(0,0)*A(2,1))/det;
    Ainv(2,2) = (A(0,0)*A(1,1) - A(0,1)*A(1,0))/det;
    return Ainv;
}

template <class T, int D1, int D2> KOKKOS_FUNCTION
Vector<T,(D1-1)> colp1(int l, const Matrix<T,D1,D2>& a)
{
  Vector<T,(D1-1)> v;
  for(int i=1; i<D1; i++) v(i-1)=(T)a(i,l);
  return v;
} // AOK

template <class T, int D1, int D2> KOKKOS_FUNCTION
void mini( Matrix<T,D1-1,D2-1> &b, const Matrix<T,D1,D2>& a)
{
  for(int i=1; i<D1; i++)
  for(int j=1; j<D2; j++)
    b(i-1,j-1) = (T)a(i,j);
} // AOK

template <class T, int D1, int D2> KOKKOS_FUNCTION
double con( const Matrix<T,D1,D2>& a, const Matrix<T,D1,D2>& b )
{
  double t = 0.0;
  for (int i = 0; i < D2; i++)
  for (int j = 0; j < D1; j++)
    t += a(j,i)*b(j,i);
  return t;
} // AOK

// takes spatial components of l^{th} row in space-time tensor (Matrix a)
template <class T, int D1, int D2> KOKKOS_FUNCTION
Vector<T,(D2-1)> rowp1(int l, const Matrix<T,D1,D2>& a)
{
  Vector<T,(D2-1)> v;
  for (int i=1; i<D2; i++) v(i-1)=(T)a(l,i);
  return v;
} // AOK

// store a in transverse spatial components of b
template <class T, int D1, int D2> KOKKOS_FUNCTION
void tmini( Matrix<T,D1,D2>& b, const Matrix<T,D1-1,D2-1>& a )
{
  for (int j = 0; j < (D2-1); j++)
  for (int i = 0; i < (D1-1); i++)
    b(i+1,j+1) = (T)a(i,j);
} // AOK

template <class T, int D> KOKKOS_INLINE_FUNCTION
T tr( const Matrix<T,D,D>& a,T t2)
{
  double s = 0.0;
  for (int i = 0; i < D-1; i++)
    s += a(i,i);
  s += a(D-1,D-1)*t2;
  return s;
} // AOK

template <> KOKKOS_INLINE_FUNCTION
double tr( const Matrix<double,2,2>& a,double t2)
{
  double s = 0.0;
  for (int i = 0; i < 2; i++)
    s += a(i,i);
  return s;
} // AOK


template <class T, int D1, int D2> KOKKOS_FUNCTION
double con2(const Matrix<T,D1,D2>& a, const Matrix<T,D1,D2>& b)
{
  double t = 0.0;
  for (int i = 0; i < D2; i++)
  for (int j = 0; j < D1; j++)
    t += a(i,j)*b(i,j);
  return t;
} // AOK

}}
#endif