#pragma once
#include <vector>
#include <stdexcept>
#include <random>
#include <type_traits>

// IVector is an interface of a generic vector of T, and the IMatrix is an interface of a generic matrix of T
template <typename T>
class IVector {
public:

  virtual const size_t size() const = 0;
  
  virtual void push( T element ) = 0;
  virtual void addAt( size_t index, T element ) = 0;

  virtual void setAt( size_t index, T element ) = 0;
  virtual T getAt( size_t index ) const = 0;
  virtual void removeAt( size_t index ) = 0;

  virtual T operator[]( size_t index ) const = 0;
  virtual ~IVector() {};
};

template <typename T>
class IMatrix {
public:
  virtual const std::pair<size_t, size_t> size() const = 0;
  virtual const size_t numCols() const = 0;
  virtual const size_t numRows() const = 0;

  virtual T getAt( size_t, size_t ) const = 0;
  virtual void setAt( size_t, size_t, T ) = 0;

  virtual const std::string type() const = 0;
  virtual const IMatrix<T> * getComponent() const = 0;
  virtual ~IMatrix() {};
};

// This approach will need some adjustments for the sparse vectors, but should do well enough as a base class.
template <typename T>
class AVector : public IVector<T>
{
protected:
  std::vector<T> vector;
public:
  AVector( size_t size = 0 ) : vector( size ) {};

  virtual const size_t size() const override { return vector.size(); }

  virtual void push( T element ) override
  {
    vector.push_back( element );
  }
  virtual void addAt( size_t index, T element ) override
  {
    if ( index < 0 || index >= vector.size() )
      throw std::out_of_range("Vector subscript out of range");
    vector.insert( vector.begin() + index, element );
  }
  virtual void setAt( size_t index, T element ) override
  {
    vector[index] = element;
  }
  virtual T getAt( size_t index ) const override
  {
    return vector[index];
  }
  virtual void removeAt( size_t index ) override
  {
    vector.erase( vector.begin() + index );
  }
  virtual T operator[]( size_t index ) const override {
    return getAt( index );
  };
  ~AVector() {};
};

// At its core a matrix is still a vector. However, ordinary matrix is a vector of vectors while sparse one is a vector of tuples.
template <typename T>
class AMatrix : public IMatrix<T>
{
protected:
  size_t cols;
  size_t rows;
public:
  AMatrix( size_t _rows = 0, size_t _cols = 0) : rows( _rows ), cols( _cols ) {};
  virtual const std::pair<size_t, size_t> size() const override { return { cols, rows }; };
  virtual const size_t numCols() const override { return cols; };
  virtual const size_t numRows() const override { return rows; };
  virtual const IMatrix<T> * getComponent() const override { return this; }
  virtual ~AMatrix() {};
};

template <typename T>
class OrdinaryVector : public AVector<T>
{
public:
  OrdinaryVector( size_t size = 0 ) : AVector<T>( size ) {};
  
  ~OrdinaryVector() {};
};

template <typename T>
class SparseVector : public IVector<T>
{
private:
  AVector<std::pair<size_t, T>> vector;
  size_t vectorsize;
public:
  SparseVector( size_t size = 0 ) : vectorsize( size ), vector( 0 ) {};

  const size_t size() const override { return vectorsize; }
  //void push( std::pair<size_t, T> ) override = delete;
  void push( T element )
  {
    if( element != NULL )
      vector.push( { vectorsize, element } );
    vectorsize++;
  };
  //std::pair<size_t, T> getAt( size_t ) override = delete;
  T getAt( size_t index ) const override {
    if ( index < 0 || index >= vectorsize )
      throw std::out_of_range("Vector subscript out of range");
    for ( size_t i = 0; i < vector.size(); ++i )
    {
      if ( vector.getAt(i).first == index )
        return vector.getAt(i).second;
    }
    return (T)0;
  }

  void setAt( size_t index, T element ) override
  {
    if ( index < 0 || index >= vectorsize )
      throw std::out_of_range("Vector subscript out of range");
    for ( size_t i = 0; i < vector.size(); ++i )
    {
      if ( vector.getAt( i ).first == index )
      {
        vector.setAt( i, { index, element } );
        return;
      }
    }
    vector.push( { index, element } );
  }

  void addAt( size_t index, T element ) {
    for ( size_t i = 0; i < vector.size(); ++i )
    {
      if ( vector.getAt(i).first >= index )
        vector.setAt( i, { vector.getAt( i ).first + 1, vector.getAt( i ).second } );
    }
    vector.push( { index, element } );
    if ( vectorsize > index )
      vectorsize++;
    else
      vectorsize = index + 1;
  };
  void removeAt( size_t index )
  {
    size_t indexInVec = 0;
    bool isIn = false;
    for ( ; indexInVec < vector.size(); ++indexInVec )
      if ( vector.getAt( indexInVec ).first == index )
      {
        isIn = true;
        break;
      }
    if ( isIn )
      vector.removeAt( indexInVec );
    for ( size_t i = 0; i < vector.size(); ++i )
      if ( vector[i].first > index )
        vector.setAt( i, { vector.getAt( i ).first - 1, vector.getAt( i ).second } );
    vectorsize--;
  };
  
