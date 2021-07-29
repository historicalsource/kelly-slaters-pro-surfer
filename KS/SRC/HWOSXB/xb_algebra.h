/*---------------------------------------------------------------------------------------------------

  C++ Vector and Matrix and Quaternion Algebra routines
  Author: Jean-Francois DOUE
  Version 3.1 --- October 1993

  Stolen, updated, and somewhat improved by Wade.

  Cleaned up and streamlined by Sean.

  Note that the implementation to all of these classes follows their definitions at the end of the
  file.  This is currently the only way to get VC++ to inline them properly, and I figured that the
  speed increase of having these classes inlined is worth the extra compile time.

---------------------------------------------------------------------------------------------------*/

#ifndef ALGEBRA_H
#define ALGEBRA_H

#include "hwmath.h"
#include "chunkfile.h"

class vector2d
{
public:
  typedef rational_t T;
  T x,y;

  // constructors
  inline vector2d() : x(0.0f), y(0.0f) {} // uninitialized default ctor
  inline vector2d(const vector2d& v) { x = v.x; y = v.y; } // compiler would auto-generate this copy ctor
  inline vector2d(T _x, T _y) { x = _x; y = _y; }
  explicit inline vector2d(T d) { x = y = d; }  // Initialize all members to d (usually 0.0f)

  inline vector2d& operator	= ( const vector2d& v )	    // Assignment operator
  { x = v.x; y = v.y; return *this; }

  inline vector2d& operator += ( const vector2d& v )	  // vector addition
  { x += v.x; y += v.y; return *this; }

  inline vector2d& operator -= ( const vector2d& v )	  // vector subtraction
  { x -= v.x; y -= v.y; return *this; }

  inline vector2d& operator *= ( T d )	    // multiplication by a scalar
  { x *= d; y *= d; return *this; }

  inline vector2d& operator /= ( T d )	    // division by a scalar
  { T d_inv = 1.0f/d; x *= d_inv; y *= d_inv; return *this; }

  inline T length2() const			// vector squared length 
  { return x*x + y*y; }

  inline T length() const			// vector length 
  { return (T)__fsqrt(x*x + y*y); }

  inline vector2d& normalize()			    // vector normalize (set length to 1.0f)
  { 
    T l2 = length2();
    if (l2>SMALL_DIST*SMALL_DIST)
      *this *= fast_recip_sqrt(l2); 
    return *this; 
  }
  inline vector2d& set_length(T newlen=1.0F)		// set vector length 
  { 
    T l2 = length2();
    if (l2>SMALL_DIST*SMALL_DIST)
      *this *= newlen * fast_recip_sqrt(l2); 
    return *this; 
  }

  inline vector2d operator - () const { return vector2d(-x,-y); } // vector reflection

  // get this vector rotated 90 degrees counterclockwise
  inline vector2d perp() const { return vector2d(-y,x); }

  inline const T& operator[](int i) const
  { return (&x)[i]; }

  inline T& operator[](int i)
  { return (&x)[i]; }

  bool is_valid() const { return x>-1e30f && x<1e30f && y>-1e30f && y<1e30f /*&& !(_isnan(x) || _isnan(y))*/; }
};

inline vector2d operator + (const vector2d& a, const vector2d& b)
{ return vector2d(a.x + b.x, a.y + b.y); }

inline vector2d operator - (const vector2d& a, const vector2d& b)
{ return vector2d(a.x - b.x, a.y - b.y); }

inline vector2d operator * (const vector2d& a, vector2d::T d)
{ return vector2d(d * a.x, d * a.y); }

inline vector2d operator * (vector2d::T d, const vector2d& a)
{ return vector2d(d * a.x, d * a.y); }

inline vector2d operator / (const vector2d& a, vector2d::T d)
{ 
  assert( d != 0.0f );
  rational_t d_inv = 1.0f/d; 
  return vector2d(a.x*d_inv, a.y*d_inv); 
}

inline bool operator == (const vector2d& a, const vector2d& b)
{ return (a.x == b.x) && (a.y == b.y); }

inline bool operator != (const vector2d& a, const vector2d& b)
{ return (a.x != b.x) || (a.y != b.y); }

inline vector2d::T dot(const vector2d& a, const vector2d& b)
{ return a.x*b.x + a.y*b.y; }

inline vector2d::T perp_dot(const vector2d& a, const vector2d& b)
{ return a.y*b.x - a.x*b.y; }

