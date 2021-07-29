#ifndef _FAST_VECTOR_H
#define _FAST_VECTOR_H


template <class T>
class fast_vector
  {
  public:
    typedef unsigned int size_t;

  private:
    size_t n;
    T* buf;

    void _construct( size_t _n )
      {
      n = _n;
//      if ( n )
        buf = NEW T[n];
//      else
//        buf = NULL;
      }

    void _copy( const T* src )
      {
      memcpy( buf, src, size()*sizeof(T) );
      }

  public:
    fast_vector()
      {
      _construct( 0 );
      }

    ~fast_vector()
      {
      delete[] buf;
      }

    fast_vector( size_t _n )
      {
      _construct( _n );
      }

    fast_vector( size_t _n, const T& fill )
      {
      _construct( _n );
      iterator i = begin();
      iterator i_end = end();
      for ( ; i!=i_end; i++ )
        *i = fill;
      }

    fast_vector( size_t _n, const T* src )
      {
      _construct( _n );
      _copy( src );
      }

    fast_vector( const fast_vector& b )
      {
      _construct( b.size() );
      _copy( b.buf );
      }

    const fast_vector& operator=( const fast_vector& b )
      {
      assert( size() == b.size() );
      _copy( b.buf );
      return *this;
      }

    size_t size() const { return n; }

    operator T*() { return buf; }
    T& operator[]( size_t i ) { assert(i<size()); return buf[i]; }

    typedef T* iterator;
    iterator begin() { return buf; }
    iterator end() { return buf+size(); }
  };


#endif  // _FAST_VECTOR_H
