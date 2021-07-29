#include "global.h"

#include "hwmath.h"
#include "profiler.h"
 

//-----------------------------------------------------------------------------
// Desc: Sets the passed in 4x4 matrix to a perpsective projection matrix built
//       from the field-of-view (fov, in y), aspect ratio, near plane (D),
//       and far plane (F).
//-----------------------------------------------------------------------------
void matrix4x4::make_projection(  
                  rational_t field_of_view,
                  rational_t aspect,
                  rational_t near_plane,
                  rational_t far_plane,
                  rational_t push) // push everything toward camera by this amount
{

  assert( (far_plane-near_plane) >= 0.01f );

  x.y = x.z = x.w = 
  y.x = y.z = y.w =
  z.x = z.y = 
  w.x = w.y = 0.0F;

  // projection assuming a cononical view space
  // this is w-friendly
  x.x = 1.0F/tan(field_of_view*0.5F);
  y.y = x.x/aspect,
  z.z = far_plane/(far_plane-near_plane);
  z.w = 1.0F;
  w.z = (push-near_plane)*z.z;
  w.w = 0.0f;//push;
}

void matrix4x4::make_frustum(rational_t left,rational_t top,rational_t right,rational_t bottom,
                             rational_t znear,rational_t zfar,rational_t push) 
{
  T xq=1.0f/(right-left);
  T yq=1.0f/(bottom-top);
  T zq=zfar/(zfar-znear);

  assert( (zfar-znear) >= 0.01f );

  x.y = x.z = x.w = 
  y.x = y.z = y.w =
  w.x = w.y = 0.0F;

  // this is w-friendly
  x.x = 2.0F*znear*xq;
  y.y = 2.0F*znear*yq;
  z.x = (right+left)*xq;
  z.y = (bottom+top)*yq;
  z.z = zq;
  z.w = 1.0F;
  w.z = (push-znear)*zq;
  w.w = 0.0f;//push;
}


#if !defined(TARGET_MKS) && !defined(TARGET_PS2)
matrix4x4 operator*( const matrix4x4& a, const matrix4x4& b ) 
{
  return matrix4x4(
    a.x.x*b.x.x + a.x.y*b.y.x + a.x.z*b.z.x + a.x.w*b.w.x,
    a.x.x*b.x.y + a.x.y*b.y.y + a.x.z*b.z.y + a.x.w*b.w.y,
    a.x.x*b.x.z + a.x.y*b.y.z + a.x.z*b.z.z + a.x.w*b.w.z,
    a.x.x*b.x.w + a.x.y*b.y.w + a.x.z*b.z.w + a.x.w*b.w.w,

    a.y.x*b.x.x + a.y.y*b.y.x + a.y.z*b.z.x + a.y.w*b.w.x,
    a.y.x*b.x.y + a.y.y*b.y.y + a.y.z*b.z.y + a.y.w*b.w.y,
    a.y.x*b.x.z + a.y.y*b.y.z + a.y.z*b.z.z + a.y.w*b.w.z,
    a.y.x*b.x.w + a.y.y*b.y.w + a.y.z*b.z.w + a.y.w*b.w.w,

    a.z.x*b.x.x + a.z.y*b.y.x + a.z.z*b.z.x + a.z.w*b.w.x,
    a.z.x*b.x.y + a.z.y*b.y.y + a.z.z*b.z.y + a.z.w*b.w.y,
    a.z.x*b.x.z + a.z.y*b.y.z + a.z.z*b.z.z + a.z.w*b.w.z,
    a.z.x*b.x.w + a.z.y*b.y.w + a.z.z*b.z.w + a.z.w*b.w.w,

    a.w.x*b.x.x + a.w.y*b.y.x + a.w.z*b.z.x + a.w.w*b.w.x,
    a.w.x*b.x.y + a.w.y*b.y.y + a.w.z*b.z.y + a.w.w*b.w.y,
    a.w.x*b.x.z + a.w.y*b.y.z + a.w.z*b.z.z + a.w.w*b.w.z,
    a.w.x*b.x.w + a.w.y*b.y.w + a.w.z*b.z.w + a.w.w*b.w.w
/*
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
*/
    );
}
#endif

void matrix4x4::make_translate( const vector3d& t )
{
  x.x = 1.0f; x.y = 0.0f; x.z = 0.0f; x.w = 0.0F;
  y.x = 0.0f; y.y = 1.0f; y.z = 0.0f; y.w = 0.0F;
  z.x = 0.0f; z.y = 0.0f; z.z = 1.0f; z.w = 0.0F;
  w.x = t.x;  w.y = t.y;  w.z = t.z;  w.w = 1.0F;
}