  T operator[]( size_t index ) const override {
    return getAt( index );
  }

  ~SparseVector() {};
};

template <typename T>
class SparseMatrix : public AMatrix<T>
{
private:
  std::vector<SparseVector<T>> data;
public:
  SparseMatrix( size_t n = 0, size_t m = 0 ) : data(m)
  {
    this->cols = n;
    this->rows = m;
    for ( size_t i = 0; i < m; i++ )
      data[i] = SparseVector<T>( n );
  }
  
  void setAt( size_t i, size_t j, T element ) override
  {
    data[i].setAt( j, element );
  }

  T getAt( size_t i, size_t j ) const override
  {
    return data[i].getAt( j );
  }

  const std::string type() const override
  {
    return "Sparse";
  }

  ~SparseMatrix() {};
};

template <typename T>
class OrdinaryMatrix : public AMatrix<T>
{
private:
  std::vector<OrdinaryVector<T>> data;
public:
  OrdinaryMatrix( size_t n = 0, size_t m = 0 ) : data( m )
  {
    this->cols = n;
    this->rows = m;
    for ( size_t i = 0; i < m; i++ )
      data[i] = OrdinaryVector<T>( n );
  }

  void setAt( size_t i, size_t j, T element ) override
  {
    data[i].setAt( j, element );
  }

  T getAt( size_t i, size_t j ) const override
  {
    return data[i].getAt( j );
  }

  const std::string type() const override
  {
    return "Ordinary";
  }
  ~OrdinaryMatrix() {};
};

template <typename T>
class SwitchingDecorator : public IMatrix<T>
{
private:
  IMatrix<T> * m;
  std::vector<size_t> columnOrder;
  std::vector<size_t> rowOrder;
public:
  // Default constructor.
  SwitchingDecorator() : m( nullptr ), columnOrder( 0 ), rowOrder( 0 ) {};
  // Copy constructor.
  SwitchingDecorator( IMatrix<T> & matr ) : columnOrder(matr.numCols()), rowOrder(matr.numRows())
  {
    m = &matr;
    for ( size_t i = 0; i < columnOrder.size(); ++i )
      columnOrder[i] = i;
    for ( size_t i = 0; i < rowOrder.size(); ++i )
      rowOrder[i] = i;
  }

  SwitchingDecorator & switchColumns( size_t i, size_t j )
  {
    std::swap( columnOrder[i], columnOrder[j] );
    return *this;
  }

  SwitchingDecorator & switchRows( size_t i, size_t j )
  {
    std::swap( rowOrder[i], rowOrder[j] );
    return *this;
  }

  const std::pair<size_t, size_t> size() const override
  {
    return m->size();
  }
  const size_t numCols() const override
  {
    return m->numCols();
  }
  const size_t numRows() const override
  {
    return m->numRows();
  }

  T getAt( size_t i, size_t j) const override
  {
    return m->getAt( rowOrder[i], columnOrder[j] );
  }
  void setAt( size_t i, size_t j, T element ) override
  {
    m->setAt( rowOrder[i], columnOrder[j], element );
  }

  const std::string type() const override
  {
    return "Decorated";
  }

  IMatrix<T> * get()
  {
    return m;
  }

  const IMatrix<T> * getComponent() const override
  {
    return m->getComponent();
  }

  ~SwitchingDecorator() {};
};

template <typename T>
class TransposingDecorator : public IMatrix<T>
{
private:
  IMatrix<T> * m;
public:
  TransposingDecorator( const IMatrix<T> * matrix ) : m( const_cast<IMatrix<float>*>(matrix) ) {};
  const std::pair<size_t, size_t> size() const
  {
    return { numCols(), numRows() };
  }
  const size_t numCols() const
  {
    return m->numRows();
  }
  const size_t numRows() const
  {
    return m->numCols();
  }

  T getAt( size_t i, size_t j) const
  {
    return m->getAt( j, i );
  }
  void setAt( size_t i, size_t j, T element )
  {
    return m->setAt( j, i, element );
  }

  const std::string type() const
  {
    return "Transposed matrix";
  }
  const IMatrix<T> * getComponent() const
  {
    return m->getComponent();
  }
  ~TransposingDecorator() {};
};