// Inexact equality based on distance
inline bool approx_equals(const vector2d& a, const vector2d& b, vector2d::T dist)
{
  return (a - b).length2() <= dist*dist;
}

inline rational_t angle_between( const vector2d& v1, const vector2d& v2 )
{
  // because dot(v1,v2) = |v1||v2|cos theta
  // theta = arccos( dot(v1,v2) / |v1||v2| )
  vector2d::T numerator = dot( v1, v2 );
  vector2d::T denominator = v1.length()*v2.length();
  if ( denominator == 0.0f )
    return 0.0f;
  return (rational_t)fast_acos( numerator/denominator );
}

// Return a stringx describing the vector.
inline stringx v2tos(const vector2d& v)
{
  return stringx("<")+ftos(v.x)+","+ftos(v.y)+">";
}

typedef vector2d vector2dr;
typedef vector2d Vc2;
typedef vector2d Pt2;

// Stream interface
#if !defined(NO_SERIAL_OUT)
inline void serial_out( chunk_file& io, const vector2d& v )
{ 
  if ( io.get_type() == chunk_file::CFT_TEXT )
  {
    io.get_text()->write( ftos(v.x) + " " );
    io.get_text()->write( ftos(v.y) + sendl );
  }
  else
    io.get_binary()->write( (void*)&v, sizeof(vector2d) );
}
#endif

#if !defined(NO_SERIAL_IN)
inline void serial_in( chunk_file& io, vector2d* v )
{
  serial_in( io, &v->x );
  serial_in( io, &v->y );
}
#endif



// 2d vector made of integers
class vector2di
{
public:
  typedef int T;
  T x,y;

  // constructors
  inline vector2di() : x(0), y(0) {} // uninitialized default ctor
  inline vector2di(const vector2d& v) { x = (int)v.x; y = (int)v.y; } // compiler would auto-generate this copy ctor
  inline vector2di(T _x, T _y) { x = _x; y = _y; }
  explicit inline vector2di(T d) { x = y = d; }  // Initialize all members to d (usually 0)

  inline vector2di& operator	= ( const vector2di& v )	   // assignment
  { x = v.x; y = v.y; return *this; }

  inline vector2di& operator += ( const vector2di& v )	  // vector addition
  { x += v.x; y += v.y; return *this; }

  inline vector2di& operator -= ( const vector2di& v )	  // vector subtraction
  { x -= v.x; y -= v.y; return *this; }

  inline vector2di& operator *= ( T d )	    // multiplication by an integer
  { x *= d; y *= d; return *this; }

  inline vector2di& operator /= ( T d )	    // division by an integer
  { x /= d; y /= d; return *this; }

  inline T length2() const			// vector squared length 
  { return x*x + y*y; }

  inline T length() const			// vector length 
  { return (T)__fsqrt(x*x + y*y); }

  inline vector2di operator - () const { return vector2di(-x,-y); } // reflection

  // get this vector rotated 90 degrees counterclockwise
  inline vector2di perp() const { return vector2di(-y,x); }

  inline const T& operator[](int i) const
  { return (&x)[i]; }

  inline T& operator[](int i)
  { return (&x)[i]; }
};

inline vector2di operator + (const vector2di& a, const vector2di& b)
{ return vector2di(a.x + b.x, a.y + b.y); }

inline vector2di operator - (const vector2di& a, const vector2di& b)
{ return vector2di(a.x - b.x, a.y - b.y); }

inline vector2di operator * (const vector2di& a, vector2di::T d)
{ return vector2di(d * a.x, d * a.y); }

inline vector2di operator * (vector2di::T d, const vector2di& a)
{ return vector2di(d * a.x, d * a.y); }

inline vector2di operator / (const vector2di& a, vector2di::T d)
{ 
  assert( d != 0 );
  return vector2di(a.x/d, a.y/d); 
}

inline bool operator == (const vector2di& a, const vector2di& b)
{ return (a.x == b.x) && (a.y == b.y); }

inline bool operator != (const vector2di& a, const vector2di& b)
{ return (a.x != b.x) || (a.y != b.y); }

inline vector2di::T dot(const vector2di& a, const vector2di& b)
{ return a.x*b.x + a.y*b.y; }

inline vector2di::T perp_dot(const vector2di& a, const vector2di& b)
{ return a.y*b.x - a.x*b.y; }

// Inexact equality based on distanct
inline bool approx_equals(const vector2di& a, const vector2di& b, vector2di::T dist)
{
  return (a - b).length2() <= dist*dist;
}

