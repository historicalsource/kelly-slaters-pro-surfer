#ifndef PO_H
#define PO_H
////////////////////////////////////////////////////////////////////////////////
/*
  po

  Stands for "position and orientation".  It is a 4x4 matrix where the uppermost
  3x3 submatrix is a rotation and the right column is an affine offset with a '1' in the
  lower right corner.  A = (a_ij) is the transform and v = (v_x, v_y, v_z) is the affine
  part of the transform, then the representative 4x4 matrix is:
        a_11  a_12  a_13  v_x
        a_21  a_22  a_23  v_y
        a_31  a_32  a_33  v_z
        0     0     0     1
  I went with four-by-four instead of three-by-three plus translation just
  for easier integration with D3D.  I have to admit the abstraction appeals
  to me, though.

  I didn't use the HW prefix because, although it's currently implemented as
  a layer on top of d3d, we may convert this to our code later


///////////////
02-08-00 (Jason Bare)
I took out all the references to the matrix using the bracket ([]) operators. It
seemed to cause a big hit on the PS2 to use the bracket operators, so I replaced them all
with the actual matrix members, where possible. I left commented out code using the bracket
operators in case we need to revert back.
///////////////

*/
////////////////////////////////////////////////////////////////////////////////
#include "algebra.h"
#include "chunkfile.h"

class po;
class Matrix3;  // for tool

// I created this class mostly so you could copy the orientation
// part of a po from one po to another without bashing position.
// It could be given more functionality, it could store itself
// as theta,psi,rho or a quaternion, we could make a matrix3x3,
// we could do a lot of things
class orientation
{
public:
  // identity orientation (y up, facing z)
  orientation();

private:
  rational_t cells[3][3];

  friend class po;
};

#if defined(DEBUG)
#if defined(TARGET_XBOX) || defined(TARGET_GC)
#define CHECK_PO_VALIDITY
#endif
#endif //DEBUG

class po
{
  private:
    matrix4x4 m;

  public:
#if defined(CHECK_PO_VALIDITY)
    bool is_valid( void ) const;
#endif /* TARGET_XBOX JIV DEBUG */

    po() {
#if defined(TARGET_XBOX) || defined(TARGET_GC)
      m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f; m[0][3] = 0.0f;
      m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f; m[1][3] = 0.0f;
      m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f; m[2][3] = 0.0f;
      m[3][0] = 0.0f; m[3][1] = 0.0f; m[3][2] = 0.0f; m[3][3] = 1.0f;
#endif

#if defined(CHECK_PO_VALIDITY)
      assert(is_valid());
#endif /* TARGET_XBOX JIV DEBUG */
    }

    // create a whole damn matrix cell-by-cell
    po( rational_t _00,rational_t _01,rational_t _02,/*rational_t _03,*/
        rational_t _10,rational_t _11,rational_t _12,/*rational_t _13,*/
        rational_t _20,rational_t _21,rational_t _22,/*rational_t _23,*/
        rational_t _30,rational_t _31,rational_t _32/*,rational_t _33*/ )
    {
      m[0][0] = _00; m[0][1] = _01; m[0][2] = _02; m[0][3] = 0.0f; //m[0][3] = _03;
      m[1][0] = _10; m[1][1] = _11; m[1][2] = _12; m[1][3] = 0.0f; //m[1][3] = _13;
      m[2][0] = _20; m[2][1] = _21; m[2][2] = _22; m[2][3] = 0.0f; //m[2][3] = _23;
      m[3][0] = _30; m[3][1] = _31; m[3][2] = _32; m[3][3] = 1.0f; //m[3][3] = _33;

#if defined(CHECK_PO_VALIDITY)
      assert(is_valid());
#endif /* TARGET_XBOX JIV DEBUG */
    }

    po(const vector3d& xdest, const vector3d& ydest, const vector3d& zdest, const vector3d& pos)
    {
      m[0][0] = xdest.x; m[0][1] = xdest.y; m[0][2] = xdest.z; m[0][3] = 0.0f;
      m[1][0] = ydest.x; m[1][1] = ydest.y; m[1][2] = ydest.z; m[1][3] = 0.0f;
      m[2][0] = zdest.x; m[2][1] = zdest.y; m[2][2] = zdest.z; m[2][3] = 0.0f;
      m[3][0] =   pos.x; m[3][1] =   pos.y; m[3][2] =   pos.z; m[3][3] = 1.0f;

#if defined(CHECK_PO_VALIDITY)
      assert(is_valid());
#endif /* TARGET_XBOX JIV DEBUG */
    }