// that, honestly, sucks. No way to just pull random "T", so I just have to choose int or double. ugh.
template <typename T>
class MatrixInitializer
{
public:
  static void initMatrix( AMatrix<T> * matrix, size_t nonzeroes, T max )
  {
    std::random_device rd;
    std::mt19937 gen(rd());
    
    std::uniform_real_distribution<long double> dist( -max, max );
    //this way we get any amount of unique random coordinates as needed.
    std::vector<std::pair<size_t, size_t>> slots;
    slots.reserve( matrix->numCols() * matrix->numRows() );
    for ( size_t i = 0; i < matrix->numRows(); ++i )
      for ( size_t j = 0; j < matrix->numCols(); ++j )
        slots.push_back( { i, j } );
    std::shuffle( slots.begin(), slots.end(), gen );
    
    for ( size_t i = 0; i < nonzeroes; ++i )
    {
      T value = static_cast<T>( dist( gen ) );
      while ( value == (T)0 )
        value = static_cast<T>( dist( gen ) );
      matrix->setAt( slots[i].first, slots[i].second, value );
    }
  }
};

template<typename T>
class MatrStats
{
private:
  T sum;
  double average;
  T max;
  size_t nonzero;
public:

  MatrStats( AMatrix<T> * matrix )
  {
    if ( matrix )
    {
      nonzero = 0;
      sum = matrix->getAt( 0, 0 );
      size_t amount = matrix->numCols() * matrix->numRows();
      double mult = 1.0 / amount;
      max = matrix->getAt( 0, 0 );
      for ( size_t i = 0; i < matrix->numCols(); ++i )
        for ( size_t j = 0; j < matrix->numRows(); ++j )
        {
          T currentValue = matrix->getAt( i, j );
          sum += currentValue;
          if ( max < currentValue )
            max = currentValue;
          if ( currentValue != NULL )
            nonzero++;
        }
      sum -= matrix->getAt( 0, 0 );
      average = static_cast<double>( sum ) * mult;
    }
    else
    {
      nonzero = 0;
      sum = static_cast<T>( 0 );
      average = 0.0;
      max = static_cast<T>( 0 );
    }
  }
  bool setMatrix( AMatrix<T> * matrix )
  {
    if ( matrix )
    {
      nonzero = 0;
      sum = matrix->getAt( 0, 0 );
      size_t amount = matrix->numCols() * matrix->numRows();
      double mult = 1.0 / amount;
      max = matrix->getAt( 0, 0 );
      for ( size_t i = 0; i < matrix->numCols(); ++i )
        for ( size_t j = 0; j < matrix->numRows(); ++j )
        {
          T currentValue = matrix->getAt( i, j );
          sum += currentValue;
          if ( max < currentValue )
            max = currentValue;
          if ( currentValue != NULL )
            nonzero++;
        }
      sum -= matrix->getAt( 0, 0 );
      average = static_cast<double>( sum ) * mult;
      return true;
    }
    else return false;
  }

  T getSum() { return sum; };
  T getMax() { return max; };
  double getAvg() { return average; };
  size_t getNonzero() { return nonzero; };
};

template<typename T>
class HorizontalGrouping : public IMatrix<T>
{
  std::vector<IMatrix<T> *> matrices;
public:
  HorizontalGrouping() : matrices( 0 ) {};
  HorizontalGrouping( const IMatrix<float>* matr ) : matrices( 0 )
  {
    matrices.reserve( 1 );
    matrices.push_back( const_cast<IMatrix<float>*>(matr) );
  }
  const std::pair<size_t, size_t> size() const
  {
    return { numRows(), numRows() };
  }
  const size_t numCols() const
  {
    size_t cols = 0;
    for ( IMatrix<T> * matr : matrices )
      cols += matr->numCols();
    return cols;
  }
  const size_t numRows() const
  {
    size_t rows = 0;
    for ( IMatrix<T> * matr : matrices )
      rows = matr->numRows() > rows ? matr->numRows() : rows;
    return rows;
  }

  T getAt( size_t i, size_t j ) const
  {
    for ( IMatrix<T> * matr : matrices )
    {
      if ( j >= matr->numCols() )
      {
        j -= matr->numCols();
        continue;
      }
      if ( i >= matr->numRows() )
        return 0.0;
      return matr->getAt( i, j );
    }
    return 0.0;
  }
  void setAt( size_t i, size_t j, T element )
  {
    for ( IMatrix<T> * matr : matrices )
    {
      if ( j >= matr->numCols() )
      {
        j -= matr->numCols();
        continue;
      }
      if ( i >= matr->numRows() )
        return;
      matr->setAt( i, j, element );
    }
  }
  void add( const IMatrix<T> * element )
  {
    matrices.push_back( const_cast<IMatrix<T>*>( element ) );
  }

  const std::string type() const
  {
    return "Horizontal matrix group";
  }
  const IMatrix<T> * getComponent() const
  {
    return this;
  }
  ~HorizontalGrouping() {};
};