inline rational_t angle_between( const vector2di& v1, const vector2di& v2 )
{
  // because dot(v1,v2) = |v1||v2|cos theta
  // theta = arccos( dot(v1,v2) / |v1||v2| )
  vector2di::T numerator = dot( v1, v2 );
  vector2di::T denominator = v1.length()*v2.length();
  if ( denominator == 0 )
    return 0.0f;
  return (rational_t)fast_acos( numerator/float(denominator) );
}

// Return a stringx describing the vector.
inline stringx v2itos(const vector2di& v)
{
  return stringx("<")+itos(v.x)+","+itos(v.y)+">";
}

typedef vector2di iVc2;
typedef vector2di iPt2;

// Stream interface
#if !defined(NO_SERIAL_OUT)
inline void serial_out( chunk_file& io, const vector2di& v )
{ 
  if ( io.get_type() == chunk_file::CFT_TEXT )
  {
    io.get_text()->write( itos(v.x) + " " );
    io.get_text()->write( itos(v.y) + sendl );
  }
  else
    io.get_binary()->write( (void*)&v, sizeof(vector2di) );
}
#endif

#if !defined(NO_SERIAL_IN)
inline void serial_in( chunk_file& io, vector2di* v )
{
  serial_in( io, &v->x );
  serial_in( io, &v->y );
}
#endif


class vector3d
{
public:
  typedef rational_t T;
  T x,y,z;

  // constructors
  inline vector3d() : x(0.0f), y(0.0f), z(0.0f) {} // uninitialized default ctor
  inline vector3d(const vector3d& v) { x = v.x; y = v.y; z = v.z; } // compiler would auto-generate this copy ctor
  inline vector3d(T _x, T _y, T _z) { x = _x; y = _y; z = _z; }
  
  // make from vector2d, d becomes new Z component.
  explicit inline vector3d(const vector2d& v, T d=1.0F)
  { x = v.x; y = v.y; z = d; }

  explicit inline vector3d(T d) { x = y = z = d; }  // Initialize all members to d (usually 0.0f)

  inline vector3d& operator	= ( const vector3d& v )	    // Assignment operator
  { x = v.x; y = v.y; z = v.z; return *this; }

  inline vector3d& operator += ( const vector3d& v )	  // vector addition
  { x += v.x; y += v.y; z += v.z; return *this; }

  inline vector3d& operator -= ( const vector3d& v )	  // vector subtraction
  { x -= v.x; y -= v.y; z -= v.z; return *this; }

  inline vector3d& operator *= ( T d )	    // multiplication by a scalar
  { x *= d; y *= d; z *= d; return *this; }

  inline vector3d& operator /= ( T d )	    // division by a scalar
  { T d_inv = 1.0f/d; x *= d_inv; y *= d_inv; z *= d_inv; return *this; }

  inline T length2() const			// vector squared length 
  { return x*x + y*y + z*z; }

  inline T length() const			// vector length 
  { return (T)__fsqrt(x*x + y*y + z*z); }

  inline vector3d& normalize()			    // vector normalize (set length to 1.0f)
  { 
    T l2 = length2();
    if (l2>SMALL_DIST*SMALL_DIST)
      *this *= fast_recip_sqrt(l2); 
    return *this; 
  }
  inline vector3d& set_length(T newlen=1.0F)		// set vector length 
  { 
    T l2 = length2();
    if (l2>SMALL_DIST*SMALL_DIST)
      *this *= newlen * fast_recip_sqrt(l2); 
    return *this; 
  }

  inline vector3d operator - () const { return vector3d(-x,-y,-z); } // vector reflection

  inline vector3d perp() const;  // find an arbitrary unit vector perpendicular to this one

  inline const vector2d& get_xy() const { return *(vector2d*)&x; }
  inline const vector2d& get_yz() const { return *(vector2d*)&y; }
  inline const vector2d  get_xz() const { return vector2d(x,z); }

  inline T xy_length2() const { return x*x + y*y; }
  inline T yz_length2() const { return y*y + z*z; }
  inline T xz_length2() const { return x*x + z*z; }

  inline T xy_length() const { return (T)__fsqrt(x*x + y*y); }
  inline T yz_length() const { return (T)__fsqrt(y*y + z*z); }
  inline T xz_length() const { return (T)__fsqrt(x*x + z*z); }

  inline const T& operator[](int i) const
  { return (&x)[i]; }

  inline T& operator[](int i)
  { return (&x)[i]; }

