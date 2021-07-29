////////////////////////////////////////////////////////////////////////////////
/*
  po

  "Position and orientation" class, son of po

///////////////
02-08-00 (Jason Bare)
I took out all the references to the matrix using the bracket ([]) operators. It
seemed to cause a big hit on the PS2 to use the bracket operators, so I replaced them all
with the actual matrix members, where possible. I left commented out code using the bracket
operators in case we need to revert back.
///////////////

*/
////////////////////////////////////////////////////////////////////////////////
#include "global.h"

#include "po.h"
#include "oserrmsg.h"

// po_result - used by: inverse, ...
static po po_result = po_identity_matrix;

// This is not labeled const, but remains constant always.  It is here so we can return
// references to an identity matrix cheaply
// Actually, this is currently NEVER USED.
po global_identity_po = po_identity_matrix;

////////////////////////////////////////////////////////////////////////////////
//  orientation
////////////////////////////////////////////////////////////////////////////////
orientation::orientation()
{
  for(int row=0;row<2;++row)
    for(int col=0;col<2;++col)
      cells[row][col] = identity_matrix[row][col];
}


////////////////////////////////////////////////////////////////////////////////
//  po
////////////////////////////////////////////////////////////////////////////////


// construct a po from position, rotation and scale values
po::po( const vector3d& p, const quaternion& r, rational_t s )
{
  r.to_matrix( &m );
  m.scale(s);  // OK to apply scale after rotation in the case of uniform scaling
  set_position( p );

#if defined(CHECK_PO_VALIDITY)
  assert( is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */
}


bool po::operator==( const po& b ) const
{
#if defined(CHECK_PO_VALIDITY)
  assert( b.is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

  return memcmp(&m,&b.m,sizeof(m))==0;
}


#if !defined(TARGET_MKS) && !defined(TARGET_PS2)

po po::operator*( const po & b ) const
{
  po local( m.x.x*b.m.x.x + m.x.y*b.m.y.x + m.x.z*b.m.z.x,
            m.x.x*b.m.x.y + m.x.y*b.m.y.y + m.x.z*b.m.z.y,
            m.x.x*b.m.x.z + m.x.y*b.m.y.z + m.x.z*b.m.z.z,
            //0,
            m.y.x*b.m.x.x + m.y.y*b.m.y.x + m.y.z*b.m.z.x,
            m.y.x*b.m.x.y + m.y.y*b.m.y.y + m.y.z*b.m.z.y,
            m.y.x*b.m.x.z + m.y.y*b.m.y.z + m.y.z*b.m.z.z,
            //0,
            m.z.x*b.m.x.x + m.z.y*b.m.y.x + m.z.z*b.m.z.x,
            m.z.x*b.m.x.y + m.z.y*b.m.y.y + m.z.z*b.m.z.y,
            m.z.x*b.m.x.z + m.z.y*b.m.y.z + m.z.z*b.m.z.z,
            //0,
            m.w.x*b.m.x.x + m.w.y*b.m.y.x + m.w.z*b.m.z.x + m.w.w*b.m.w.x,
            m.w.x*b.m.x.y + m.w.y*b.m.y.y + m.w.z*b.m.z.y + m.w.w*b.m.w.y,
            m.w.x*b.m.x.z + m.w.y*b.m.y.z + m.w.z*b.m.z.z + m.w.w*b.m.w.z//,
            //1
            );
/*
  po local( m[0][0]*b.m[0][0] + m[0][1]*b.m[1][0] + m[0][2]*b.m[2][0],
            m[0][0]*b.m[0][1] + m[0][1]*b.m[1][1] + m[0][2]*b.m[2][1],
            m[0][0]*b.m[0][2] + m[0][1]*b.m[1][2] + m[0][2]*b.m[2][2],
            //0,
            m[1][0]*b.m[0][0] + m[1][1]*b.m[1][0] + m[1][2]*b.m[2][0],
            m[1][0]*b.m[0][1] + m[1][1]*b.m[1][1] + m[1][2]*b.m[2][1],
            m[1][0]*b.m[0][2] + m[1][1]*b.m[1][2] + m[1][2]*b.m[2][2],
            //0,
            m[2][0]*b.m[0][0] + m[2][1]*b.m[1][0] + m[2][2]*b.m[2][0],
            m[2][0]*b.m[0][1] + m[2][1]*b.m[1][1] + m[2][2]*b.m[2][1],
            m[2][0]*b.m[0][2] + m[2][1]*b.m[1][2] + m[2][2]*b.m[2][2],
            //0,
            m[3][0]*b.m[0][0] + m[3][1]*b.m[1][0] + m[3][2]*b.m[2][0] + m[3][3]*b.m[3][0],
            m[3][0]*b.m[0][1] + m[3][1]*b.m[1][1] + m[3][2]*b.m[2][1] + m[3][3]*b.m[3][1],
            m[3][0]*b.m[0][2] + m[3][1]*b.m[1][2] + m[3][2]*b.m[2][2] + m[3][3]*b.m[3][2]//,
            //1
            );
*/
#if defined(CHECK_PO_VALIDITY)
  assert( local.is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

  return local;
}


#endif

vector3d po::slow_xform(const vector3d& sv) const
{
#if defined(TARGET_MKS) && 1
  if((uint32(&m)&7)==0)
    asm_m4x4_load_aligned(&m);
  else
    asm_m4x4_load(&m);
  vector3d result;
  asm_ftrv_3_1(sv.x,sv.y,sv.z,&result);
  return result;
#else
#if defined(CHECK_PO_VALIDITY)
  assert( sv.is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

  vector3d result;

  result.x = m.x.x * sv.x + m.y.x * sv.y + m.z.x * sv.z + m.w.x;
  result.y = m.x.y * sv.x + m.y.y * sv.y + m.z.y * sv.z + m.w.y;
  result.z = m.x.z * sv.x + m.y.z * sv.y + m.z.z * sv.z + m.w.z;
/*
  result.x = m[0][0] * sv.x + m[1][0] * sv.y + m[2][0] * sv.z + m[3][0];
  result.y = m[0][1] * sv.x + m[1][1] * sv.y + m[2][1] * sv.z + m[3][1];
  result.z = m[0][2] * sv.x + m[1][2] * sv.y + m[2][2] * sv.z + m[3][2];
*/
  #ifdef DEBUG
    rational_t w;
    w        = m[0][3] * sv.x + m[1][3] * sv.y + m[2][3] * sv.z + m[3][3];
    assert( (w>0.999f)&&(w<1.001f) );  // fudge factor for floating point error
  #endif

#if defined(CHECK_PO_VALIDITY)
    assert( is_valid() );
    assert( result.is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

  return result;
#endif
}

//-----------------------------------------------------------------------------
// Name: non_affine_slow_xform
// Desc: Like slow_xform (above) but ignores the affine part.
//-----------------------------------------------------------------------------
vector3d po::non_affine_slow_xform(const vector3d& sv) const
{
#if defined(CHECK_PO_VALIDITY)
  assert( is_valid() );
  assert( sv.is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

#if defined(TARGET_MKS) && 1
  if((uint32(&m)&7)==0)
    asm_m4x4_load_aligned(&m);
  else
    asm_m4x4_load(&m);
  vector3d result;
  asm_ftrv_3_0(sv.x,sv.y,sv.z,&result);
  return result;
#else
  vector3d result;

  result.x = m.x.x * sv.x + m.y.x * sv.y + m.z.x * sv.z;
  result.y = m.x.y * sv.x + m.y.y * sv.y + m.z.y * sv.z;
  result.z = m.x.z * sv.x + m.y.z * sv.y + m.z.z * sv.z;

#if defined(CHECK_PO_VALIDITY)
  assert( result.is_valid() );
  assert( is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

/*
  result.x = m[0][0] * sv.x + m[1][0] * sv.y + m[2][0] * sv.z;
  result.y = m[0][1] * sv.x + m[1][1] * sv.y + m[2][1] * sv.z;
  result.z = m[0][2] * sv.x + m[1][2] * sv.y + m[2][2] * sv.z;
*/

  return result;
#endif
}




//-----------------------------------------------------------------------------
// Name: set_translate
// Desc: set a translation matrix
//-----------------------------------------------------------------------------
void po::set_translate( const vector3d& translation )
{
  m = identity_matrix;

  m.w.x = translation.x;
  m.w.y = translation.y;
  m.w.z = translation.z;

#if defined(CHECK_PO_VALIDITY)
  assert( is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

/*
  m[3][0] = translation.x;
  m[3][1] = translation.y;
  m[3][2] = translation.z;
*/
}


//-----------------------------------------------------------------------------
// Name: set_scale
// Desc: set a scale matrix
//-----------------------------------------------------------------------------
void po::set_scale( const vector3d& scale )
{
  m = identity_matrix;

  m.x.x = scale.x;
  m.y.y = scale.y;
  m.z.z = scale.z;

#if defined(CHECK_PO_VALIDITY)
  assert( is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

/*
  m[0][0] = scale.x;
  m[1][1] = scale.y;
  m[2][2] = scale.z;
*/
}


//-----------------------------------------------------------------------------
// Name: scale3x3
// Desc: scale only the rotation components of the matrix
//-----------------------------------------------------------------------------
void po::scale3x3( rational_t scale )
{
  m.x.x *= scale; m.y.x *= scale; m.z.x *= scale;
  m.x.y *= scale; m.y.y *= scale; m.z.y *= scale;
  m.x.z *= scale; m.y.z *= scale; m.z.z *= scale;
}


//-----------------------------------------------------------------------------
// Name: set_rotate_x()
// Desc: Create Rotation matrix about X axis
//-----------------------------------------------------------------------------
void po::set_rotate_x( rational_t rads )
{
  m = identity_matrix;
  float sine;
  float cosine;
  fast_sin_cos_approx( rads, &sine, &cosine );

  m.y.y =  cosine;
  m.y.z = -sine;
  m.z.y =  sine;
  m.z.z =  cosine;

#if defined(CHECK_PO_VALIDITY)
  assert( is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

/*
  m[1][1] =  cosine;
  m[1][2] = -sine;
  m[2][1] =  sine;
  m[2][2] =  cosine;
*/
}




//-----------------------------------------------------------------------------
// Name: set_rotate_y()
// Desc: Create Rotation matrix about Y axis
//-----------------------------------------------------------------------------
void po::set_rotate_y( rational_t rads )
{
  m = identity_matrix;
  float sine;
  float cosine;
  fast_sin_cos_approx( rads, &sine, &cosine );

  m.x.x =  cosine;
  m.x.z =  sine;
  m.z.x = -sine;
  m.z.z =  cosine;


#if defined(CHECK_PO_VALIDITY)
  assert( is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

/*
  m[0][0] =  cosine;
  m[0][2] =  sine;
  m[2][0] = -sine;
  m[2][2] =  cosine;
*/
}




//-----------------------------------------------------------------------------
// Name: set_rotate_z()
// Desc: Create Rotation matrix about Z axis
//-----------------------------------------------------------------------------
void po::set_rotate_z( rational_t rads )
{
  m = identity_matrix;
  float sine;
  float cosine;
  fast_sin_cos_approx( rads, &sine, &cosine );

  m.x.x  =  cosine;
  m.x.y  = -sine;
  m.y.x  =  sine;
  m.y.y  =  cosine;

#if defined(CHECK_PO_VALIDITY)
  assert( is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

/*
  m[0][0]  =  cosine;
  m[0][1]  = -sine;
  m[1][0]  =  sine;
  m[1][1]  =  cosine;
*/
}




//-----------------------------------------------------------------------------
// Name: set_rot
// Desc: Create a Rotation matrix about vector direction
//-----------------------------------------------------------------------------
void po::set_rot( const vector3d& rot_dir, rational_t rads )
{
#if defined(CHECK_PO_VALIDITY)
  assert( rot_dir.is_valid() );
#endif
#if defined(TARGET_XBOX)
  assert( !_isnan(rads));
  assert(_finite(rads));
#endif /* TARGET_XBOX JIV DEBUG */

  rational_t     fCos;
  rational_t     fSin;

  fast_sin_cos_approx( rads, &fSin, &fCos );

  rational_t vx,vy,vz;

  rational_t recip_norm = fast_recip_length( rot_dir.x, rot_dir.y, rot_dir.z );

  vx = rot_dir.x * recip_norm;
  vy = rot_dir.y * recip_norm;
  vz = rot_dir.z * recip_norm;

  rational_t _1_minus_fCos = ( 1.0f - fCos );

  m.x.x = ( vx * vx ) * _1_minus_fCos + fCos;
  m.x.y = ( vx * vy ) * _1_minus_fCos - (vz * fSin);
  m.x.z = ( vx * vz ) * _1_minus_fCos + (vy * fSin);

  m.y.x = ( vy * vx ) * _1_minus_fCos + (vz * fSin);
  m.y.y = ( vy * vy ) * _1_minus_fCos + fCos ;
  m.y.z = ( vy * vz ) * _1_minus_fCos - (vx * fSin);

  m.z.x = ( vz * vx ) * _1_minus_fCos - (vy * fSin);
  m.z.y = ( vz * vy ) * _1_minus_fCos + (vx * fSin);
  m.z.z = ( vz * vz ) * _1_minus_fCos + fCos;

  m.x.w = m.y.w = m.z.w = 0.0f;
  m.w.x = m.w.y = m.w.z = 0.0f;
  m.w.w = 1.0f;

/*
  m[0][0] = ( vx * vx ) * ( 1.0f - fCos ) + fCos;
  m[0][1] = ( vx * vy ) * ( 1.0f - fCos ) - (vz * fSin);
  m[0][2] = ( vx * vz ) * ( 1.0f - fCos ) + (vy * fSin);

  m[1][0] = ( vy * vx ) * ( 1.0f - fCos ) + (vz * fSin);
  m[1][1] = ( vy * vy ) * ( 1.0f - fCos ) + fCos ;
  m[1][2] = ( vy * vz ) * ( 1.0f - fCos ) - (vx * fSin);

  m[2][0] = ( vz * vx ) * ( 1.0f - fCos ) - (vy * fSin);
  m[2][1] = ( vz * vy ) * ( 1.0f - fCos ) + (vx * fSin);
  m[2][2] = ( vz * vz ) * ( 1.0f - fCos ) + fCos;

  m[0][3] = m[1][3] = m[2][3] = 0.0f;
  m[3][0] = m[3][1] = m[3][2] = 0.0f;
  m[3][3] = 1.0f;
*/
#if defined(CHECK_PO_VALIDITY)
  assert( is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */
}


//-----------------------------------------------------------------------------
// Name: set_rot
// Desc: Create a Rotation matrix about vector rot_dir, whose angle is the
// magnitude of that vector
//-----------------------------------------------------------------------------
void po::set_rot( const vector3d& rot_dir )
{
#if defined(CHECK_PO_VALIDITY)
  assert(rot_dir.is_valid());
#endif /* TARGET_XBOX JIV DEBUG */

  vector3d       v      = rot_dir;
  rational_t     rads   = v.length();
  rational_t     fCos, fSin;
  fast_sin_cos_approx( rads, &fSin, &fCos );

  if (__fabs(rads)>SMALL_ANGLE)
    v *= 1/rads;

  rational_t _1_minus_fCos = ( 1.0f - fCos );

  m.x.x = ( v.x * v.x ) * _1_minus_fCos + fCos;
  m.x.y = ( v.x * v.y ) * _1_minus_fCos - (v.z * fSin);
  m.x.z = ( v.x * v.z ) * _1_minus_fCos + (v.y * fSin);

  m.y.x = ( v.y * v.x ) * _1_minus_fCos + (v.z * fSin);
  m.y.y = ( v.y * v.y ) * _1_minus_fCos + fCos ;
  m.y.z = ( v.y * v.z ) * _1_minus_fCos - (v.x * fSin);

  m.z.x = ( v.z * v.x ) * _1_minus_fCos - (v.y * fSin);
  m.z.y = ( v.z * v.y ) * _1_minus_fCos + (v.x * fSin);
  m.z.z = ( v.z * v.z ) * _1_minus_fCos + fCos;

  m.x.w = m.y.w = m.z.w = 0.0f;
  m.w.x = m.w.y = m.w.z = 0.0f;
  m.w.w = 1.0f;
/*
  m[0][0] = ( v.x * v.x ) * ( 1.0f - fCos ) + fCos;
  m[0][1] = ( v.x * v.y ) * ( 1.0f - fCos ) - (v.z * fSin);
  m[0][2] = ( v.x * v.z ) * ( 1.0f - fCos ) + (v.y * fSin);

  m[1][0] = ( v.y * v.x ) * ( 1.0f - fCos ) + (v.z * fSin);
  m[1][1] = ( v.y * v.y ) * ( 1.0f - fCos ) + fCos ;
  m[1][2] = ( v.y * v.z ) * ( 1.0f - fCos ) - (v.x * fSin);

  m[2][0] = ( v.z * v.x ) * ( 1.0f - fCos ) - (v.y * fSin);
  m[2][1] = ( v.z * v.y ) * ( 1.0f - fCos ) + (v.x * fSin);
  m[2][2] = ( v.z * v.z ) * ( 1.0f - fCos ) + fCos;

  m[0][3] = m[1][3] = m[2][3] = 0.0f;
  m[3][0] = m[3][1] = m[3][2] = 0.0f;
  m[3][3] = 1.0f;
*/
#if defined(CHECK_PO_VALIDITY)
  assert( is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */
}




//-----------------------------------------------------------------------------
// Name: fixup
// Desc: Forces the so3 portion of the matrix to be a legitimate so3.  This is a
// first pass as compensating for numerical error, and will probably need
// continued attention and improvement.
//-----------------------------------------------------------------------------
void po::fixup()
{
  vector3d new_x(m.x.x,m.y.x,m.z.x);
  vector3d new_y(m.x.y,m.y.y,m.z.y);
  vector3d new_z;

  new_x.normalize();
  new_z = cross(new_x,new_y);
  new_z.normalize();
  new_y = cross(new_z,new_x);
  m.x.x = new_x.x;
  m.y.x = new_x.y;
  m.z.x = new_x.z;
  m.x.y = new_y.x;
  m.y.y = new_y.y;
  m.z.y = new_y.z;
  m.x.z = new_z.x;
  m.y.z = new_z.y;
  m.z.z = new_z.z;
/*
  vector3d new_x(m[0][0],m[1][0],m[2][0]);
  vector3d new_y(m[0][1],m[1][1],m[2][1]);
  vector3d new_z;

  new_x.normalize();
  new_z = cross(new_x,new_y);
  new_z.normalize();
  new_y = cross(new_z,new_x);
  m[0][0] = new_x.x;
  m[1][0] = new_x.y;
  m[2][0] = new_x.z;
  m[0][1] = new_y.x;
  m[1][1] = new_y.y;
  m[2][1] = new_y.z;
  m[0][2] = new_z.x;
  m[1][2] = new_z.y;
  m[2][2] = new_z.z;
*/
#if defined(CHECK_PO_VALIDITY)
  assert( is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */
}


//-----------------------------------------------------------------------------
// Name: add_increment
// Desc: Adds the matrix inc to itself as an delta of position and orientation.
// Unlike simply left multiplying this by inc, the position and orientation must
// be handled separately.
//-----------------------------------------------------------------------------
void po::add_increment( po* inc )
{
  vector3d posn(m.w.x,m.w.y,m.w.z);
  vector3d inc_posn(inc->m.w.x,inc->m.w.y,inc->m.w.z);
  vector3d new_posn = posn+inc_posn;

//  (*this) = (*this) * (*inc);
  fast_po_mul((*this), (*this), (*inc));

  m.w.x = new_posn.x;
  m.w.y = new_posn.y;
  m.w.z = new_posn.z;
/*
  vector3d posn(m[3][0],m[3][1],m[3][2]);
  vector3d inc_posn(inc->m[3][0],inc->m[3][1],inc->m[3][2]);
  vector3d new_posn = posn+inc_posn;

//  (*this) = (*this) * (*inc);
  fast_po_mul((*this), (*this), (*inc));

  m[3][0] = new_posn.x;
  m[3][1] = new_posn.y;
  m[3][2] = new_posn.z;
*/
#if defined(CHECK_PO_VALIDITY)
  assert( is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */
}


//-----------------------------------------------------------------------------
// Name: inverse
// Desc: Computes the inverse of the the transform, stores it in po_result, and
// returns a reference to po_result.
//-----------------------------------------------------------------------------
po & po::inverse() const
{
  // scale stuff is gone
/*  rational_t scale = get_scale();
  scale *= scale;
  scale = 1/scale;*/

  // simple transpose, this won't handle scaling
  po_result.m.x.x = m.x.x; po_result.m.x.y = m.y.x; po_result.m.x.z = m.z.x;
  po_result.m.y.x = m.x.y; po_result.m.y.y = m.y.y; po_result.m.y.z = m.z.y;
  po_result.m.z.x = m.x.z; po_result.m.z.y = m.y.z; po_result.m.z.z = m.z.z;
  po_result.m.w.x = 0.0f; po_result.m.w.y = 0.0f; po_result.m.w.z = 0.0f; po_result.m.w.w = 1.0f;
  po_result.m.x.w = 0.0f; po_result.m.y.w = 0.0f; po_result.m.z.w = 0.0f;
/*
  po_result.m[0][0] = m[0][0]; po_result.m[0][1] = m[1][0]; po_result.m[0][2] = m[2][0];
  po_result.m[1][0] = m[0][1]; po_result.m[1][1] = m[1][1]; po_result.m[1][2] = m[2][1];
  po_result.m[2][0] = m[0][2]; po_result.m[2][1] = m[1][2]; po_result.m[2][2] = m[2][2];
  po_result.m[3][0] = 0; po_result.m[3][1] = 0; po_result.m[3][2] = 0; po_result.m[3][3] = 1;
  po_result.m[0][3] = 0; po_result.m[1][3] = 0; po_result.m[2][3] = 0;
*/

/*  po_result.m[0][0] *= scale;
  po_result.m[0][1] *= scale;
  po_result.m[0][2] *= scale;
  po_result.m[1][0] *= scale;
  po_result.m[1][1] *= scale;
  po_result.m[1][2] *= scale;
  po_result.m[2][0] *= scale;
  po_result.m[2][1] *= scale;
  po_result.m[2][2] *= scale;*/

  vector3d affine = -po_result.non_affine_slow_xform(vector3d(m.w.x,m.w.y,m.w.z));
  po_result.m.w.x = affine.x;
  po_result.m.w.y = affine.y;
  po_result.m.w.z = affine.z;

/*
  vector3d affine = -po_result.non_affine_slow_xform(vector3d(m[3][0],m[3][1],m[3][2]));
  po_result.m[3][0] = affine.x;
  po_result.m[3][1] = affine.y;
  po_result.m[3][2] = affine.z;
*/
#if defined(CHECK_PO_VALIDITY)
  assert( is_valid() );
  assert( po_result.is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */
  return po_result;
}



//-----------------------------------------------------------------------------
// Name: inverse_transform
// Desc: Transforms v by the inverse of the matrix
//-----------------------------------------------------------------------------
/*
vector3d po::inverse_xform( const vector3d & v ) const
{
  assert (m[0][0]>=-1.01f && m[0][0]<= 1.01f);
  vector3d result;
  result.x = m[0][0] * (v.x-m[3][0]) + m[0][1] * (v.y-m[3][1]) + m[0][2] * (v.z-m[3][2]);
  result.y = m[1][0] * (v.x-m[3][0]) + m[1][1] * (v.y-m[3][1]) + m[1][2] * (v.z-m[3][2]);
  result.z = m[2][0] * (v.x-m[3][0]) + m[2][1] * (v.y-m[3][1]) + m[2][2] * (v.z-m[3][2]);
  return result;
}
*/

//-----------------------------------------------------------------------------
// Name: non_affine_inverse_transform
// Desc: Transforms v by the inverse of the matrix, using only
//       the rotational component.
//-----------------------------------------------------------------------------
vector3d po::non_affine_inverse_xform( const vector3d & v ) const
{
  assert (m.x.x>=-1.01f && m.x.x<= 1.01f);
//  assert (m[0][0]>=-1.01f && m[0][0]<= 1.01f);
#if defined(TARGET_MKS)
  asm_po_load_inverse(this);
  vector3d result;
  asm_ftrv_3_0(v.x,v.y,v.z,&result);
  return result;
#else
#if defined(CHECK_PO_VALIDITY)
  assert( v.is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

  vector3d result;
  result.x = m.x.x * v.x + m.x.y * v.y + m.x.z * v.z;
  result.y = m.y.x * v.x + m.y.y * v.y + m.y.z * v.z;
  result.z = m.z.x * v.x + m.z.y * v.y + m.z.z * v.z;

#if defined(CHECK_PO_VALIDITY)
  assert( is_valid() );
  assert( result.is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

  return result;
/*
  vector3d result;
  result.x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z;
  result.y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z;
  result.z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z;
  return result;
*/
#endif
}

//-----------------------------------------------------------------------------
// Name: non_affine_inverse_scaled_transform
// Desc: Transforms v by the inverse of the matrix, using only
//       the scaled rotational component.
//-----------------------------------------------------------------------------
vector3d po::non_affine_inverse_scaled_xform( const vector3d & v ) const
{
#if defined(CHECK_PO_VALIDITY)
  assert( v.is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

  vector3d result;
  result.x = m.x.x * v.x + m.x.y * v.y + m.x.z * v.z;
  result.y = m.y.x * v.x + m.y.y * v.y + m.y.z * v.z;
  result.z = m.z.x * v.x + m.z.y * v.y + m.z.z * v.z;

/*
  vector3d result;
  result.x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z;
  result.y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z;
  result.z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z;
*/
  // CTT 04/14/00: TEMPORARY:
  // This is not terribly efficient, but to fix the problem I need to totally
  // revamp the po class so that it mantains a uniform scale value (which in
  // turn means using a specialized structure instead of matrix4x4).
  rational_t s = get_scale();
  if ( s<0.999f || s>1.001f )
    result *= 1.0f / (s*s);

#if defined(CHECK_PO_VALIDITY)
  assert( result.is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

  return result;
}


orientation po::get_orientation() const
{
  orientation retval;
  for(int row=0;row<3;++row)
    for(int col=0;col<3;++col)
      retval.cells[row][col] = m[row][col];
  return retval;
}

void po::set_orientation(const orientation& o)
{
  for(int row=0;row<3;++row)
    for(int col=0;col<3;++col)
      m[row][col] = o.cells[row][col];

#if defined(CHECK_PO_VALIDITY)
  assert( is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */
}


void po::set_orientation(const po & o)
{
  for(int row=0;row<3;++row)
    for(int col=0;col<3;++col)
      m[row][col]=o.m[row][col];
#if defined(CHECK_PO_VALIDITY)
  assert( is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */
}

void po::set_facing( const vector3d& v )
{
#if defined(CHECK_PO_VALIDITY)
  assert( v.is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

  vector3d zv = v - get_position();
  zv.normalize();
  if(( zv.y > 0.9999f )&&( zv.y < 1.0001f ))
  {
    m.x.x= 1.0f;  m.x.y= 0.0f;  m.x.z= 0.0f;
    m.y.x= 0.0f;  m.y.y= 0.0f;  m.y.z=-1.0f;
    m.z.x= 0.0f;  m.z.y= 1.0f;  m.z.z= 0.0f;
/*
    m[0][0]= 1;  m[0][1]= 0;  m[0][2]= 0;
    m[1][0]= 0;  m[1][1]= 0;  m[1][2]=-1;
    m[2][0]= 0;  m[2][1]= 1;  m[2][2]= 0;
*/
  }
  else if(( zv.y < -0.9999f )&&( zv.y > -1.0001f ))
  {
    m.x.x= 1.0f;  m.x.y= 0.0f;  m.x.z= 0.0f;
    m.y.x= 0.0f;  m.y.y= 0.0f;  m.y.z= 1.0f;
    m.z.x= 0.0f;  m.z.y= -1.0f;  m.z.z= 0.0f;
/*
    m[0][0]= 1;  m[0][1]= 0;  m[0][2]= 0;
    m[1][0]= 0;  m[1][1]= 0;  m[1][2]= 1;
    m[2][0]= 0;  m[2][1]= -1;  m[2][2]= 0;
*/
  }
  else
  {
    vector3d yv(0.0f,1.0f,0.0f);
    vector3d xv = cross( yv, zv );
    xv.normalize();
    yv = cross( zv, xv );
    yv.normalize();

    m.x.x=xv.x;  m.x.y=xv.y;  m.x.z=xv.z;
    m.y.x=yv.x;  m.y.y=yv.y;  m.y.z=yv.z;
    m.z.x=zv.x;  m.z.y=zv.y;  m.z.z=zv.z;
/*
    m[0][0]=xv.x;  m[0][1]=xv.y;  m[0][2]=xv.z;
    m[1][0]=yv.x;  m[1][1]=yv.y;  m[1][2]=yv.z;
    m[2][0]=zv.x;  m[2][1]=zv.y;  m[2][2]=zv.z;
*/
  }
#if defined(CHECK_PO_VALIDITY)
  assert( is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */
}

bool g_ignore_nonuniform_scaling = false;
bool po::has_nonuniform_scaling() const
{
  if(g_ignore_nonuniform_scaling)
    return(false);

  float len2s[3];
	int i;
  for ( i=0; i<3; ++i )
  {
    vector3d p3(m[i].x, m[i].y, m[i].z);
//    vector3d p3(m[i][0], m[i][1], m[i][2]);
    len2s[i] = p3.length2();
  }
  for ( i=0; i<3; ++i )
  {
   // modulo 3 is a slow operation, hopefully the compiler will unroll this loop
    if ( len2s[i]<len2s[(i+1)%3]*0.96f || len2s[i]>len2s[(i+1)%3]*1.04f )
      return true;
//    if (len2s[i]<0.95f || len2s[i]>1.05f)
//      return true;
  }
  return false;
}


quaternion po::get_quaternion() const
{
#if defined(CHECK_PO_VALIDITY)
  assert( is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

  // obtain normalized orientation matrix
  matrix4x4 orient_m = get_matrix();
#if defined(TARGET_XBOX) || defined(TARGET_GC)
  rational_t scale = get_scale();

  rational_t s = (scale == 0.0f) ? 1.0f : (1.0f / scale);
#else
  rational_t s = 1.0f / get_scale();
#endif /* TARGET_XBOX JIV DEBUG */

  orient_m.x.x *= s;  orient_m.x.y *= s;  orient_m.x.z *= s;
  orient_m.y.x *= s;  orient_m.y.y *= s;  orient_m.y.z *= s;
  orient_m.z.x *= s;  orient_m.z.y *= s;  orient_m.z.z *= s;

/*
  matrix4x4 orient_m = get_matrix();
  rational_t s = 1.0f / get_scale();

  orient_m[0][0] *= s;  orient_m[0][1] *= s;  orient_m[0][2] *= s;
  orient_m[1][0] *= s;  orient_m[1][1] *= s;  orient_m[1][2] *= s;
  orient_m[2][0] *= s;  orient_m[2][1] *= s;  orient_m[2][2] *= s;
*/
  // construct quaternion
#if defined(TARGET_XBOX)
  assert( orient_m.is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */
  return quaternion(orient_m);
}


void po::decompose_rot(const vector3d &axis1, const vector3d &axis2, const vector3d &axis3, rational_t *ang1, rational_t *ang2, rational_t *ang3)
{
#if defined(CHECK_PO_VALIDITY)
  assert( axis1.is_valid() );
  assert( axis2.is_valid() );
  assert( axis3.is_valid() );
  assert( is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

  matrix4x4 mat;
  matrix4x4 A1(identity_matrix), B1(identity_matrix), C1(identity_matrix);
  vector3d vec1, vec2, vec3;
  char xfound = 0, yfound = 0, zfound = 0;

  assert((axis1.x == XVEC.x && axis1.y == XVEC.y && axis1.z == XVEC.z) ||
	     (axis1.x == YVEC.x && axis1.y == YVEC.y && axis1.z == YVEC.z) ||
		 (axis1.x == ZVEC.x && axis1.y == ZVEC.y && axis1.z == ZVEC.z) ||
		 (axis1.x == ZEROVEC.x && axis1.y == ZEROVEC.y && axis1.z == ZEROVEC.z));

  assert((axis2.x == XVEC.x && axis2.y == XVEC.y && axis2.z == XVEC.z) ||
	     (axis2.x == YVEC.x && axis2.y == YVEC.y && axis2.z == YVEC.z) ||
		 (axis2.x == ZVEC.x && axis2.y == ZVEC.y && axis2.z == ZVEC.z) ||
		 (axis2.x == ZEROVEC.x && axis2.y == ZEROVEC.y && axis2.z == ZEROVEC.z));

  assert((axis3.x == XVEC.x && axis3.y == XVEC.y && axis3.z == XVEC.z) ||
	     (axis3.x == YVEC.x && axis3.y == YVEC.y && axis3.z == YVEC.z) ||
		 (axis3.x == ZVEC.x && axis3.y == ZVEC.y && axis3.z == ZVEC.z) ||
		 (axis3.x == ZEROVEC.x && axis3.y == ZEROVEC.y && axis3.z == ZEROVEC.z));

  if(axis1.x == 1.0f || axis2.x == 1.0f || axis3.x == 1.0f)
	  xfound = 1;
  if(axis1.y == 1.0f || axis2.y == 1.0f || axis3.y == 1.0f)
	  yfound = 1;
  if(axis1.z == 1.0f || axis2.z == 1.0f || axis3.z == 1.0f)
	  zfound = 1;

  if(axis1.x != 0.0f || axis1.y != 0.0f || axis1.z != 0.0f)
  {
    assert(ang1);

    *ang1 = 0.0f;
    vec1 = axis1;
  }
  else
  {
    if(!xfound)
    {
      vec1 = XVEC;
      xfound = 1;
    }
    else if(!yfound)
    {
      vec1 = YVEC;
      yfound = 1;
    }
    else if(!zfound)
      {
      vec1 = ZVEC;
      zfound = 1;
    }
  }

  if(axis2.x != 0.0f || axis2.y != 0.0f || axis2.z != 0.0f)
  {
    assert(ang2);

    *ang2 = 0.0f;
    vec2 = axis2;
  }
  else
  {
    if(!xfound)
    {
      vec2 = XVEC;
      xfound = 1;
    }
    else if(!yfound)
    {
      vec2 = YVEC;
      yfound = 1;
    }
    else if(!zfound)
    {
      vec2 = ZVEC;
      zfound = 1;
    }
  }

  if(axis3.x != 0.0f || axis3.y != 0.0f || axis3.z != 0.0f)
  {
    assert(ang3);

    *ang3 = 0.0f;
    vec3 = axis3;
  }
  else
  {
    if(!xfound)
    {
      vec3 = XVEC;
      xfound = 1;
    }
    else if(!yfound)
    {
      vec3 = YVEC;
      yfound = 1;
    }
    else if(!zfound)
    {
      vec3 = ZVEC;
      zfound = 1;
    }
  }

  // Conjugate so we can do it in standard x,y,z  (easier)
  matrix4x4 conj_left( vec1.x, vec2.x, vec3.x, 0.0f,
                       vec1.y, vec2.y, vec3.y, 0.0f,
                       vec1.z, vec2.z, vec3.z, 0.0f,
                       0.0f,   0.0f,   0.0f,   1.0f );

  // inverse (transpose)
  matrix4x4 conj_right(conj_left[0][0], conj_left[1][0], conj_left[2][0], conj_left[3][0],
                       conj_left[0][1], conj_left[1][1], conj_left[2][1], conj_left[3][1],
                       conj_left[0][2], conj_left[1][2], conj_left[2][2], conj_left[3][2],
                       conj_left[0][3], conj_left[1][3], conj_left[2][3], conj_left[3][3]);

  mat = conj_right*m;
  mat = mat*conj_left;

  rational_t length2 = 0.0f;

  if ((length2 = mat[1][2]*mat[1][2] + mat[2][2]*mat[2][2]) >= 0.01f)
  {
    rational_t scale = fast_recip_sqrt(length2);

    vec1.x = 0.0f;
    vec1.y = mat[1][2] * scale;
    vec1.z = mat[2][2] * scale;

/*
    int neg_ok = 1;
    if (vec1.z<0 && neg_ok)
      vec1 *= -1;
//*/
    vec2 = cross(vec1, XVEC);

    matrix4x4 tmp_mat( XVEC.x, vec2.x, vec1.x, 0.0f,
                       XVEC.y, vec2.y, vec1.y, 0.0f,
                       XVEC.z, vec2.z, vec1.z, 0.0f,
                       0.0f,   0.0f,   0.0f,   1.0f );

    A1 = tmp_mat;

    matrix4x4 inv_temp(A1[0][0], A1[1][0], A1[2][0], A1[3][0],
                       A1[0][1], A1[1][1], A1[2][1], A1[3][1],
                       A1[0][2], A1[1][2], A1[2][2], A1[3][2],
                       A1[0][3], A1[1][3], A1[2][3], A1[3][3]);

    mat = inv_temp*mat;
  }

  // B1
  vec1.x = mat[0][2];
  vec1.y = mat[1][2];
  vec1.z = mat[2][2];

  vec2 = cross(YVEC, vec1);

  matrix4x4 tmp_mat( vec2.x, YVEC.x, vec1.x, 0.0f,
                     vec2.y, YVEC.y, vec1.y, 0.0f,
                     vec2.z, YVEC.z, vec1.z, 0.0f,
                     0.0f,   0.0f,   0.0f,   1.0f );

  B1 = tmp_mat;

  // C1
  matrix4x4 inv_temp(B1[0][0], B1[1][0], B1[2][0], B1[3][0],
                     B1[0][1], B1[1][1], B1[2][1], B1[3][1],
                     B1[0][2], B1[1][2], B1[2][2], B1[3][2],
                     B1[0][3], B1[1][3], B1[2][3], B1[3][3]);


  C1 = inv_temp*mat;

  // Conjugate back
  A1 = conj_left*A1;
  A1 = A1*conj_right;
  B1 = conj_left*B1;
  B1 = B1*conj_right;
  C1 = conj_left*C1;
  C1 = C1*conj_right;


  if(axis1.x == 1.0f)
    *ang1 = -safe_atan2(A1[1][2], A1[1][1]);
  else if(axis2.x == 1.0f)
    *ang2 = -safe_atan2(B1[1][2], B1[1][1]);
  else if(axis3.x == 1.0f)
    *ang3 = -safe_atan2(C1[1][2], C1[1][1]);


  if(axis1.y == 1.0f)
    *ang1 = safe_atan2(A1[0][2], A1[0][0]);
  else if(axis2.y == 1.0f)
    *ang2 = safe_atan2(B1[0][2], B1[0][0]);
  else if(axis3.y == 1.0f)
    *ang3 = safe_atan2(C1[0][2], C1[0][0]);


  if(axis1.z == 1.0f)
    *ang1 = safe_atan2(A1[1][0], A1[1][1]);
  else if(axis2.z == 1.0f)
    *ang2 = safe_atan2(B1[1][0], B1[1][1]);
  else if(axis3.z == 1.0f)
    *ang3 = safe_atan2(C1[1][0], C1[1][1]);

#if defined(CHECK_PO_VALIDITY)
  assert( is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */
}


void interpolate_po( const po& _1st, const po& _2nd, po& res, rational_t time_ratio )
{
#if defined(CHECK_PO_VALIDITY)
  assert( _1st.is_valid() );
  assert( _2nd.is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

  if( time_ratio <= 0.0f ) res = _1st;
  else if( time_ratio >= 1.0f ) res = _2nd;
  else
  {
    for( int i = 0; i < 4; ++i )
      for( int j = 0; j < 3; ++j )
        res.m[i][j] = _1st.m[i][j] + ( _2nd.m[i][j] - _1st.m[i][j] ) * time_ratio;
    res.m[0][3] = res.m[1][3] = res.m[2][3] = 0.0f;
    res.m[3][3] = 1.0f;
  }
/*
  if( time_ratio <= 0.0f ) res = _1st;
  else if( time_ratio >= 1.0f ) res = _2nd;
  else
  {
    for( int i = 0; i < 4; ++i )
      for( int j = 0; j < 3; ++j )
        res.m[i][j] = _1st.m[i][j] + ( _2nd.m[i][j] - _1st.m[i][j] ) * time_ratio;
    res.m[0][3] = res.m[1][3] = res.m[2][3] = 0.0f;
    res.m[3][3] = 1.0f;
  }
*/
#if defined(CHECK_PO_VALIDITY)
  assert( res.is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */
}

#if defined(CHECK_PO_VALIDITY)
bool po::is_valid( void ) const
{
  return m.x.is_valid() &&
    m.y.is_valid() &&
    m.z.is_valid() &&
    m.z.is_valid() &&
    get_position().is_valid();
}

#endif /* TARGET_XBOX JIV DEBUG */