    po(const vector3d& desired_forward, const vector3d& normal, const vector3d& pos)
    {
      vector3d ydest = normal;
      ydest.normalize();

      vector3d zdest = desired_forward;
      zdest.normalize();

      vector3d xdest;
      xdest.x = (ydest.y*zdest.z - ydest.z*zdest.y);
      xdest.y = (ydest.z*zdest.x - ydest.x*zdest.z);
      xdest.z = (ydest.x*zdest.y - ydest.y*zdest.x);

      zdest.x = (xdest.y*ydest.z - xdest.z*ydest.y);
      zdest.y = (xdest.z*ydest.x - xdest.x*ydest.z);
      zdest.z = (xdest.x*ydest.y - xdest.y*ydest.x);

      m[0][0] = xdest.x; m[0][1] = xdest.y; m[0][2] = xdest.z; m[0][3] = 0.0f;
      m[1][0] = ydest.x; m[1][1] = ydest.y; m[1][2] = ydest.z; m[1][3] = 0.0f;
      m[2][0] = zdest.x; m[2][1] = zdest.y; m[2][2] = zdest.z; m[2][3] = 0.0f;
      m[3][0] =   pos.x; m[3][1] =   pos.y; m[3][2] =   pos.z; m[3][3] = 1.0f;

#if defined(CHECK_PO_VALIDITY)
      assert(is_valid());
#endif /* TARGET_XBOX JIV DEBUG */
    }

    // construct a po from position, rotation and scale values
    po( const vector3d& p, const quaternion& r, rational_t s );

  public:  // more dangerously slow overused functions!
    po( const matrix4x4& _m )
    {
      m = _m;
#ifdef DEBUG
      assert(!has_nonuniform_scaling());
#endif
      //assert(__fabs(m[0].get_xyz().length2()-1.0f)<=1e-2f);
      assert(m[0][3]==0 && m[1][3]==0 && m[2][3]==0 && m[3][3]==1);

#if defined(CHECK_PO_VALIDITY)
      assert(is_valid());
#endif /* TARGET_XBOX JIV DEBUG */
    }
    //po( const po& b ) { m = b.m; }
    //const po& operator=( const po& b )
    //{
    //  m = b.m;
    //  return *this;
    //}
  public:

    const matrix4x4& get_matrix() const
    { return m; }
    matrix4x4& get_matrix()
    { return m; }

    bool operator==( const po& b ) const;
    bool operator!=( const po& b ) const { return !(*this==b); }

    // this assumes that the matrix doesn't have any scaling
    // factors and will produce a [x,y,z,1] vector
    // (which it shouldn't, because it's a po.)
    // don't use for bottleneck code
    vector3d slow_xform(const vector3d& source_vector) const;

    // ignores the translation part
    vector3d non_affine_slow_xform(const vector3d& source_vector) const;

    // set a translation matrix
    void set_translate( const vector3d& translation );

    // set a scale matrix
    void set_scale( const vector3d& scale );

    // set a matrix that rotates around x
    void set_rotate_x( rational_t rads );

    // set a matrix that rotates around y
    void set_rotate_y( rational_t rads );

    // set a matrix that rotates around z
    void set_rotate_z( rational_t rads );

    // set a matrix that rotates around an arbitrary vector
    void set_rot( const vector3d& rot_dir, float rads );

    // set a matrix that rotates around an arbitrary vector
    void set_rot( const vector3d& rot_dir);

    // extract the position vector
    inline const vector3d& get_position() const
    {
      return (const vector3d&)(m[3]);
    }