  bool is_valid() const { return x>-1e30f && x<1e30f && y>-1e30f && y<1e30f && z>-1e30f && z<1e30f 
                                 /*&& !(_isnan(x) || _isnan(y) || _isnan(z))*/; }
};

inline vector3d operator + (const vector3d& a, const vector3d& b)
{ return vector3d(a.x + b.x, a.y + b.y, a.z + b.z); }

inline vector3d operator - (const vector3d& a, const vector3d& b)
{ return vector3d(a.x - b.x, a.y - b.y, a.z - b.z); }

inline vector3d operator * (const vector3d& a, vector3d::T d)
{ return vector3d(d * a.x, d * a.y, d * a.z); }

inline vector3d operator * (vector3d::T d, const vector3d& a)
{ return vector3d(d * a.x, d * a.y, d * a.z); }

inline vector3d operator / (const vector3d& a, vector3d::T d)
{ 
  assert( d != 0.0f );
  vector3d::T d_inv = 1.0f/d; 
  return vector3d(a.x*d_inv, a.y*d_inv, a.z*d_inv); 
}

inline bool operator == (const vector3d& a, const vector3d& b)
{ return (a.x == b.x) && (a.y == b.y) && (a.z == b.z); }

inline bool operator != (const vector3d& a, const vector3d& b)
{ return (a.x != b.x) || (a.y != b.y) || (a.z != b.z); }

// Dot product
inline vector3d::T dot(const vector3d& a, const vector3d& b)
{
//  START_PROF_TIMER( proftimer_vector_math );
  vector3d::T t = a.x*b.x + a.y*b.y + a.z*b.z;
//  STOP_PROF_TIMER( proftimer_vector_math );
  return t;
}

// vector cross product
inline vector3d cross(const vector3d& a, const vector3d& b)
{
//  START_PROF_TIMER( proftimer_vector_math );
  vector3d v( a.y * b.z - a.z * b.y,
              a.z * b.x - a.x * b.z,
              a.x * b.y - a.y * b.x );
//  STOP_PROF_TIMER( proftimer_vector_math );
  return v;
}

inline vector3d lerp( vector3d a, vector3d b, float l )
{
  return a + ( b - a ) * l;
}

// Build from the cross-product of two vectors constructed from three points
inline vector3d get_triangle_normal(const vector3d& p1, const vector3d& p2, const vector3d& p3)
{ 
  vector3d e0(p1-p2); 
  vector3d e1(p3-p2); 
  vector3d n = cross(e0,e1); 
  n.normalize();  // this may not always be desirable!
  return n;
}

inline vector3d vector3d::perp() const  // find an arbitrary unit vector perpendicular to this one
{
  vector3d n;
  // find smallest component and build a unit vector on that axis
  if (x<y)
  {
    if (x<z)
    {
      n=vector3d(1,0,0);
    }
    else
    {
      n=vector3d(0,0,1);
    }
  }
  else
  {
    if (y<z)
    {
      n=vector3d(0,1,0);
    }
    else
    {
      n=vector3d(0,0,1);
    }
  }
  n = cross(n,*this);
  n.normalize();
  return n;
}

// Inexact equality based on distanct
inline bool approx_equals(const vector3d& a, const vector3d& b, vector3d::T dist)
{
  return (a - b).length2() <= dist*dist;
}

inline rational_t angle_between( const vector3d& v1, const vector3d& v2 )
{
  // because dot(v1,v2) = |v1||v2|cos theta
  // theta = arccos( dot(v1,v2) / |v1||v2| )
  vector3d::T numerator = dot( v1, v2 );
  vector3d::T denominator = v1.length()*v2.length();
  if ( denominator == 0.0f )
    return 0.0f;
  return (rational_t)fast_acos( numerator/denominator );
}

// Return a stringx describing the vector.
inline stringx v3tos(const vector3d& v)
{
  return stringx("<")+ftos(v.x)+","+ftos(v.y)+","+ftos(v.z)+">";
}

typedef vector3d Vc3;
typedef vector3d Pt3;

// Stream interface
#if !defined(NO_SERIAL_OUT)
inline void serial_out(chunk_file& io, const vector3d& v ) 
{ 
  if (io.get_type()==chunk_file::CFT_TEXT)
  {
    io.get_text()->write( ftos(v.x) + " " );
    io.get_text()->write( ftos(v.y) + " " );
    io.get_text()->write( ftos(v.z) + sendl );
  }
  else
    io.get_binary()->write((void*)&v,sizeof(vector3d));
}
#endif