void matrix4x4::make_rotate( const vector3d& u, rational_t a )
{
  rational_t sa, ca;

  fast_sin_cos_approx( a, &sa, &ca );

  x.x = u.x * u.x + ca * (1.0f - u.x * u.x);
  y.x = u.x * u.y * (1.0f - ca) - u.z * sa;
  z.x = u.x * u.z * (1.0f - ca) + u.y * sa;
  w.x = 0.0f;
  x.y = u.x * u.y * (1.0f - ca) + u.z * sa;
  y.y = u.y * u.y + ca * (1.0f - u.y * u.y);
  z.y = u.y * u.z * (1.0f - ca) - u.x * sa;
  w.y = 0.0f;
  x.z = u.x * u.z * (1.0f - ca) - u.y * sa;
  y.z = u.y * u.z * (1.0f - ca) + u.x * sa;
  z.z = u.z * u.z + ca * (1.0f - u.z * u.z);
  w.z = 0.0f;
  x.w = 0.0f;
  y.w = 0.0f;
  z.w = 0.0f;
  w.w = 1.0f;
}

void matrix4x4::make_scale( const vector3d& s )
{
  x.x = s.x;
  y.x = 0.0f;
  z.x = 0.0f;
  w.x = 0.0f;
  x.y = 0.0f;
  y.y = s.y;
  z.y = 0.0f;
  w.y = 0.0f;
  x.z = 0.0f;
  y.z = 0.0f;
  z.z = s.z;
  w.z = 0.0f;
  x.w = 0.0f;
  y.w = 0.0f;
  z.w = 0.0f;
  w.w = 1.0f;
}

void matrix4x4::make_mirror( const vector3d& n, rational_t d )
{
	x.x = -(n.x * n.x) + (n.y * n.y) + (n.z * n.z);
	y.x = -2.0f * n.x * n.y;
	z.x = -2.0f * n.x * n.z;
	w.x = -2.0f * n.x * d;

	x.y = -2.0f * n.y * n.x;
	y.y = (n.x * n.x) - (n.y * n.y) + (n.z * n.z);
	z.y = -2.0f * n.y * n.z;
	w.y = -2.0f * n.y * d;

	x.z = -2.0f * n.z * n.x;
	y.z = -2.0f * n.z * n.y;
	z.z = (n.x * n.x) + (n.y * n.y) - (n.z * n.z);
	w.z = -2.0f * n.z * d;

	x.w = 0.0f;
	y.w = 0.0f;
	z.w = 0.0f;
	w.w = 1.0f;
}

void matrix4x4::identity()
{
  x.x = 1.0f; x.y = 0.0f; x.z = 0.0f; x.w = 0.0f;
  y.x = 0.0f; y.y = 1.0f; y.z = 0.0f; y.w = 0.0f;
  z.x = 0.0f; z.y = 0.0f; z.z = 1.0f; z.w = 0.0f;
  w.x = 0.0f; w.y = 0.0f; w.z = 0.0f; w.w = 1.0f;
}


void matrix4x4::translate( const vector3d& t )
{
  w.x += t.x;
  w.y += t.y;
  w.z += t.z;
}

void matrix4x4::rotate( const vector3d& u, T a )
{
  matrix4x4 r;
  r.make_rotate(u,a);
  *this = *this * r;
}

void matrix4x4::scale( const vector3d& s )
{
  x.x *= s.x; y.x *= s.x; z.x *= s.x; w.x *= s.x;
  x.y *= s.y; y.y *= s.y; z.y *= s.y; w.y *= s.y;
  x.z *= s.z; y.z *= s.z; z.z *= s.z; w.z *= s.z;
}

void matrix4x4::scale( rational_t s )
{
  x.x *= s; y.x *= s; z.x *= s; w.x *= s;
  x.y *= s; y.y *= s; z.y *= s; w.y *= s;
  x.z *= s; y.z *= s; z.z *= s; w.z *= s;
}

