// Fixed length (8 and 16 byte) string classes.
//
// A fixed length string has a locally allocated array of characters containing the string text
// padded with zeros.  This allows comparison, assignment, etc. to happen very quickly (approx size/4 CPU cycles).
//
// These strings don't support too many operators, because they're meant to be used as identifiers
// and not for general string usage.  If you need to use them with STL strings, use the c_str() function,
//
// WARNING: Since the strings are 8 (or 16) bytes long, only 7 (or 15)  can be used for text if you want them
// to be compatible with c-style strings.  You can initialize them to 8 (or 16 ) text characters if you want,
// but just make sure not to use them as c strings.  Note that the c_str() function asserts that the last character
// is a 0, however taking the address of a character and using it as a string can cause problems.
//
// by Wade
// converted to template by Sean
// fixed a couple silly things by heypat

#ifndef FIXEDSTRING_H
#define FIXEDSTRING_H

#include "stringx.h"
#include "oserrmsg.h"

#ifdef TARGET_PS2
  typedef unsigned long long fixedstring_big_int;
#elif defined(TARGET_PC) || defined(TARGET_XBOX)
  typedef uint64 fixedstring_big_int;
#elif defined(TARGET_GC)
	typedef unsigned long long fixedstring_big_int;
#else
#endif

#define FIXEDSTRING_BIG_INT_SIZE (sizeof(fixedstring_big_int))

template <int ndwords=2>
class fixedstring
{
public:
  enum { MAX_CHARS = ndwords*4 };
  typedef fixedstring<ndwords> FS;

  // Default constructor - set to all 0's.  Very fast.
  inline fixedstring()
  {
    for (int i=0; i<ndwords; ++i)
      value[ i ] = 0;
  }

  // Copy constructor.  Very fast.
  inline fixedstring(const FS & str )
  {
    for (int i=0; i<ndwords; ++i)
      value[ i ] = str.value[ i ];
  }

  // Construct with char*.  Not very fast.
  // NOTE: Non-explicit so implicit conversion can occur.
  inline fixedstring( const char * str )
  {
    assert( str );

    int len = strlen( str );
    assert( len<=MAX_CHARS );
    //      error("String overflow: " + stringx(str));

    // Set all zeros - this is so comparison doesn't have to look for a terminator.
    for (int i=0; i<ndwords; ++i)
      value[ i ] = 0;

    // Strings are memcpy'd so they don't get a trailing 0.
    memcpy( value, str, len );
  }

  // Assignment operator.  Very fast.
  inline const FS & operator=( const FS & rhs )
  {
    // NOTE: No ( &rhs == this ) check 'cause this way is faster.
    for (int i=0; i<ndwords; ++i)
      value[ i ] = rhs.value[ i ];

    return *this;
  }

  // Array operators.  Very fast.
  // NOTE: No bounds checking is performed.
  inline const char & operator[]( int i ) const
  {
    return reinterpret_cast<const char *>( value )[ i ];
  }

  inline char & operator[]( int i )
  {
    return reinterpret_cast<char *>( value )[ i ];
  }

  // Comparison operator - Very fast.
  inline bool operator==( const FS & rhs ) const
  {
    for (int i=0; i<ndwords; ++i)
      if (value[i] != rhs.value[i])
        return false;
    return true;
  }

  // Comparison operator - Very fast.
  inline bool operator!=( const FS & rhs ) const
  {
    for (int i=0; i<ndwords; ++i)
      if (value[i] != rhs.value[i])
        return true;
    return false;
  }

  // Convert to c style string.  Fast in release, slightly slower in debug due to assert.
  // Makes sure the last character is actually a 0, just in case all bytes are text.
  inline const char * c_str() const
  {
    const char * str = reinterpret_cast<const char *>( value );
    assert( str[ MAX_CHARS - 1 ] == '\0' );
    return str;
  }

  // convert to a c++ style string.  will append a 0 if you don't have one.  that way
  // you can use all characters for text.  not fast.
  inline stringx to_string() const
  {
    stringx retval;
    for(int i=0;i<MAX_CHARS;++i)
    {
      if( (*this)[i] )
        retval.append( 1, (*this)[i] );
    }
    return retval;
  }

private:
  // String characters are stored as ints to make comparison involve less casts.  The casts are instead done
  // in the [] operators.
  unsigned value[ ndwords ];

#ifndef __GNUC__
  friend bool operator<( const FS & lhs, const FS & rhs );
#else
  bool operator<(const FS & rhs )
  {
    const char * s1 = reinterpret_cast<const char *>( &this->value );
    const char * s2 = reinterpret_cast<const char *>( &rhs.value );

    return strncmp( s1, s2, FS::MAX_CHARS ) < 0;
  }
#endif
};

#ifndef __GNUC__
// <<<<< TODO: Speed this up using integer compares if possible.
// Comparison operator - not very fast.
template <int ndwords>
inline bool operator<( const fixedstring<ndwords> & lhs, const fixedstring<ndwords> & rhs )
{
  const char * s1 = reinterpret_cast<const char *>( &lhs.value );
  const char * s2 = reinterpret_cast<const char *>( &rhs.value );

  return strncmp( s1, s2, fixedstring<ndwords>::MAX_CHARS ) < 0;
}
#endif

typedef fixedstring<2> string8;
typedef fixedstring<4> string16;

#endif