#if !defined(NO_SERIAL_IN)
inline void serial_in(chunk_file& io, vector3d* v) 
{
  serial_in(io,&v->x);
  serial_in(io,&v->y);
  serial_in(io,&v->z);
}
#endif

// Standard vectors
const vector3d XVEC(1,0,0);
const vector3d YVEC(0,1,0);
const vector3d ZVEC(0,0,1);
const vector3d ZEROVEC(0,0,0);


class vector4d
{
public:
  typedef rational_t T;
  T x,y,z,w;

  // constructors
  inline vector4d() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {} // uninitialized default ctor
  inline vector4d(const vector4d& v) { x = v.x; y = v.y; z = v.z; w = v.w; } // compiler would auto-generate this copy ctor
  inline vector4d(T _x, T _y, T _z, T _w) { x = _x; y = _y; z = _z; w = _w; }
  
  // make from vector3d, d becomes new W component.
  explicit inline vector4d(const vector3d& v, T d=1.0F)
  { x = v.x; y = v.y; z = v.z; w = d; }

  explicit inline vector4d(T d) { x = y = z = w = d; }  // Initialize all members to d (usually 0.0f)

  inline vector4d& operator	= ( const vector4d& v )	    // Assignment operator
  { x = v.x; y = v.y; z = v.z; w = v.w; return *this; }

  inline vector4d& operator += ( const vector4d& v )	  // vector addition
  { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }

  inline vector4d& operator -= ( const vector4d& v )	  // vector subtraction
  { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }

  inline vector4d& operator *= ( T d )	    // multiplication by a scalar
  { x *= d; y *= d; z *= d; w *= d; return *this; }

  inline vector4d& operator /= ( T d )	    // division by a scalar
  { T d_inv = 1.0f/d; x *= d_inv; y *= d_inv; z *= d_inv; w *= d_inv; return *this; }

  inline T length2() const			// vector squared length 
  { 
    return x*x + y*y + z*z + w*w; 
  }

  inline T length() const			// vector length 
  { return (T)__fsqrt(x*x + y*y + z*z + w*w); }

  inline vector4d& normalize()			    // vector normalize (set length to 1.0f)
  { 
    T l2 = length2();
    if (l2>SMALL_DIST*SMALL_DIST)
      *this *= fast_recip_sqrt(l2); 
    return *this; 
  }
  inline vector4d& homogenize()			    // vector homogenize 4d -> 3d
  {                                     // leaves reciprocal of homogenous w in w
    if (w) w=1.0f/w;
    x*=w; y*=w; z*=w; 
    return *this; 
  }
  inline vector4d& set_length(T newlen=1.0F)		// set vector length 
  { 
    T l2 = length2();
    if (l2>SMALL_DIST*SMALL_DIST)
      *this *= newlen * fast_recip_sqrt(l2); 
    return *this; 
  }

  inline vector4d operator - () const { return vector4d(-x,-y,-z,-w); } // vector reflection

  inline const vector2d& get_xy()  const { return *(vector2d*)&x; }
  inline const vector3d& get_xyz() const { return *(vector3d*)&x; }

  inline const T& operator[](int i) const
  { return (&x)[i]; }

  inline T& operator[](int i)
  { return (&x)[i]; }

  bool is_valid() const { return x>-1e30f && x<1e30f && y>-1e30f && y<1e30f && z>-1e30f && z<1e30f && w>-1e30f && w<1e30f; }
};

