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
// made base type independent by Wade

#ifndef nglFixedString_H
#define nglFixedString_H

#include <string.h>
#ifdef NGL_GC
#include <ctype.h>
#endif

class nglFixedString
{
public:
#ifdef NGL_PS2
  typedef u_long fint;
  enum { FINT_SIZE = 8 };
#else
  typedef int fint;
  enum { FINT_SIZE = 4 };
#endif

  enum { MAX_CHARS = 32 };
  enum { NUM_FINTS = MAX_CHARS / FINT_SIZE };

  typedef nglFixedString FS;

  fint* value() { return reinterpret_cast<fint*>( str ); }
  const fint* value() const { return reinterpret_cast<const fint*>( str ); }

  // Default constructor - set to all 0's.  Very fast.
  nglFixedString()
  {
    for (int i=0; i<NUM_FINTS; ++i)
      value()[i] = 0;
  }

  // Copy constructor.  Very fast.
  nglFixedString(const FS & str )
  {
    for (int i=0; i<NUM_FINTS; ++i)
      value()[i] = str.value()[i];
  }

  // Construct with char*.  Not very fast.
  // NOTE: Non-explicit so implicit conversion can occur.
  nglFixedString( const char* s )
  {
    // Set all zeros - this is so comparison doesn't have to look for a terminator.
    for (int i=0; i<NUM_FINTS; ++i)
      value()[i] = 0;

#ifdef NGL_GC
	for (int j=0; j<MAX_CHARS; ++j)
	  if (!s[j]) break;
	  else str[j] = tolower(s[j]);
#else
    strncpy( str, s, MAX_CHARS );
    str[ MAX_CHARS - 1 ] = '\0';
    strlwr( str );
#endif
  }

  // Assignment operator.  Very fast.
  const FS & operator=( const FS & rhs )
  {
    // NOTE: No ( &rhs == this ) check 'cause this way is faster.
    for (int i=0; i<NUM_FINTS; ++i)
      value()[i] = rhs.value()[i];

    return *this;
  }

  // Array operators.  Very fast.
  // NOTE: No bounds checking is performed.
  const char & operator[]( int i ) const
  {
    return reinterpret_cast<const char*>( value() )[i];
  }

  char & operator[]( int i )
  {
    return reinterpret_cast<char*>( value() )[i];
  }

  // Comparison operator - Very fast.
  bool operator==( const FS & rhs ) const
  {
    for (int i=0; i<NUM_FINTS; ++i)
      if (!(value()[i] == rhs.value()[i]))
        return false;
    return true;
  }

  // Comparison operator - Very fast.
  bool operator!=( const FS & rhs ) const
  {
    for (int i=0; i<NUM_FINTS; ++i)
      if (value()[i] != rhs.value()[i])
        return true;
    return false;
  }

  // Convert to c style string.  Fast in release, slightly slower in debug due to assert.
  // Makes sure the last character is actually a 0, just in case all bytes are text.
  const char* c_str() const
  {
//    assert( str[ MAX_CHARS - 1 ] == '\0' );
    return str;
  }

  // Non-const implicit cast operator is provided since no conversion is neccessary.
  operator const char*() const { return c_str(); };

  // Convert to c style string.  Fast in release, slightly slower in debug due to assert.
  // Makes sure the last character is actually a 0, just in case all bytes are text.
  char* c_str()
  {
//    assert( str[ MAX_CHARS - 1 ] == '\0' );
    return str;
  }

  // Non-const implicit cast operator is provided since no conversion is neccessary.
  operator char*() { return c_str(); };

  bool operator<(const FS & rhs )
  {
    return strncmp( &str[0], &rhs.str[0], FS::MAX_CHARS ) < 0;
  }

  bool operator>=(const FS & rhs )
  {
    return strncmp( &str[0], &rhs.str[0], FS::MAX_CHARS ) >= 0;
  }

private:
  char str[MAX_CHARS];
}
#ifdef NGL_PS2
__attribute__ ((aligned(8)))
#endif
;

#endif
