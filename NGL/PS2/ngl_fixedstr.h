// Fixed length (32 byte, 31 character) string classes.
//
// A fixed length string has a locally allocated array of characters containing the string text
// padded with zeros.  This allows comparison, assignment, etc. to happen very quickly (approx size/4 CPU cycles).
//
// These strings don't support too many operators, because they're meant to be used as identifiers
// and not for general string usage.  If you need to use them with STL strings, use the c_str() function,

#ifndef nglFixedString_H
#define nglFixedString_H

#include <string.h>

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
  
  enum { STR_SIZE = 32 };
  enum { NUM_FINTS = STR_SIZE / FINT_SIZE };

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
  nglFixedString(const FS & s )
  { 
    for (int i=0; i<NUM_FINTS; ++i)
      value()[i] = s.value()[i]; 
  }

  // Construct with char*.  Not very fast.
  // NOTE: Non-explicit so implicit conversion can occur.
  nglFixedString( const char* s )
  {
    // Set all zeros - this is so comparison doesn't have to look for a terminator.
    for (int i=0; i<NUM_FINTS; ++i)
      value()[i] = 0;

    strncpy( str, s, STR_SIZE - 1 );
    strlwr( str );
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
    return str; 
  }

  // Non-const implicit cast operator is provided since no conversion is neccessary.
  operator const char*() const { return c_str(); };

  // Convert to c style string.  Fast in release, slightly slower in debug due to assert.
  // Makes sure the last character is actually a 0, just in case all bytes are text.
  char* c_str()
  { 
    return str; 
  }

  // Non-const implicit cast operator is provided since no conversion is neccessary.
  operator char*() { return c_str(); };

  bool operator<(const FS & rhs )
  {
    return strcmp( &str[0], &rhs.str[0] ) < 0;
  }
  
  bool operator>=(const FS & rhs )
  {
    return strcmp( &str[0], &rhs.str[0] ) >= 0;
  }
  
private:
  char str[STR_SIZE];
}
#ifdef NGL_PS2
__attribute__ ((aligned(8)))
#endif
;

#endif