inline vector4d operator + (const vector4d& a, const vector4d& b)
{ return vector4d(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }

inline vector4d operator - (const vector4d& a, const vector4d& b)
{ return vector4d(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }

inline vector4d operator * (const vector4d& a, vector4d::T d)
{ return vector4d(d * a.x, d * a.y, d * a.z, d * a.w); }

inline vector4d operator * (vector4d::T d, const vector4d& a)
{ return vector4d(d * a.x, d * a.y, d * a.z, d * a.w); }

inline vector4d operator / (const vector4d& a, vector4d::T d)
{ 
  assert( d != 0.0f );
  vector4d::T d_inv = 1.0f/d; 
  return vector4d(a.x*d_inv, a.y*d_inv, a.z*d_inv, a.w*d_inv); 
}

inline bool operator == (const vector4d& a, const vector4d& b)
{ return (a.x == b.x) && (a.y == b.y) && (a.z == b.z) && (a.w == b.w); }

inline bool operator != (const vector4d& a, const vector4d& b)
{ return (a.x != b.x) || (a.y != b.y) || (a.z != b.z) || (a.w != b.w); }

inline vector4d::T dot(const vector4d& a, const vector4d& b)
{ 
  return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; 
}


// Inexact equality based on distanct
inline bool approx_equals(const vector4d& a, const vector4d& b, vector4d::T dist)
{
  return (a - b).length2() <= dist*dist;
}

inline rational_t angle_between( const vector4d& v1, const vector4d& v2 )
{
  // because dot(v1,v2) = |v1||v2|cos theta
  // theta = arccos( dot(v1,v2) / |v1||v2| )
  vector4d::T numerator = dot( v1, v2 );
  vector4d::T denominator = v1.length()*v2.length();
  if ( denominator == 0.0f )
    return 0.0f;
  return (rational_t)fast_acos( numerator/denominator );
}

// Return a stringx describing the vector.
inline stringx v4tos(const vector4d& v)
{
  return stringx("<")+ftos(v.x)+","+ftos(v.y)+","+ftos(v.z)+","+ftos(v.w)+">";
}

typedef vector4d Vc4;
typedef vector4d Pt4;

// Stream interface
#if !defined(NO_SERIAL_OUT)
inline void serial_out(chunk_file& io, const vector4d& v ) 
{ 
  if (io.get_type()==chunk_file::CFT_TEXT)
  {
    io.get_text()->write( ftos(v.x) + " " );
    io.get_text()->write( ftos(v.y) + " " );
    io.get_text()->write( ftos(v.z) + " " );
    io.get_text()->write( ftos(v.w) + sendl );
  }
  else
    io.get_binary()->write((void*)&v,sizeof(vector4d));
}
#endif


#if !defined(NO_SERIAL_IN)
inline void serial_in(chunk_file& io, vector4d* v) 
{
  serial_in(io,&v->x);
  serial_in(io,&v->y);
  serial_in(io,&v->z);
  serial_in(io,&v->w);
}
#endif


/*---------------------------------------------------------------------------------------------------

  4x4 Matrix

---------------------------------------------------------------------------------------------------*/

class matrix4x4
{
public:
  typedef rational_t T;
  typedef vector4d row;
  // row exists to allow things like my_matrix[3][2] to work. 
  // Accesses should get completely optimized out of the code.

  // JIV FIXME: pragma pack
  row x;
  row y;
  row z;
  row w;

  // Constructors
  inline matrix4x4() :
    x(1.0f, 0.0f, 0.0f, 0.0f),
    y(0.0f, 1.0f, 0.0f, 0.0f),
    z(0.0f, 0.0f, 1.0f, 0.0f),
    w(0.0f, 0.0f, 0.0f, 1.0f)    
    {} // uninitialized ctor

  inline matrix4x4(
        T _00, T _01, T _02, T _03,
        T _10, T _11, T _12, T _13,
        T _20, T _21, T _22, T _23,
        T _30, T _31, T _32, T _33 )
    : x(_00, _01, _02, _03)
    , y(_10, _11, _12, _13)
    , z(_20, _21, _22, _23)
    , w(_30, _31, _32, _33)
  {}

  inline const row & operator[](int i) const
  { return (&x)[i]; }

  inline row & operator[](int i)
  { return (&x)[i]; }

        vector3d& xrow()       { return *(vector3d*)&x; }
  const vector3d& xrow() const { return *(vector3d*)&x; }
        vector3d& yrow()       { return *(vector3d*)&y; }
  const vector3d& yrow() const { return *(vector3d*)&y; }
        vector3d& zrow()       { return *(vector3d*)&z; }
  const vector3d& zrow() const { return *(vector3d*)&z; }
        vector3d& wrow()       { return *(vector3d*)&w; }
  const vector3d& wrow() const { return *(vector3d*)&w; }

public:

  // Set a view-to-screen matrix
  void make_projection(T field_of_view = 1.570795f, T aspect = 1.0f,
    T near_plane = 1.0f, T far_plane = 1000.0f,
    T push = 0.0f); // push everything toward camera by this amount

  void make_frustum(T left=-1.0f,T top=-1.0f,T right=1.0f,T bottom=1.0f,
    T znear=1.0f,T zfar=1000.0f,
    T push=0.0f); // push everything toward camera by this amount

  inline matrix4x4& operator*=( T s )
  {
    x.x *= s;  x.y *= s;  x.z *= s; x.w *= s;
    y.x *= s;  y.y *= s;  y.z *= s; y.w *= s;
    z.x *= s;  z.y *= s;  z.z *= s; z.w *= s;
    w.x *= s;  w.y *= s;  w.z *= s; w.w *= s;
    return *this;
  }

  // define a few useful functions for generating useful matrices
  void make_translate( const vector3d& t );
  void make_rotate( const vector3d& u, T a );
  void make_scale( const vector3d& s );
	void make_mirror( const vector3d& n, T d );
  void identity();

  void translate( const vector3d& t );
  void rotate( const vector3d& u, T a );
  void scale( const vector3d& s );
  void scale( rational_t s );

  // Invert the matrix (from Slava)
  matrix4x4 inverse();
  matrix4x4 transpose();
	matrix4x4 Cof();
	matrix4x4 adjugate();
	T cofactor(int, int) const;
	T det() const;

  // if I, J, and K are the row vectors of the matrix,
  // determinant = I dot ( J cross K )
  T determinant3() const  // determinant of upper-left 3x3
  {
    //return dot(xrow(),cross(yrow(),zrow())), or triple(xrow(),yrow(),zrow());
    return x.x*y.y*z.z - y.x*z.z*x.y +
           y.x*z.y*x.z - z.x*y.y*x.z +
           z.x*x.y*y.z - x.x*z.y*y.z;
  };

  T determinant() const;  // full 4x4 determinant

  // The normalize function updates the current matrix making it an
  // orthogonal matrix with all rows/columns unit length;
  // this is done normalizing every row after subtracting
  // any component from a row in direction of previous rows.
  // this method is biased in favor of retaining the
  // orientation of the z axis
  void orthonormalize();

  bool is_valid( void ) const;
};

/*
static inline matrix4x4 operator*( const matrix4x4& a, const matrix4x4& b )
{
  return matrix4x4(
    a[0][0]*b[0][0] + a[0][1]*b[1][0] + a[0][2]*b[2][0] + a[0][3]*b[3][0],
    a[0][0]*b[0][1] + a[0][1]*b[1][1] + a[0][2]*b[2][1] + a[0][3]*b[3][1],
    a[0][0]*b[0][2] + a[0][1]*b[1][2] + a[0][2]*b[2][2] + a[0][3]*b[3][2],
    a[0][0]*b[0][3] + a[0][1]*b[1][3] + a[0][2]*b[2][3] + a[0][3]*b[3][3],

    a[1][0]*b[0][0] + a[1][1]*b[1][0] + a[1][2]*b[2][0] + a[1][3]*b[3][0],
    a[1][0]*b[0][1] + a[1][1]*b[1][1] + a[1][2]*b[2][1] + a[1][3]*b[3][1],
    a[1][0]*b[0][2] + a[1][1]*b[1][2] + a[1][2]*b[2][2] + a[1][3]*b[3][2],
    a[1][0]*b[0][3] + a[1][1]*b[1][3] + a[1][2]*b[2][3] + a[1][3]*b[3][3],

    a[2][0]*b[0][0] + a[2][1]*b[1][0] + a[2][2]*b[2][0] + a[2][3]*b[3][0],
    a[2][0]*b[0][1] + a[2][1]*b[1][1] + a[2][2]*b[2][1] + a[2][3]*b[3][1],
    a[2][0]*b[0][2] + a[2][1]*b[1][2] + a[2][2]*b[2][2] + a[2][3]*b[3][2],
    a[2][0]*b[0][3] + a[2][1]*b[1][3] + a[2][2]*b[2][3] + a[2][3]*b[3][3],

    a[3][0]*b[0][0] + a[3][1]*b[1][0] + a[3][2]*b[2][0] + a[3][3]*b[3][0],
    a[3][0]*b[0][1] + a[3][1]*b[1][1] + a[3][2]*b[2][1] + a[3][3]*b[3][1],
    a[3][0]*b[0][2] + a[3][1]*b[1][2] + a[3][2]*b[2][2] + a[3][3]*b[3][2],
    a[3][0]*b[0][3] + a[3][1]*b[1][3] + a[3][2]*b[2][3] + a[3][3]*b[3][3]
    );
}
*/

static inline void fast_matrix4x4_mul(matrix4x4 &res, const matrix4x4& a, const matrix4x4& b)
{
  assert(false);
  STUB( "fast_matrix4x4_mul" );
}

inline matrix4x4 operator*( const matrix4x4& a, const matrix4x4& b );

// this is slow:
// constant identity matrix.  to set a matrix to the identity, you simply say:
// matrix4x4 my_matrix = identity_matrix;
const matrix4x4 identity_matrix(
  1.0f, 0.0f, 0.0f, 0.0f,
  0.0f, 1.0f, 0.0f, 0.0f,
  0.0f, 0.0f, 1.0f, 0.0f,
  0.0f, 0.0f, 0.0f, 1.0f );

vector3d xform3d_0( const matrix4x4& m44, const vector3d& sv );
vector3d xform3d_1( const matrix4x4& m44, const vector3d& sv );
vector3d xform3d_1_homog( const matrix4x4& m44, const vector3d& sv );
vector4d xform4d( const matrix4x4& m44, const vector4d& sv );

// quaternion section (note: these are designed only to be unit quaternions)
// 
// unit quaternion representing rotation of R radians about the axis [x y z]:
//   a = cos(R/2)
//   b = x * sin(R/2)
//   c = y * sin(R/2)
//   d = z * sin(R/2)

// We should probably derive this from vector4d instead
class quaternion
{
public:
  inline quaternion() { a=1.0f; b=c=d=0.0f; }  // identity
  inline quaternion( rational_t _a, rational_t _b, rational_t _c, rational_t _d )
    : a(_a), b(_b), c(_c), d(_d) 
  {}
  // construct a quaternion from the upper left 3x3 rotation matrix
  quaternion(const matrix4x4 &M);

  //  get square length of quaternion
  inline rational_t length2() const
  { return a*a + b*b + c*c + d*d; }

  // return the conjugate of a quaternion
  inline quaternion get_conjugate() const;
  // multiply by the given quaternion
  inline quaternion& operator*=( const quaternion& B );
  // multiply two quaternions (equivalent to applying successive rotations)
  inline quaternion operator*( const quaternion& B ) const;
  // normalize quaternion
  inline void normalize();
  // derive rotation matrix equivalent of quaternion
  void to_matrix(matrix4x4* dest) const;

  bool is_valid( void ) const;

  rational_t a;// __attribute__((aligned(16))); // for PS2 speedups
  rational_t b;
  rational_t c;
  rational_t d;
};

inline bool operator==( const quaternion& q1, const quaternion& q2 )
{
  return ( (q1.a==q2.a)&&(q1.b==q2.b)&&(q1.c==q2.c)&&(q1.d==q2.d) );
}

quaternion slerp( const quaternion &q0, const quaternion &q1, rational_t t );

inline quaternion quaternion::get_conjugate() const
{
  //return quaternion( a, -b, -c, -d ); // slower
  return quaternion( -a, b, c, d ); 
}

inline quaternion quaternion::operator*( const quaternion& B ) const
{
  return quaternion( a*B.a - b*B.b - c*B.c - d*B.d,
                     a*B.b + b*B.a + c*B.d - d*B.c,
                     a*B.c - b*B.d + c*B.a + d*B.b,
                     a*B.d + b*B.c - c*B.b + d*B.a );
}

inline quaternion& quaternion::operator*=( const quaternion& B )
{
  return *this = *this * B;
}

inline void quaternion::normalize()
{ 
    rational_t l2 = length2();
	rational_t len;
    if (l2>SMALL_DIST*SMALL_DIST)
	{
      len = fast_recip_sqrt(l2);
	  a *= len;
	  b *= len;
	  c *= len;
	  d *= len;
	}
}

// Stream interface
#if !defined(NO_SERIAL_OUT)
inline void serial_out( chunk_file& io, const quaternion& q ) 
{
  if ( io.get_type() == chunk_file::CFT_TEXT )
  {
    io.get_text()->write( ftos(q.a) + " " );
    io.get_text()->write( ftos(q.b) + " " );
    io.get_text()->write( ftos(q.c) + " " );
    io.get_text()->write( ftos(q.d) + sendl );
  }
  else
    io.get_binary()->write( (void*)&q, sizeof(quaternion) );
}
#endif


#if !defined(NO_SERIAL_IN)
inline void serial_in( chunk_file& io, quaternion* q ) 
{
  if ( io.get_type() == chunk_file::CFT_TEXT )
  {
    serial_in( io, &q->a);
    serial_in( io, &q->b);
    serial_in( io, &q->c);
    serial_in( io, &q->d);
  }
  else
    io.get_binary()->read( q, sizeof(quaternion) );
}
#endif


/*******************************************************************************
Calculates and returns point on the line segment, closest to given point. Line
segment represented by origin (o) and line segment vector (v).
********************************************************************************/
vector3d point_line_closest_point( const vector3d& point, const vector3d& o, const vector3d& v, bool clip_edges = false, rational_t *pt = NULL );

#endif // ALGEBRA_H