    // replace the position vector
    inline void set_position(const vector3d& v)
    {
#if defined(CHECK_PO_VALIDITY)
      assert( v.is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

      m[3][0] = v.x; m[3][1] = v.y; m[3][2] = v.z;

#if defined(CHECK_PO_VALIDITY)
      assert(is_valid());
#endif /* TARGET_XBOX JIV DEBUG */

    }

    // scale the po
    inline void scale( rational_t s )
    {
      m.scale(s);

#if defined(CHECK_PO_VALIDITY)
      assert( is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */
    }

    // extract the orientation
    orientation get_orientation() const;

    // replace the orientation
    void set_orientation(const orientation& o);

    // replace the orientation
    void set_orientation(const po & o);

    // correct orientation part of 4x4 to be a true SO3
    void fixup();

    inline rational_t get_scale() const
    {
      return ((const vector3d&)m).length();
    }

    // remove scaling from po without affecting position component
    inline void non_affine_normalize()
    {
      // CTT 04/14/00: TEMPORARY:
      // This is not terribly efficient, but to fix the problem I need to totally
      // revamp the po class so that it maintains a uniform scale value (which in
      // turn means using a specialized structure instead of matrix4x4).
      rational_t s = get_scale();
      if ( s<0.999f || s>1.001f )
      {
        s = 1.0f / s;
        (vector3d&)m *= s;
        (vector3d&)m[1] *= s;
        (vector3d&)m[2] *= s;
      }

#if defined(CHECK_PO_VALIDITY)
      assert( is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

    }

    // advance orientation and position by inc
    void add_increment(po * inc);

    // puts the inverse of the matrix in result, returns a reference to result.
    po & inverse() const;

    // inverts the transform.
    inline void invert()
    {
      *this = inverse();

#if defined(CHECK_PO_VALIDITY)
      assert( is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

    }

    // xforms source_vector by the inverse of the transformation
//    vector3d inverse_xform(const vector3d& source_vector) const;
    inline vector3d inverse_xform( const vector3d & v ) const
    {
#if defined(CHECK_PO_VALIDITY)
      assert( v.is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

      assert (m[0][0]>=-1.01f && m[0][0]<= 1.01f);

      vector3d imv(v.x-m.w.x,v.y-m.w.y,v.z-m.w.z);

      return vector3d(m.x.x * imv.x + m.x.y * imv.y + m.x.z * imv.z,
                      m.y.x * imv.x + m.y.y * imv.y + m.y.z * imv.z,
                      m.z.x * imv.x + m.z.y * imv.y + m.z.z * imv.z);
/*
      vector3d imv(v.x-m[3][0],v.y-m[3][1],v.z-m[3][2]);

      return vector3d(m[0][0] * imv.x + m[0][1] * imv.y + m[0][2] * imv.z,
                      m[1][0] * imv.x + m[1][1] * imv.y + m[1][2] * imv.z,
                      m[2][0] * imv.x + m[2][1] * imv.y + m[2][2] * imv.z);
*/
    }

    vector3d non_affine_inverse_xform( const vector3d& v ) const;
    vector3d non_affine_inverse_scaled_xform( const vector3d& v ) const;

    void scale3x3( rational_t scale );

    const vector3d& get_x_facing() const { return (const vector3d&)m[0]; }
    const vector3d& get_y_facing() const { return (const vector3d&)m[1]; }
    const vector3d& get_z_facing() const { return (const vector3d&)m[2]; }

    // get facing returns a vector pointing in the direction you're facing
    const vector3d& get_facing() const { return get_z_facing(); }

    // set_facing sets the orientation to point to the given vector (with zero roll);
    // set_facing won't bash position
    void set_facing( const vector3d& v );

    bool has_nonuniform_scaling() const;

    // obtain a quaternion representing the orientation
    quaternion get_quaternion() const;

	  // Decompose the po into radian angles around the specified axiis. Currently only works when the axiis are the XYZ axiis.
	  // However the order or number (1-3) doesn't matter.
	  void decompose_rot(const vector3d &axis1, const vector3d &axis2, const vector3d &axis3, rational_t *ang1, rational_t *ang2, rational_t *ang3);

    // only works on 8byte aligned PO's!!!
    inline vector3d fast_8byte_xform(const vector3d& sv) const
    {
#if defined(TARGET_MKS)
      asm_m4x4_load_aligned(&m);
      vector3d imv;
      asm_ftrv_3_1(sv.x, sv.y, sv.z, &imv);
      return imv;
#else
      return slow_xform(sv);
#endif
    }

    // only works on 8byte aligned PO's!!!
    inline vector3d fast_inverse_xform(const vector3d& sv) const
    {
#if defined(TARGET_MKS)
      asm_po_load_inverse(this);
      vector3d imv;
      asm_ftrv_3_1(sv.x, sv.y, sv.z, &imv);
      return imv;
#else
      return inverse_xform(sv);
#endif
    }

    // only works on 8byte aligned PO's!!!
    inline vector3d fast_8byte_non_affine_xform(const vector3d& sv) const
    {
#if defined(TARGET_MKS)
      asm_m4x4_load_aligned(&m);
      vector3d imv;
      asm_ftrv_3_0(sv.x, sv.y, sv.z, &imv);
      return imv;
#else
      return non_affine_slow_xform(sv);
#endif
    }

    inline vector3d fast_non_affine_inverse_xform(const vector3d& sv) const
    {
#if defined(TARGET_MKS)
      asm_po_load_transpose(this);
      vector3d imv;
      asm_ftrv_3_0(sv.x, sv.y, sv.z, &imv);
      return imv;
#else
      return non_affine_inverse_xform(sv);
#endif
    }

    friend void interpolate_po( const po& _1st, const po& _2nd, po& res, rational_t time_ratio );

#if defined(TARGET_MKS)
    po operator*( const po& b ) const
    {
#if defined(CHECK_PO_VALIDITY)
      assert( po.is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

      po local;
      asm_po_mul( this, &b, &local );

#if defined(CHECK_PO_VALIDITY)
      assert( local.is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

      return local;
    }
#elif defined(TARGET_PS2)
/*
    po operator*( const po& b ) const
    {
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
      return local;
    }
//*/
//    po ps2_mul( const po& b ) const
    inline po operator*( const po& b ) const
    {
      po local;

      // Keep this in sync with the PS2 function 'inline void fast_po_mul(po &res, const po& a, const po& b)'
      asm("lqc2    vf1,0x0(%2)  \n"
	        "lqc2    vf2,0x10(%2) \n"
	        "lqc2    vf3,0x20(%2) \n"
	        "lqc2    vf4,0x30(%2) \n"

	        "lqc2    vf5,0x0(%1) \n"
	        "lqc2    vf6,0x10(%1) \n"
	        "lqc2    vf7,0x20(%1) \n"
	        "lqc2    vf8,0x30(%1) \n"

          "vmulax.xyzw	ACC,   vf1,vf5 \n"
	        "vmadday.xyzw	ACC,   vf2,vf5 \n"
	        "vmaddz.xyzw	vf9,   vf3,vf5 \n"

	        "vmulax.xyzw	ACC,   vf1,vf6 \n"
	        "vmadday.xyzw	ACC,   vf2,vf6 \n"
	        "vmaddz.xyzw	vf10,   vf3,vf6 \n"

	        "vmulax.xyzw	ACC,   vf1,vf7 \n"
	        "vmadday.xyzw	ACC,   vf2,vf7 \n"
	        "vmaddz.xyzw	vf11,   vf3,vf7 \n"

	        "vmulax.xyzw	ACC,   vf1,vf8 \n"
	        "vmadday.xyzw	ACC,   vf2,vf8 \n"
	        "vmaddaz.xyzw	ACC,   vf3,vf8 \n"
	        "vmaddw.xyzw	vf12,  vf4,vf8 \n"

	        "sqc2    vf9,0x0(%0) \n"
	        "sqc2    vf10,0x10(%0) \n"
	        "sqc2    vf11,0x20(%0) \n"
	        "sqc2    vf12,0x30(%0) \n"
	    :
      : "r" ((sceVu0FMATRIX)&local.m.x.x), "r" ((sceVu0FMATRIX)&m.x.x), "r" ((sceVu0FMATRIX)&b.m.x.x)
      );

      local.m.x.w = 0.0f;
      local.m.y.w = 0.0f;
      local.m.z.w = 0.0f;
      local.m.w.w = 1.0f;

      return local;
    }
#else
    po operator*( const po& b ) const;
#endif

#if !defined(NO_SERIAL_OUT)
  friend void serial_out(chunk_file& io, const po& _po );
#endif
#if !defined(NO_SERIAL_IN)
  friend void serial_in(chunk_file& io, po* _po);
#endif
  friend po to_po( const Matrix3& m3 );
  friend po MAX_to_game_coords( const Matrix3& m3 );
  friend inline void fast_po_mul(po &res, const po& a, const po& b);
};

#if defined(TARGET_PS2)
inline void fast_po_mul(po &res, const po& a, const po& b)
{
  asm("lqc2    vf1,0x0(%2)  \n"
	    "lqc2    vf2,0x10(%2) \n"
	    "lqc2    vf3,0x20(%2) \n"
	    "lqc2    vf4,0x30(%2) \n"

	    "lqc2    vf5,0x0(%1) \n"
	    "lqc2    vf6,0x10(%1) \n"
	    "lqc2    vf7,0x20(%1) \n"
	    "lqc2    vf8,0x30(%1) \n"

      "vmulax.xyzw	ACC,   vf1,vf5 \n"
	    "vmadday.xyzw	ACC,   vf2,vf5 \n"
	    "vmaddz.xyzw	vf9,   vf3,vf5 \n"

	    "vmulax.xyzw	ACC,   vf1,vf6 \n"
	    "vmadday.xyzw	ACC,   vf2,vf6 \n"
	    "vmaddz.xyzw	vf10,   vf3,vf6 \n"

	    "vmulax.xyzw	ACC,   vf1,vf7 \n"
	    "vmadday.xyzw	ACC,   vf2,vf7 \n"
	    "vmaddz.xyzw	vf11,   vf3,vf7 \n"

	    "vmulax.xyzw	ACC,   vf1,vf8 \n"
	    "vmadday.xyzw	ACC,   vf2,vf8 \n"
	    "vmaddaz.xyzw	ACC,   vf3,vf8 \n"
	    "vmaddw.xyzw	vf12,  vf4,vf8 \n"

	    "sqc2    vf9,0x0(%0) \n"
	    "sqc2    vf10,0x10(%0) \n"
	    "sqc2    vf11,0x20(%0) \n"
	    "sqc2    vf12,0x30(%0) \n"
	:
  : "r" ((sceVu0FMATRIX)&res.m.x.x), "r" ((sceVu0FMATRIX)&a.m.x.x), "r" ((sceVu0FMATRIX)&b.m.x.x)
  );

  res.m.x.w = 0.0f;
  res.m.y.w = 0.0f;
  res.m.z.w = 0.0f;
  res.m.w.w = 1.0f;
}
#else
inline void fast_po_mul(po &res, const po& a, const po& b)
{
#if defined(CHECK_PO_VALIDITY)
  assert( a.is_valid() );
  assert( b.is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

  res = a*b;

#if defined(CHECK_PO_VALIDITY)
  assert( res.is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */
}
#endif



inline void fast_po_copy( const po* a, po* dest )
{
#if defined(CHECK_PO_VALIDITY)
  assert( a->is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */

#if defined(TARGET_MKS)
  asm_m4x4_copy((const matrix4x4*)a,(matrix4x4*)dest);
#else
  *dest = *a;
#endif

#if defined(CHECK_PO_VALIDITY)
  assert( dest->is_valid() );
#endif /* TARGET_XBOX JIV DEBUG */
}

const po po_identity_matrix(1.0f, 0.0f, 0.0f,// 0.0f,
                            0.0f, 1.0f, 0.0f,// 0.0f,
                            0.0f, 0.0f, 1.0f,// 0.0f,
                            0.0f, 0.0f, 0.0f//, 1.0f
                            );




// Stream interface
#if !defined(NO_SERIAL_OUT)
inline void serial_out( chunk_file & io, const po& _po )
{
  assert( _po.m[0][3] == 0 && _po.m[1][3] == 0.0f && _po.m[2][3] == 0.0f && _po.m[3][3] == 1.0f );

  if (io.get_type()==chunk_file::CFT_TEXT)
  {
    for ( int i=0; i<3; ++i )
      for ( int j=0; j<4; ++j )
        io.get_text()->write(ftos(_po.m[j][i])+" ");
    io.get_text()->write(sendl);
  }
  else
    io.get_binary()->write((void*)&_po,sizeof(po));
}
#endif

#if !defined(NO_SERIAL_IN)
inline void serial_in ( chunk_file& io, po* _po )
{
  for ( int i=0; i<3; ++i )
  {
    for ( int j=0; j<4; ++j )
      serial_in( io, &_po->m[j][i] );
  }
  for ( int j=0; j<3; ++j )
    _po->m[j][3] = 0.0f;
  _po->m[3][3] = 1.0f;
}
#endif

extern po global_identity_po;

#endif