////////////////////////////////////////////////////////////////////////////////
//  quaternion 
////////////////////////////////////////////////////////////////////////////////
quaternion::quaternion(const matrix4x4 &M)
{
#if defined(TARGET_XBOX)
  assert( M.is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

  int        i, j, k;
  rational_t tr, s, q[4];

  tr = M.x.x + M.y.y + M.z.z;

#if defined(TARGET_XBOX)
  assert(!_isnan( tr ));
#endif /* TARGET_XBOX JIV DEBUG */
  
  if(tr > 0.0f)
  {
    s = sqrtf(tr + 1.0f);
#if defined(TARGET_XBOX)
    assert(!_isnan(s));
#endif /* TARGET_XBOX JIV DEBUG */
    a = s * 0.5f;
#if defined(TARGET_XBOX)
    assert( s );
#endif /* TARGET_XBOX JIV DEBUG */
    s = 0.5f / s;


    b = (M.z.y - M.y.z) * s;
    c = (M.x.z - M.z.x) * s;
    d = (M.y.x - M.x.y) * s;
  }
  else
  {
    i = 0; j = 1; k = 2;
    if(M.y.y > M.x.x){ i = 1; j = 2; k = 0; }
    if(M.z.z > M[i][i]){ i = 2; j = 0; k = 1; }

    s = sqrtf(M[i][i] - M[j][j] - M[k][k] + 1.0f);
#if defined(TARGET_XBOX)
    assert(!_isnan(s));
#endif /* TARGET_XBOX JIV DEBUG */
    q[i + 1] = s * 0.5f;
#if defined(TARGET_XBOX)
    assert(!_isnan(q[i+1]));
#endif /* TARGET_XBOX JIV DEBUG */
    if(s != 0.0f) s = 0.5f / s;
    q[0] = (M[k][j] - M[j][k]) * s;

#if defined(TARGET_XBOX)
    assert(!_isnan(q[0]));
#endif /* TARGET_XBOX JIV DEBUG */
    
    q[j + 1] = (M[j][i] + M[i][j]) * s;
    q[k + 1] = (M[k][i] + M[i][k]) * s;

    a = q[0];
    b = q[1];
    c = q[2];
    d = q[3];
  }

#if defined(TARGET_XBOX)
  assert( is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */
}


void quaternion::to_matrix(matrix4x4* M) const
{
  rational_t ab, ac, ad, bb, bc, bd, cc, cd, dd, b2, c2, d2;

#if defined(TARGET_XBOX)
  assert( is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */
  
  b2 = b + b;
  c2 = c + c;
  d2 = d + d;

  ab = a * b2;
  ac = a * c2;
  ad = a * d2;
  bb = b * b2;
  bc = b * c2;
  bd = b * d2;
  cc = c * c2;
  cd = c * d2;
  dd = d * d2;

  (*M).x.x = 1.0f - (cc + dd);
  (*M).x.y = bc - ad;
  (*M).x.z = bd + ac;
  (*M).x.w = 0.0f;

  (*M).y.x = bc + ad;
  (*M).y.y = 1.0f - (bb + dd);
  (*M).y.z = cd - ab;
  (*M).y.w = 0.0f;

  (*M).z.x = bd - ac;
  (*M).z.y = cd + ab;
  (*M).z.z = 1.0f - (bb + cc);
  (*M).z.w = 0.0f;

  (*M).w.x = 0.0f;
  (*M).w.y = 0.0f;
  (*M).w.z = 0.0f;
  (*M).w.w = 1.0f;

#if defined(TARGET_XBOX)
  assert( M->is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */
  
}
// Assembly bashes VF4-VF14 on VU0 (VF4-VF8 in fast sin)
quaternion slerp( const quaternion &q0, const quaternion &q1, rational_t t )
{
#if defined(TARGET_XBOX)
  assert( q0.is_valid() );
  assert( q1.is_valid() );
  assert(!_isnan(t));
#endif /* TARGET_XBOX JIV DEBUG */

  static quaternion qr;
  static quaternion qp;

  
  rational_t a, ca, s0, s1, isa;

  ca = q0.a * q1.a + q0.b * q1.b + q0.c * q1.c + q0.d * q1.d;
  if ( ca > 0.999999f )
  {
    return q0;
  }

  if ( ca < 0.0f )
  {
    ca = -ca;
    
    if(ca > 1.0f)
      ca = 1.0f;

    qp.a = -q1.a;
    qp.b = -q1.b;
    qp.c = -q1.c;
    qp.d = -q1.d;
  }
  else
  {
    qp.a = q1.a;
    qp.b = q1.b;
    qp.c = q1.c;
    qp.d = q1.d;
  }

  a = fast_acos( ca );
  if ( __fabs(a) < 0.0000001f )
  {
    return q0;
  }

  isa = 1.0f / fast_sin( a ); 
  s0 = fast_sin( (1.0f-t)*a ) * isa;
  s1 = fast_sin( t*a ) * isa;

  qr.a = s0 * q0.a + s1 * qp.a;
  qr.b = s0 * q0.b + s1 * qp.b;
  qr.c = s0 * q0.c + s1 * qp.c;
  qr.d = s0 * q0.d + s1 * qp.d;

#if defined(TARGET_XBOX)
  assert( qr.is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */
  
  return qr;
}

bool quaternion::is_valid( void ) const
{
  return ( a == a ) &&
    ( b == b ) &&
    ( c == c ) &&
    ( d == d );
}

vector3d point_line_closest_point( const vector3d& point, const vector3d& o, const vector3d& v, bool clip_edges, rational_t *pt )
{
  rational_t  t;
  vector3d    n;

  n = -v;
  t = - ( dot(o, n) - dot(point, n) ) / dot( v, n );
  if( pt ) *pt = t;
  if( clip_edges )
  {
    if( t < 0.0f ) t = 0.0f;
    else if( t > 1.0f ) t = 1.0f;
  }
  return o + t * v;
}


// enforces that the matrix upperleft 3x3 is orthogonal and each
// basis vector is unit length
void matrix4x4::orthonormalize() 
{
  zrow().normalize();  // normalize first vector
  // make second vector perpendicular to the first
  (yrow()-=zrow()*dot(zrow(),yrow())).normalize();
  xrow()=cross(yrow(),zrow());  // third vector is first cross second
}


static rational_t det3(const rational_t m[3][3])
{
	return	(m[0][0] * m[1][1] * m[2][2] +
 			     m[0][1] * m[1][2] * m[2][0] +
 			     m[0][2] * m[1][0] * m[2][1] -
 			     m[0][2] * m[1][1] * m[2][0] -
 			     m[0][0] * m[1][2] * m[2][1] -
			     m[0][1] * m[1][0] * m[2][2]);
}

rational_t matrix4x4::determinant() const  // full 4x4 determinant
{
  return y.x*w.z*z.w*x.y - y.x*z.z*w.w*x.y - w.z*y.w*z.x*x.y + z.z*y.w*w.x*x.y -
         x.x*w.z*z.w*y.y + x.x*z.z*w.w*y.y + w.z*x.w*z.x*y.y - z.z*x.w*w.x*y.y -
         y.x*w.z*x.w*z.y + x.x*w.z*y.w*z.y + y.x*z.z*x.w*w.y - x.x*z.z*y.w*w.y -
         w.w*z.x*y.y*x.z + z.w*w.x*y.y*x.z + y.x*w.w*z.y*x.z - y.w*w.x*z.y*x.z -
         y.x*z.w*w.y*x.z + y.w*z.x*w.y*x.z + w.w*z.x*x.y*y.z - z.w*w.x*x.y*y.z -
         x.x*w.w*z.y*y.z + x.w*w.x*z.y*y.z + x.x*z.w*w.y*y.z - x.w*z.x*w.y*y.z;
}


matrix4x4 matrix4x4::inverse()
{
	rational_t det = this->det();

	if(det != 0.0f) 
	  return this->adjugate() *= (1.0f / det);
  else
    return identity_matrix;
}

/*
// The invert function computes the inverse of the current matrix and stores
// the result back in the current matrix. If the current matrix is singular,
// the resulting matrix is identity.
matrix4x4& matrix4x4::invert() 
{
  matrix4x4 b(*this);  //make copy
  matrix4x4& a = *this; //easier to use operator()(r,c) on
  a.identity();

  for (int i=0; i<4; ++i) 
  {
    T val = b(i,i);         // find pivot
    T aval=__fabs(val);
    int ind = i;
    int j;
    for (j=i+1; j<4; ++j) 
    {
      F t=b(j,i);
      F at=__fabs(t);
      if (at > aval) 
      {
        ind = j;
        val = t;
        aval=at;
      }
    }

    if (!aval)  //singular
    {
      a.identity();
      return a;
    }

    if (ind != i)          // swap columns
    {
      for (j=0; j<4; ++j) 
      {
          T val2 = a(i,j);
          a(i,j) = a(ind,j);
          a(ind,j) = val2;
          val2 = b(i,j);
          b(i,j) = b(ind,j);
          b(ind,j) = val2;
      }
    }

    if (val!=F(1)) 
    {
      T ival=F(1)/val;
      for (j=0; j<4; ++j) 
      {
        b(i,j) *= ival;
        a(i,j) *= ival;
      }
    }

    for (j=0; j<4; ++j)        // eliminate column
    {
      if (j == i)
        continue;
      T n = b(j,i);
      if (n) 
      {
        for (int k=0; k<4; ++k) 
        {
            b(j,k) -= b(i,k) * n;
            a(j,k) -= a(i,k) * n;
        }
      }
    }
  }
  return a;
}
*/


matrix4x4 matrix4x4::transpose()
{
  static matrix4x4 loc;

	for(int i = 0; i < 4; ++i)
  {
		for(int j = 0; j < 4; ++j)
    {
			loc[i][j] = (*this)[j][i];
		}
  }

	return loc;
}


matrix4x4::T matrix4x4::cofactor(int x, int y) const
{
	rational_t M[3][3];
	int s = ((x ^ y) & 1)? -1 : 1;

	for(int i = 0, k = 0; i < 4; ++i)
  {
		if(i != x)
    {
			for(int j = 0, l = 0; j < 4; ++j)
      {
        if(j != y)
        { 
          M[k][l] = (*this)[i][j]; 
          ++l;
        }
      }

			++k;
		}
	}

  return s * det3(M);
}


matrix4x4::T matrix4x4::det() const
{
	T s = 0;

	for(int i = 0; i < 4; ++i)
    s += (*this)[0][i] * this->cofactor(0, i);

	return s;
}


matrix4x4 matrix4x4::Cof()
{
	matrix4x4 t;

	for(int i = 0; i < 4; ++i)
  {
    for(int j = 0; j < 4; ++j) 
      t[i][j] = this->cofactor(i, j);
  }

	return t;
}


matrix4x4 matrix4x4::adjugate()
{
  matrix4x4 loc;

	loc = this->Cof();

	return loc.transpose();
}

bool matrix4x4::is_valid( void ) const
{
  return x.is_valid() && y.is_valid() && z.is_valid() && w.is_valid();
}


vector3d xform3d_1_homog( const matrix4x4& m44, const vector3d& sv )
{
  assert( m44.is_valid() );
  assert( sv.is_valid() );

  float w;
  w = m44.x.w * sv.x + m44.y.w * sv.y + m44.z.w * sv.z + m44.w.w;

  assert( w == w );

  if (w)
    w=1.0f/w;

  assert( w == w );
  
  vector3d result;
  result.x = (m44.x.x * sv.x + m44.y.x * sv.y + m44.z.x * sv.z + m44.w.x)*w;
  result.y = (m44.x.y * sv.x + m44.y.y * sv.y + m44.z.y * sv.z + m44.w.y)*w;
  result.z = (m44.x.z * sv.x + m44.y.z * sv.y + m44.z.z * sv.z + m44.w.z)*w;

  assert( result.is_valid() );

  return result;
}

vector4d xform4d( const matrix4x4& m44, const vector4d& sv )
{
  assert( m44.is_valid() );
  assert( sv.is_valid() );

  vector4d result;
  result.x = m44.x.x * sv.x + m44.y.x * sv.y + m44.z.x * sv.z + m44.w.x * sv.w;
  result.y = m44.x.y * sv.x + m44.y.y * sv.y + m44.z.y * sv.z + m44.w.y * sv.w;
  result.z = m44.x.z * sv.x + m44.y.z * sv.y + m44.z.z * sv.z + m44.w.z * sv.w;
  result.w = m44.x.w * sv.x + m44.y.w * sv.y + m44.z.w * sv.z + m44.w.w * sv.w;

  assert( result.is_valid() );
  
  return result;
}

vector3d xform3d_1( const matrix4x4& m44, const vector3d& sv )
{
  assert( m44.is_valid() );
  assert( sv.is_valid() );

  vector3d result;
  result.x = m44.x.x * sv.x + m44.y.x * sv.y + m44.z.x * sv.z + m44.w.x;
  result.y = m44.x.y * sv.x + m44.y.y * sv.y + m44.z.y * sv.z + m44.w.y;
  result.z = m44.x.z * sv.x + m44.y.z * sv.y + m44.z.z * sv.z + m44.w.z;


  assert( result.is_valid());
  
  return result;
}

vector3d xform3d_0( const matrix4x4& m44, const vector3d& sv )
{
  assert( m44.is_valid() );
  assert( sv.is_valid() );

  vector3d result;
  result.x = m44.x.x * sv.x + m44.y.x * sv.y + m44.z.x * sv.z;
  result.y = m44.x.y * sv.x + m44.y.y * sv.y + m44.z.y * sv.z;
  result.z = m44.x.z * sv.x + m44.y.z * sv.y + m44.z.z * sv.z;

  assert( result.is_valid());
  
  return result;
}
